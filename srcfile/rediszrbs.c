/******************************************************************************
 * File: rediszrbs.c
 * Author: Ernest Rozloznik (e@er400.io)
 * Date: 2025-02-22
 * Description: Implementation of the Redis ZRANGEBYSCORE function for IBM i.
 *              Returns members from a sorted set with scores between min and
 *              max as a comma-separated string. Supports special values like
 *              "-inf", "+inf", and exclusive ranges like "(1.5".
 *              Example output: "member1,member2,member3"
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
/* Helper: Parse RESP array for ZRANGEBYSCORE                         */
/**********************************************************************/

static int parse_zrangebyscore_response(char *ebcdic_response, char *output, size_t output_size)
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
        return -2; // Empty range

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
/* SQL External Function: ZRANGEBYSCORE                               */
/**********************************************************************/

/**
 * Function: zrangebyscoreRedisSSet
 * Description: SQL external function to get members from a Redis sorted set
 *              with scores between min and max.
 * Parameters:
 *   - key: Input Redis key (VARCHAR(255), EBCDIC).
 *   - minval: Input minimum score (VARCHAR(50), EBCDIC) - supports "-inf", "(1.5".
 *   - maxval: Input maximum score (VARCHAR(50), EBCDIC) - supports "+inf", "(5.0".
 *   - value: Output comma-separated members (VARCHAR(16370), EBCDIC).
 *   - keyInd: Null indicator for the key.
 *   - minInd: Null indicator for the min.
 *   - maxInd: Null indicator for the max.
 *   - valueInd: Null indicator for the output value.
 *   - sqlstate: SQLSTATE (5 chars, e.g., "00000").
 *   - funcname: Fully qualified function name.
 *   - specname: Specific name.
 *   - msgtext: Error message text (up to 70 chars).
 *   - sqlcode: SQLCODE (optional, not used here).
 *   - nullind: Additional null indicators for DB2SQL.
 */
void SQL_API_FN zrangebyscoreRedisSSet(
    SQLUDF_VARCHAR *key,          // Input: Redis key (VARCHAR(255), EBCDIC)
    SQLUDF_VARCHAR *minval,       // Input: min score (VARCHAR(50), EBCDIC)
    SQLUDF_VARCHAR *maxval,       // Input: max score (VARCHAR(50), EBCDIC)
    SQLUDF_VARCHAR *value,        // Output: comma-separated members (VARCHAR(16370), EBCDIC)
    SQLUDF_NULLIND *keyInd,
    SQLUDF_NULLIND *minInd,
    SQLUDF_NULLIND *maxInd,
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
    char ebcdic_key_len[10] = {0}, ebcdic_min_len[10] = {0}, ebcdic_max_len[10] = {0};
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

    // Initialize SQLSTATE to success
    strncpy(sqlstate, "00000", 5);
    sqlstate[5] = '\0';
    msgtext[0] = '\0';

    // Check for NULL inputs
    if (*keyInd == -1 || *minInd == -1 || *maxInd == -1)
    {
        strncpy(sqlstate, "38001", 5);
        strncpy(msgtext, "Input key, min, or max is NULL", 70);
        *valueInd = -1;
        return;
    }

    // Initialize
    value[0] = '\0';
    *valueInd = 0;

    // Connect to Redis
    if (connect_to_redis(&sockfd) != 0)
    {
        strncpy(sqlstate, "38901", 5);
        snprintf(msgtext, 70, "Failed to connect to Redis: errno=%d", errno);
        *valueInd = -1;
        close(sockfd);
        return;
    }

    // Format Redis ZRANGEBYSCORE command in EBCDIC
    ebcdic_send_buf[0] = '\0';

    // Calculate and format key length in EBCDIC
    int key_len = strlen(key);
    if (key_len > 255)
        key_len = 255;
    if (key_len < 10)
    {
        ebcdic_key_len[0] = 0xF0 + key_len;
        ebcdic_key_len[1] = '\0';
    }
    else
    {
        int i = 0, temp_len = key_len;
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

    // Calculate and format min length in EBCDIC
    int min_len = strlen(minval);
    if (min_len > 50)
        min_len = 50;
    if (min_len < 10)
    {
        ebcdic_min_len[0] = 0xF0 + min_len;
        ebcdic_min_len[1] = '\0';
    }
    else
    {
        int i = 0, temp_len = min_len;
        while (temp_len > 0)
        {
            ebcdic_min_len[i++] = 0xF0 + (temp_len % 10);
            temp_len /= 10;
        }
        ebcdic_min_len[i] = '\0';
        for (int j = 0; j < i / 2; j++)
        {
            char tmp = ebcdic_min_len[j];
            ebcdic_min_len[j] = ebcdic_min_len[i - 1 - j];
            ebcdic_min_len[i - 1 - j] = tmp;
        }
    }

    // Calculate and format max length in EBCDIC
    int max_len = strlen(maxval);
    if (max_len > 50)
        max_len = 50;
    if (max_len < 10)
    {
        ebcdic_max_len[0] = 0xF0 + max_len;
        ebcdic_max_len[1] = '\0';
    }
    else
    {
        int i = 0, temp_len = max_len;
        while (temp_len > 0)
        {
            ebcdic_max_len[i++] = 0xF0 + (temp_len % 10);
            temp_len /= 10;
        }
        ebcdic_max_len[i] = '\0';
        for (int j = 0; j < i / 2; j++)
        {
            char tmp = ebcdic_max_len[j];
            ebcdic_max_len[j] = ebcdic_max_len[i - 1 - j];
            ebcdic_max_len[i - 1 - j] = tmp;
        }
    }

    // Build ZRANGEBYSCORE command in EBCDIC:
    // "*4\r\n$13\r\nZRANGEBYSCORE\r\n$<key_len>\r\n<key>\r\n$<min_len>\r\n<min>\r\n$<max_len>\r\n<max>\r\n"
    strcat(ebcdic_send_buf, "\x5C\xF4\x0D\x25\x5B\xF1\xF3\x0D\x25\xE9\xD9\xC1\xD5\xC7\xC5\xC2\xE8\xE2\xC3\xD6\xD9\xC5\x0D\x25"); // *4\r\n$13\r\nZRANGEBYSCORE\r\n
    strcat(ebcdic_send_buf, "\x5B");
    strcat(ebcdic_send_buf, ebcdic_key_len);
    strcat(ebcdic_send_buf, "\x0D\x25");
    strncat(ebcdic_send_buf, key, key_len);
    strcat(ebcdic_send_buf, "\x0D\x25\x5B");
    strcat(ebcdic_send_buf, ebcdic_min_len);
    strcat(ebcdic_send_buf, "\x0D\x25");
    strncat(ebcdic_send_buf, minval, min_len);
    strcat(ebcdic_send_buf, "\x0D\x25\x5B");
    strcat(ebcdic_send_buf, ebcdic_max_len);
    strcat(ebcdic_send_buf, "\x0D\x25");
    strncat(ebcdic_send_buf, maxval, max_len);
    strcat(ebcdic_send_buf, "\x0D\x25");

    // Convert EBCDIC ZRANGEBYSCORE command to ASCII before sending
    size_t ebcdic_len_size = strlen(ebcdic_send_buf);
    if (ConvertToASCII(ebcdic_send_buf, ebcdic_len_size, ascii_send_buf, sizeof(ascii_send_buf) - 1) < 0)
    {
        strncpy(sqlstate, "38902", 5);
        strncpy(msgtext, "Failed to convert command to ASCII", 70);
        *valueInd = -1;
        close(sockfd);
        return;
    }
    ascii_send_buf[ebcdic_len_size] = '\0';

    // Send ZRANGEBYSCORE command to Redis
    len = send(sockfd, ascii_send_buf, strlen(ascii_send_buf), 0);
    if (len < 0)
    {
        strncpy(sqlstate, "38903", 5);
        snprintf(msgtext, 70, "Failed to send command to Redis: errno=%d", errno);
        *valueInd = -1;
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
        *valueInd = -1;
        close(sockfd);
        return;
    }
    else if (len == 0)
    {
        strncpy(sqlstate, "38906", 5);
        snprintf(msgtext, 70, "Connection closed by Redis, socket=%d", sockfd);
        *valueInd = -1;
        close(sockfd);
        return;
    }
    total_len = len;
    recv_buf[total_len] = '\0';

    // Convert ASCII response to EBCDIC
    if (ConvertToEBCDIC(recv_buf, total_len, ebcdic_response, sizeof(ebcdic_response) - 1) < 0)
    {
        strncpy(sqlstate, "38907", 5);
        strncpy(msgtext, "Failed to convert response to EBCDIC", 70);
        *valueInd = -1;
        close(sockfd);
        return;
    }
    ebcdic_response[total_len] = '\0';

    // Parse the RESP array response into comma-separated values
    int parse_result = parse_zrangebyscore_response(ebcdic_response, value, 16370);

    if (parse_result > 0)
    {
        *valueInd = 0; // Success with data
    }
    else if (parse_result == -2 || parse_result == 0)
    {
        // Empty range or no matching members
        strncpy(sqlstate, "02000", 5);
        strncpy(msgtext, "No members in score range", 70);
        *valueInd = -1;
    }
    else
    {
        strncpy(sqlstate, "38909", 5);
        snprintf(msgtext, 70, "Failed to parse ZRANGEBYSCORE response");
        *valueInd = -1;
    }

    close(sockfd);
}

#pragma linkage(zrangebyscoreRedisSSet, OS)
