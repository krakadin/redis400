/******************************************************************************
 * File: redismget.c
 * Author: Ernest Rozloznik (e@er400.io)
 * Date: 2025-02-22
 * Description: Implementation of the Redis MGET function for IBM i.
 *              Takes a comma-separated list of keys and returns comma-separated
 *              values. Missing keys produce empty strings between commas.
 *              Example: MGET("k1,k2,k3") -> "val1,,val3" (k2 not found)
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
/* Helper: Parse RESP array for MGET (handles nil bulk strings)       */
/**********************************************************************/

static int parse_mget_response(char *ebcdic_response, char *output, size_t output_size)
{
    char *pos = ebcdic_response;
    int elem_count = 0;
    size_t out_offset = 0;

    // Check for RESP array prefix: EBCDIC '*' = 0x5C
    if (*pos != 0x5C)
        return -1;
    pos++;

    // Parse array element count
    char count_buf[10];
    char *crlf = strstr(pos, "\x0D\x25"); // EBCDIC \r\n
    if (!crlf)
        return -1;

    size_t count_len = crlf - pos;
    if (count_len >= sizeof(count_buf))
        return -1;
    strncpy(count_buf, pos, count_len);
    count_buf[count_len] = '\0';

    int element_count = atoi(count_buf);
    if (element_count <= 0)
        return -2; // Empty result

    pos = crlf + 2; // Skip past \r\n

    // Parse each element
    int i;
    for (i = 0; i < element_count; i++)
    {
        // Append comma separator (if not first element)
        if (elem_count > 0)
        {
            if (out_offset + 1 >= output_size)
                break; // Output buffer full
            output[out_offset++] = 0x6B; // EBCDIC ','
        }

        // Parse bulk string: $len\r\ndata\r\n or $-1\r\n (nil)
        if (*pos != 0x5B) // EBCDIC '$'
            return -1;
        pos++;

        // Check for nil: $-1\r\n (EBCDIC: 0x60 0xF1 = "-1")
        if (pos[0] == 0x60 && pos[1] == 0xF1)
        {
            // Nil bulk string - skip past -1\r\n
            crlf = strstr(pos, "\x0D\x25");
            if (!crlf)
                return -1;
            pos = crlf + 2;
            // Don't append anything for nil (empty string between commas)
            elem_count++;
            continue;
        }

        crlf = strstr(pos, "\x0D\x25");
        if (!crlf)
            return -1;

        char len_buf[10];
        size_t len_len = crlf - pos;
        if (len_len >= sizeof(len_buf))
            return -1;
        strncpy(len_buf, pos, len_len);
        len_buf[len_len] = '\0';

        size_t elem_length = atoi(len_buf);
        pos = crlf + 2; // Skip to data
        char *elem_start = pos;
        pos += elem_length;

        // Skip trailing \r\n
        if (strncmp(pos, "\x0D\x25", 2) != 0)
            return -1;
        pos += 2;

        // Check if we have room for this element
        if (out_offset + elem_length >= output_size)
            break; // Output buffer full

        strncpy(output + out_offset, elem_start, elem_length);
        out_offset += elem_length;

        elem_count++;
    }

    output[out_offset] = '\0';
    return elem_count;
}

/**********************************************************************/
/* SQL External Function: MGET                                        */
/**********************************************************************/

void SQL_API_FN mgetRedisValues(
    SQLUDF_VARCHAR *keys,         // Input: comma-separated keys (VARCHAR(16370), EBCDIC)
    SQLUDF_VARCHAR *value,        // Output: comma-separated values (VARCHAR(16370), EBCDIC)
    SQLUDF_NULLIND *keysInd,
    SQLUDF_NULLIND *valueInd,
    char *sqlstate,
    char *funcname,
    char *specname,
    char *msgtext,
    short *sqlcode,
    SQLUDF_NULLIND *nullind)
{
    int sockfd;
    char ebcdic_send_buf[33000], ascii_send_buf[33000];
    char recv_buf[32000], ebcdic_response[32000];
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
        *valueInd = -1;
        return;
    }
#endif

    // Check for NULL input
    if (*keysInd < 0)
    {
        strcpy(sqlstate, "38001");
        strcpy(msgtext, "Input keys is NULL");
        *valueInd = -1;
        return;
    }

    // Initialize
    strcpy(sqlstate, "00000");
    value[0] = '\0';
    *valueInd = 0;

    // Parse comma-separated keys to count them and store pointers
    // EBCDIC comma = 0x6B
    char *key_ptrs[256];  // Max 256 keys
    int key_lens[256];
    int key_count = 0;
    char *start = keys;
    char *p = keys;

    while (*p != '\0' && key_count < 256)
    {
        if ((unsigned char)*p == 0x6B) // EBCDIC ','
        {
            key_ptrs[key_count] = start;
            key_lens[key_count] = p - start;
            key_count++;
            start = p + 1;
        }
        p++;
    }
    // Last key (after last comma or only key)
    if (start <= p && key_count < 256)
    {
        key_ptrs[key_count] = start;
        key_lens[key_count] = p - start;
        key_count++;
    }

    if (key_count == 0)
    {
        strcpy(sqlstate, "38001");
        strcpy(msgtext, "No keys provided");
        *valueInd = -1;
        return;
    }

    // Connect to Redis
    if (connect_to_redis(&sockfd) != 0)
    {
        strcpy(sqlstate, "38901");
        strcpy(msgtext, "Failed to connect to Redis");
        *valueInd = -1;
        return;
    }

    // Build RESP command: *<N+1>\r\n$4\r\nMGET\r\n then for each key: $<keylen>\r\n<key>\r\n
    ebcdic_send_buf[0] = '\0';

    // Format N+1 (total args = key_count + 1 for MGET command)
    int total_args = key_count + 1;
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

    // *<N+1>\r\n
    strcat(ebcdic_send_buf, "\x5C");           // *
    strcat(ebcdic_send_buf, ebcdic_total);
    strcat(ebcdic_send_buf, "\x0D\x25");       // \r\n
    // $4\r\nMGET\r\n
    strcat(ebcdic_send_buf, "\x5B\xF4\x0D\x25\xD4\xC7\xC5\xE3\x0D\x25"); // $4\r\nMGET\r\n

    // For each key: $<keylen>\r\n<key>\r\n
    int k;
    for (k = 0; k < key_count; k++)
    {
        char ebcdic_key_len[10] = {0};
        int kl = key_lens[k];

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

        strcat(ebcdic_send_buf, "\x5B");           // $
        strcat(ebcdic_send_buf, ebcdic_key_len);
        strcat(ebcdic_send_buf, "\x0D\x25");       // \r\n
        strncat(ebcdic_send_buf, key_ptrs[k], kl);
        strcat(ebcdic_send_buf, "\x0D\x25");       // \r\n
    }

    // Convert to ASCII
    size_t ebcdic_len_size = strlen(ebcdic_send_buf);
    if (ConvertToASCII(ebcdic_send_buf, ebcdic_len_size, ascii_send_buf, sizeof(ascii_send_buf) - 1) < 0)
    {
        strcpy(sqlstate, "38902");
        strcpy(msgtext, "Failed to convert command to ASCII");
        *valueInd = -1;
        close(sockfd);
        return;
    }
    ascii_send_buf[ebcdic_len_size] = '\0';

    // Send command
    len = send(sockfd, ascii_send_buf, strlen(ascii_send_buf), 0);
    if (len < 0)
    {
        strcpy(sqlstate, "38903");
        strcpy(msgtext, "Failed to send command to Redis");
        *valueInd = -1;
        close(sockfd);
        return;
    }

    // Receive response
    len = recv(sockfd, recv_buf, sizeof(recv_buf) - 1, 0);
    if (len < 0)
    {
        if (errno == EWOULDBLOCK || errno == EAGAIN)
        {
            strcpy(sqlstate, "38904");
            strcpy(msgtext, "Receive timeout from Redis");
        }
        else
        {
            strcpy(sqlstate, "38905");
            strcpy(msgtext, "Failed to receive data from Redis");
        }
        *valueInd = -1;
        close(sockfd);
        return;
    }
    else if (len == 0)
    {
        strcpy(sqlstate, "38906");
        strcpy(msgtext, "Connection closed by Redis");
        *valueInd = -1;
        close(sockfd);
        return;
    }
    total_len = len;
    recv_buf[total_len] = '\0';

    // Convert response to EBCDIC
    if (ConvertToEBCDIC(recv_buf, total_len, ebcdic_response, sizeof(ebcdic_response) - 1) < 0)
    {
        strcpy(sqlstate, "38907");
        strcpy(msgtext, "Failed to convert response to EBCDIC");
        *valueInd = -1;
        close(sockfd);
        return;
    }
    ebcdic_response[total_len] = '\0';

    // Parse the RESP array response into comma-separated values
    int parse_result = parse_mget_response(ebcdic_response, value, 16370);

    if (parse_result > 0)
    {
        *valueInd = 0; // Success with data
    }
    else if (parse_result == -2 || parse_result == 0)
    {
        // Empty result
        strncpy(sqlstate, "02000", 5);
        strncpy(msgtext, "No keys found or empty result", 70);
        *valueInd = -1;
    }
    else
    {
        strncpy(sqlstate, "38909", 5);
        snprintf(msgtext, 70, "Failed to parse MGET response");
        *valueInd = -1;
    }

    close(sockfd);
}

#pragma linkage(mgetRedisValues, OS)
