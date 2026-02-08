/******************************************************************************
 * File: redismset.c
 * Author: Ernest Rozloznik (e@er400.io)
 * Date: 2025-02-22
 * Description: Implementation of the Redis MSET function for IBM i.
 *              Takes comma-separated key=value pairs and sets them atomically.
 *              Input format: "key1=val1,key2=val2,key3=val3"
 *              Returns "OK" on success.
 * License: MIT (https://opensource.org/licenses/MIT)
 * Version: 1.0.0
 ******************************************************************************/

#include "redis_utils.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#ifdef USE_ICONV
#include <qtqiconv.h>
#endif

/**********************************************************************/
/* SQL External Function: MSET                                        */
/**********************************************************************/

void SQL_API_FN msetRedisValues(
    SQLUDF_VARCHAR *kvpairs,      // Input: "key1=val1,key2=val2" (VARCHAR(16370), EBCDIC)
    SQLUDF_VARCHAR *response,     // Output: "OK" (VARCHAR(128), EBCDIC)
    SQLUDF_NULLIND *kvpairsInd,
    SQLUDF_NULLIND *responseInd,
    char *sqlstate,
    char *funcname,
    char *specname,
    char *msgtext,
    short *sqlcode,
    SQLUDF_NULLIND *nullind)
{
    int sockfd;
    char ebcdic_send_buf[33000], ascii_send_buf[33000];
    char recv_buf[1024], ebcdic_payload[1024];
    int len, total_len = 0;

#ifdef USE_ICONV
    if (!initialized)
    {
        initialize_conversion();
        initialized = 1;
    }
    if (errno != 0)
    {
        strcpy(sqlstate, "38999");
        strcpy(msgtext, "iconv initialization failed");
        *responseInd = -1;
        return;
    }
#endif

    // Initialize SQLSTATE to success
    strncpy(sqlstate, "00000", 5);
    sqlstate[5] = '\0';
    msgtext[0] = '\0';

    // Check for NULL input
    if (*kvpairsInd == -1)
    {
        strncpy(sqlstate, "38001", 5);
        strncpy(msgtext, "Input key-value pairs is NULL", 70);
        *responseInd = -1;
        return;
    }

    // Initialize response
    response[0] = '\0';
    *responseInd = 0;

    // Parse comma-separated key=value pairs
    // EBCDIC comma = 0x6B, EBCDIC '=' = 0x7E
    char *pair_ptrs[256];     // Max 256 pairs
    int pair_lens[256];
    int pair_count = 0;
    char *start = kvpairs;
    char *p = kvpairs;

    while (*p != '\0' && pair_count < 256)
    {
        if ((unsigned char)*p == 0x6B) // EBCDIC ','
        {
            pair_ptrs[pair_count] = start;
            pair_lens[pair_count] = p - start;
            pair_count++;
            start = p + 1;
        }
        p++;
    }
    // Last pair
    if (start <= p && pair_count < 256)
    {
        pair_ptrs[pair_count] = start;
        pair_lens[pair_count] = p - start;
        pair_count++;
    }

    if (pair_count == 0)
    {
        strncpy(sqlstate, "38001", 5);
        strncpy(msgtext, "No key-value pairs provided", 70);
        *responseInd = -1;
        return;
    }

    // Split each pair on '=' to get keys and values
    char *keys[256], *vals[256];
    int klens[256], vlens[256];
    int k;
    for (k = 0; k < pair_count; k++)
    {
        char *eq = NULL;
        char *pp = pair_ptrs[k];
        int pl = pair_lens[k];
        int e;
        for (e = 0; e < pl; e++)
        {
            if ((unsigned char)pp[e] == 0x7E) // EBCDIC '='
            {
                eq = pp + e;
                break;
            }
        }
        if (eq == NULL)
        {
            strncpy(sqlstate, "38001", 5);
            strncpy(msgtext, "Invalid key=value pair format", 70);
            *responseInd = -1;
            return;
        }
        keys[k] = pair_ptrs[k];
        klens[k] = eq - pair_ptrs[k];
        vals[k] = eq + 1;
        vlens[k] = pair_lens[k] - klens[k] - 1;
    }

    // Connect to Redis
    if (connect_to_redis(&sockfd) != 0)
    {
        strncpy(sqlstate, "38901", 5);
        snprintf(msgtext, 70, "Failed to connect to Redis: errno=%d", errno);
        *responseInd = -1;
        close(sockfd);
        return;
    }

    // Build RESP command: *<2N+1>\r\n$4\r\nMSET\r\n then for each pair: $<klen>\r\n<key>\r\n$<vlen>\r\n<value>\r\n
    ebcdic_send_buf[0] = '\0';

    // Format 2*N+1 (total args = 2*pair_count + 1 for MSET command)
    int total_args = 2 * pair_count + 1;
    char ebcdic_total[10] = {0};
    if (total_args < 10)
    {
        ebcdic_total[0] = 0xF0 + total_args;
        ebcdic_total[1] = '\0';
    }
    else
    {
        int i = 0, temp_len = total_args;
        while (temp_len > 0)
        {
            ebcdic_total[i++] = 0xF0 + (temp_len % 10);
            temp_len /= 10;
        }
        ebcdic_total[i] = '\0';
        for (int j = 0; j < i / 2; j++)
        {
            char tmp = ebcdic_total[j];
            ebcdic_total[j] = ebcdic_total[i - 1 - j];
            ebcdic_total[i - 1 - j] = tmp;
        }
    }

    // *<2N+1>\r\n
    strcat(ebcdic_send_buf, "\x5C");           // *
    strcat(ebcdic_send_buf, ebcdic_total);
    strcat(ebcdic_send_buf, "\x0D\x25");       // \r\n
    // $4\r\nMSET\r\n
    strcat(ebcdic_send_buf, "\x5B\xF4\x0D\x25\xD4\xE2\xC5\xE3\x0D\x25"); // $4\r\nMSET\r\n

    // For each pair: $<klen>\r\n<key>\r\n$<vlen>\r\n<value>\r\n
    for (k = 0; k < pair_count; k++)
    {
        // Format key length
        char ebcdic_key_len[10] = {0};
        int kl = klens[k];
        if (kl < 10)
        {
            ebcdic_key_len[0] = 0xF0 + kl;
            ebcdic_key_len[1] = '\0';
        }
        else
        {
            int i = 0, temp_len = kl;
            while (temp_len > 0)
            {
                ebcdic_key_len[i++] = 0xF0 + (temp_len % 10);
                temp_len /= 10;
            }
            ebcdic_key_len[i] = '\0';
            for (int j = 0; j < i / 2; j++)
            {
                char tmp = ebcdic_key_len[j];
                ebcdic_key_len[j] = ebcdic_key_len[i - 1 - j];
                ebcdic_key_len[i - 1 - j] = tmp;
            }
        }

        // Format value length
        char ebcdic_val_len[10] = {0};
        int vl = vlens[k];
        if (vl < 10)
        {
            ebcdic_val_len[0] = 0xF0 + vl;
            ebcdic_val_len[1] = '\0';
        }
        else
        {
            int i = 0, temp_len = vl;
            while (temp_len > 0)
            {
                ebcdic_val_len[i++] = 0xF0 + (temp_len % 10);
                temp_len /= 10;
            }
            ebcdic_val_len[i] = '\0';
            for (int j = 0; j < i / 2; j++)
            {
                char tmp = ebcdic_val_len[j];
                ebcdic_val_len[j] = ebcdic_val_len[i - 1 - j];
                ebcdic_val_len[i - 1 - j] = tmp;
            }
        }

        strcat(ebcdic_send_buf, "\x5B");           // $
        strcat(ebcdic_send_buf, ebcdic_key_len);
        strcat(ebcdic_send_buf, "\x0D\x25");       // \r\n
        strncat(ebcdic_send_buf, keys[k], kl);
        strcat(ebcdic_send_buf, "\x0D\x25\x5B");   // \r\n$
        strcat(ebcdic_send_buf, ebcdic_val_len);
        strcat(ebcdic_send_buf, "\x0D\x25");       // \r\n
        strncat(ebcdic_send_buf, vals[k], vl);
        strcat(ebcdic_send_buf, "\x0D\x25");       // \r\n
    }

    // Convert EBCDIC command to ASCII before sending
    size_t ebcdic_len_size = strlen(ebcdic_send_buf);
    if (ConvertToASCII(ebcdic_send_buf, ebcdic_len_size, ascii_send_buf, sizeof(ascii_send_buf) - 1) < 0)
    {
        strncpy(sqlstate, "38902", 5);
        strncpy(msgtext, "Failed to convert command to ASCII", 70);
        *responseInd = -1;
        close(sockfd);
        return;
    }
    ascii_send_buf[ebcdic_len_size] = '\0';

    // Send MSET command to Redis
    len = send(sockfd, ascii_send_buf, strlen(ascii_send_buf), 0);
    if (len < 0)
    {
        strncpy(sqlstate, "38903", 5);
        snprintf(msgtext, 70, "Failed to send command to Redis: errno=%d", errno);
        *responseInd = -1;
        close(sockfd);
        return;
    }

    // Receive response from Redis
    len = recv(sockfd, recv_buf, sizeof(recv_buf) - 1, 0);
    if (len < 0)
    {
        if (errno == EWOULDBLOCK || errno == EAGAIN)
        {
            strncpy(sqlstate, "38904", 5);
            snprintf(msgtext, 70, "Receive timeout from Redis: errno=%d, socket=%d", errno, sockfd);
        }
        else
        {
            strncpy(sqlstate, "38905", 5);
            snprintf(msgtext, 70, "Failed to receive data from Redis: errno=%d, socket=%d", errno, sockfd);
        }
        *responseInd = -1;
        close(sockfd);
        return;
    }
    else if (len == 0)
    {
        strncpy(sqlstate, "38906", 5);
        snprintf(msgtext, 70, "Connection closed by Redis, socket=%d", sockfd);
        *responseInd = -1;
        close(sockfd);
        return;
    }

    total_len = len;
    recv_buf[total_len] = '\0';

    // Convert ASCII response to EBCDIC
    if (ConvertToEBCDIC(recv_buf, total_len, ebcdic_payload, sizeof(ebcdic_payload) - 1) < 0)
    {
        strncpy(sqlstate, "38907", 5);
        strncpy(msgtext, "Failed to convert response to EBCDIC", 70);
        *responseInd = -1;
        close(sockfd);
        return;
    }
    ebcdic_payload[total_len] = '\0';

    // Extract Redis response (expects +OK)
    char *payload = NULL;
    size_t payload_length;
    if (extract_redis_payload(ebcdic_payload, &payload, &payload_length) == 0)
    {
        if (payload_length < 128)
        {
            strncpy(response, payload, payload_length);
            response[payload_length] = '\0';
            *responseInd = 0;
        }
        else
        {
            strncpy(sqlstate, "38908", 5);
            strncpy(msgtext, "Response exceeds maximum length", 70);
            *responseInd = -1;
        }
    }
    else
    {
        strncpy(sqlstate, "38909", 5);
        snprintf(msgtext, 512, "Failed to extract payload from Redis response: EBCDIC=%.462s...", ebcdic_payload);
        *responseInd = -1;
    }

    close(sockfd);
}

#pragma linkage(msetRedisValues, OS)
