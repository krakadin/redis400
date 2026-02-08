/******************************************************************************
 * File: rediskeys.c
 * Author: Ernest Rozloznik (e@er400.io)
 * Date: 2025-02-22
 * Description: Implementation of the Redis KEYS function for IBM i.
 *              Returns all keys matching a pattern as a comma-separated string.
 *              Example: KEYS('user:*') → "user:1,user:2,user:3"
 *              WARNING: KEYS blocks Redis on large datasets. Use SCAN for
 *              production workloads.
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
/* Helper: Parse RESP array for KEYS                                   */
/**********************************************************************/

static int parse_keys_response(char *ebcdic_response, char *output, size_t output_size)
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
        return -2; // No matching keys

    pos = crlf + 2; // Skip past \r\n

    // Parse each element
    int i;
    for (i = 0; i < element_count; i++)
    {
        // Parse bulk string: $len\r\ndata\r\n
        if (*pos != 0x5B) // EBCDIC '$'
            return -1;
        pos++;

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

        // Append comma separator (if not first element)
        if (elem_count > 0)
        {
            if (out_offset + 1 >= output_size)
                break; // Output buffer full
            output[out_offset++] = 0x6B; // EBCDIC ','
        }

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
/* SQL External Function: KEYS                                         */
/**********************************************************************/

void SQL_API_FN keysRedisPattern(
    SQLUDF_VARCHAR *pattern,      // Input: Redis pattern (VARCHAR(255), EBCDIC)
    SQLUDF_VARCHAR *value,        // Output: comma-separated keys (VARCHAR(16370), EBCDIC)
    SQLUDF_NULLIND *patternInd,
    SQLUDF_NULLIND *valueInd,
    char *sqlstate,
    char *funcname,
    char *specname,
    char *msgtext,
    short *sqlcode,
    SQLUDF_NULLIND *nullind)
{
    int sockfd;
    char ebcdic_send_buf[1024], ascii_send_buf[1024];
    char recv_buf[32000], ebcdic_response[32000];
    char ebcdic_pat_len[10] = {0};
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
    if (*patternInd < 0)
    {
        strcpy(sqlstate, "38001");
        strcpy(msgtext, "Input pattern is NULL");
        *valueInd = -1;
        return;
    }

    // Initialize
    strcpy(sqlstate, "00000");
    value[0] = '\0';
    *valueInd = 0;

    // Connect to Redis
    if (connect_to_redis(&sockfd) != 0)
    {
        strcpy(sqlstate, "38901");
        strcpy(msgtext, "Failed to connect to Redis");
        *valueInd = -1;
        return;
    }

    // Format pattern length in EBCDIC
    ebcdic_send_buf[0] = '\0';
    int pat_len = strlen(pattern);

    if (pat_len < 10)
    {
        ebcdic_pat_len[0] = 0xF0 + pat_len;
        ebcdic_pat_len[1] = '\0';
    }
    else
    {
        int i = 0, temp_len = pat_len;
        while (temp_len > 0)
        {
            ebcdic_pat_len[i++] = 0xF0 + (temp_len % 10);
            temp_len /= 10;
        }
        ebcdic_pat_len[i] = '\0';
        for (int j = 0; j < i / 2; j++)
        {
            char tmp = ebcdic_pat_len[j];
            ebcdic_pat_len[j] = ebcdic_pat_len[i - 1 - j];
            ebcdic_pat_len[i - 1 - j] = tmp;
        }
    }

    // Build KEYS command:
    // "*2\r\n$4\r\nKEYS\r\n$<pat_len>\r\n<pattern>\r\n"
    strcat(ebcdic_send_buf, "\x5C\xF2\x0D\x25\x5B\xF4\x0D\x25\xD2\xC5\xE8\xE2\x0D\x25"); // *2\r\n$4\r\nKEYS\r\n
    strcat(ebcdic_send_buf, "\x5B");
    strcat(ebcdic_send_buf, ebcdic_pat_len);
    strcat(ebcdic_send_buf, "\x0D\x25");
    strcat(ebcdic_send_buf, pattern);
    strcat(ebcdic_send_buf, "\x0D\x25");

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

    // Parse the RESP array response into comma-separated keys
    int parse_result = parse_keys_response(ebcdic_response, value, 16370);

    if (parse_result > 0)
    {
        *valueInd = 0; // Success with data
    }
    else if (parse_result == -2 || parse_result == 0)
    {
        // No matching keys
        strncpy(sqlstate, "02000", 5);
        strncpy(msgtext, "No keys match the given pattern", 70);
        *valueInd = -1;
    }
    else
    {
        strncpy(sqlstate, "38909", 5);
        snprintf(msgtext, 70, "Failed to parse KEYS response");
        *valueInd = -1;
    }

    close(sockfd);
}

#pragma linkage(keysRedisPattern, OS)
