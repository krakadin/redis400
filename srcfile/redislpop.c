/******************************************************************************
 * File: redislpop.c
 * Author: Ernest Rozloznik (e@er400.io)
 * Date: 2025-02-22
 * Description: Implementation of the Redis LPOP function for IBM i.
 *              Removes and returns the first element of a Redis list.
 *              Returns NULL if the list is empty or key does not exist.
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
/* SQL External Function: LPOP */
/**********************************************************************/

void SQL_API_FN lpopRedisList(
    SQLUDF_VARCHAR *key,      // Input: Redis key (VARCHAR(255), EBCDIC)
    SQLUDF_VARCHAR *value,    // Output: popped element (VARCHAR(16370), EBCDIC)
    SQLUDF_NULLIND *keyInd,   // Null indicator for input
    SQLUDF_NULLIND *valueInd, // Null indicator for output
    char *sqlstate,
    char *funcname,
    char *specname,
    char *msgtext,
    short *sqlcode,
    SQLUDF_NULLIND *nullind)
{
    int sockfd;
    char ebcdic_send_buf[1024], ascii_send_buf[1024], recv_buf[16370], ebcdic_payload[16370];
    char ebcdic_len[10] = {0};
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

    if (*keyInd < 0)
    {
        strcpy(sqlstate, "38001");
        strcpy(msgtext, "Input key is NULL");
        *valueInd = -1;
        return;
    }

    strcpy(sqlstate, "00000");
    value[0] = '\0';
    *valueInd = 0;

    if (connect_to_redis(&sockfd) != 0)
    {
        strcpy(sqlstate, "38901");
        strcpy(msgtext, "Failed to connect to Redis");
        *valueInd = -1;
        return;
    }

    ebcdic_send_buf[0] = '\0';
    int key_len = strlen(key);

    if (key_len < 10)
    {
        ebcdic_len[0] = 0xF0 + key_len;
        ebcdic_len[1] = '\0';
    }
    else
    {
        int i = 0, temp_len = key_len;
        while (temp_len > 0)
        {
            ebcdic_len[i++] = 0xF0 + (temp_len % 10);
            temp_len /= 10;
        }
        ebcdic_len[i] = '\0';
        for (int j = 0; j < i / 2; j++)
        {
            char tmp = ebcdic_len[j];
            ebcdic_len[j] = ebcdic_len[i - 1 - j];
            ebcdic_len[i - 1 - j] = tmp;
        }
    }

    // Build LPOP command: "*2\r\n$4\r\nLPOP\r\n$<key_len>\r\n<key>\r\n"
    strcat(ebcdic_send_buf, "\x5C\xF2\x0D\x25\x5B\xF4\x0D\x25\xD3\xD7\xD6\xD7\x0D\x25"); // *2\r\n$4\r\nLPOP\r\n
    strcat(ebcdic_send_buf, "\x5B");
    strcat(ebcdic_send_buf, ebcdic_len);
    strcat(ebcdic_send_buf, "\x0D\x25");
    strcat(ebcdic_send_buf, key);
    strcat(ebcdic_send_buf, "\x0D\x25");

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

    len = send(sockfd, ascii_send_buf, strlen(ascii_send_buf), 0);
    if (len < 0)
    {
        strcpy(sqlstate, "38903");
        strcpy(msgtext, "Failed to send command to Redis");
        *valueInd = -1;
        close(sockfd);
        return;
    }

    len = recv(sockfd, recv_buf, sizeof(recv_buf) - 1, 0);
    if (len < 0)
    {
        strcpy(sqlstate, errno == EWOULDBLOCK || errno == EAGAIN ? "38904" : "38905");
        strcpy(msgtext, "Failed to receive data from Redis");
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

    if (ConvertToEBCDIC(recv_buf, total_len, ebcdic_payload, sizeof(ebcdic_payload) - 1) < 0)
    {
        strcpy(sqlstate, "38907");
        strcpy(msgtext, "Failed to convert response to EBCDIC");
        *valueInd = -1;
        close(sockfd);
        return;
    }
    ebcdic_payload[total_len] = '\0';

    char *payload = NULL;
    size_t payload_length;
    int extract_result = extract_redis_payload(ebcdic_payload, &payload, &payload_length);

    if (extract_result == 0)
    {
        if (payload_length < sizeof(ebcdic_payload))
        {
            strncpy(value, payload, payload_length);
            value[payload_length] = '\0';
            *valueInd = 0;
        }
        else
        {
            strcpy(sqlstate, "38908");
            strcpy(msgtext, "Payload exceeds maximum length");
            *valueInd = -1;
        }
    }
    else if (extract_result == -2)
    {
        strncpy(sqlstate, "02000", 5);
        strncpy(msgtext, "List is empty or key not found", 70);
        *valueInd = -1;
    }
    else
    {
        strcpy(sqlstate, "38909");
        strcpy(msgtext, "Failed to extract payload from Redis response");
        *valueInd = -1;
    }

    close(sockfd);
}

#pragma linkage(lpopRedisList, OS)
