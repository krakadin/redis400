/******************************************************************************
 * File: redishgta.c
 * Author: Ernest Rozloznik (e@er400.io)
 * Date: 2025-02-22
 * Description: Implementation of the Redis HGETALL function for IBM i.
 *              This function retrieves all fields and values from a Redis hash.
 *              Returns a comma-separated string of field=value pairs.
 *              Example output: "name=John,age=30,city=NYC"
 *              The function is designed to be used in an ILE environment.
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
/* Helper: Parse RESP array for HGETALL                               */
/**********************************************************************/

/**
 * Function: parse_hgetall_response
 * Description: Parses a RESP array response from HGETALL into field=value pairs.
 *              RESP array format: *N\r\n$len\r\nfield1\r\n$len\r\nvalue1\r\n...
 *              Output format: "field1=value1,field2=value2,..."
 * Parameters:
 *   - ebcdic_response: The EBCDIC-converted response from Redis.
 *   - output: Output buffer for the formatted string.
 *   - output_size: Size of the output buffer.
 * Returns:
 *   - Number of field-value pairs parsed, or -1 on error, -2 for empty hash.
 */
static int parse_hgetall_response(char *ebcdic_response, char *output, size_t output_size)
{
    char *pos = ebcdic_response;
    int pair_count = 0;
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
        return -2; // Empty hash

    // HGETALL returns field-value pairs, so element_count should be even
    if (element_count % 2 != 0)
        return -1;

    pos = crlf + 2; // Skip past \r\n

    // Parse each field-value pair
    int i;
    for (i = 0; i < element_count; i += 2)
    {
        char *field_start, *value_start;
        size_t field_length, value_length;

        // Parse field (bulk string: $len\r\ndata\r\n)
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

        field_length = atoi(len_buf);
        pos = crlf + 2; // Skip past \r\n to data
        field_start = pos;
        pos += field_length;

        // Skip trailing \r\n after field data
        if (strncmp(pos, "\x0D\x25", 2) != 0)
            return -1;
        pos += 2;

        // Parse value (bulk string: $len\r\ndata\r\n)
        if (*pos != 0x5B) // EBCDIC '$'
            return -1;
        pos++;

        crlf = strstr(pos, "\x0D\x25");
        if (!crlf)
            return -1;

        len_len = crlf - pos;
        if (len_len >= sizeof(len_buf))
            return -1;
        strncpy(len_buf, pos, len_len);
        len_buf[len_len] = '\0';

        value_length = atoi(len_buf);
        pos = crlf + 2; // Skip past \r\n to data
        value_start = pos;
        pos += value_length;

        // Skip trailing \r\n after value data
        if (strncmp(pos, "\x0D\x25", 2) != 0)
            return -1;
        pos += 2;

        // Append "field=value" to output (with comma separator if not first)
        if (pair_count > 0)
        {
            if (out_offset + 1 >= output_size)
                break; // Output buffer full
            output[out_offset++] = 0x6B; // EBCDIC ','
        }

        // Check if we have room for field=value
        if (out_offset + field_length + 1 + value_length >= output_size)
            break; // Output buffer full

        strncpy(output + out_offset, field_start, field_length);
        out_offset += field_length;
        output[out_offset++] = 0x7E; // EBCDIC '='
        strncpy(output + out_offset, value_start, value_length);
        out_offset += value_length;

        pair_count++;
    }

    output[out_offset] = '\0';
    return pair_count;
}

/**********************************************************************/
/* SQL External Function: HGETALL */
/**********************************************************************/

/**
 * Function: hgetallRedis
 * Description: SQL external function to get all fields and values from a Redis hash.
 *              Returns comma-separated field=value pairs.
 * Parameters:
 *   - key: Input Redis key (VARCHAR(255), EBCDIC).
 *   - value: Output formatted string (VARCHAR(16370), EBCDIC).
 *   - keyInd: Null indicator for the input key.
 *   - valueInd: Null indicator for the output value.
 *   - sqlstate: SQLSTATE (5 chars, e.g., "00000").
 *   - funcname: Fully qualified function name.
 *   - specname: Specific name.
 *   - msgtext: Error message text (up to 70 chars).
 *   - sqlcode: SQLCODE (optional, not used here).
 *   - nullind: Additional null indicators for DB2SQL.
 */
void SQL_API_FN hgetallRedis(
    SQLUDF_VARCHAR *key,      // Input: Redis key (VARCHAR(255), EBCDIC)
    SQLUDF_VARCHAR *value,    // Output: field=value pairs (VARCHAR(16370), EBCDIC)
    SQLUDF_NULLIND *keyInd,   // Null indicator for input
    SQLUDF_NULLIND *valueInd, // Null indicator for output
    char *sqlstate,           // SQLSTATE (5 chars, e.g., "00000")
    char *funcname,           // Fully qualified function name
    char *specname,           // Specific name
    char *msgtext,            // Error message text (up to 70 chars)
    short *sqlcode,           // SQLCODE (optional, not used here)
    SQLUDF_NULLIND *nullind)  // Additional null indicators for DB2SQL
{
    int sockfd;
    char ebcdic_send_buf[1024], ascii_send_buf[1024];
    char recv_buf[32000], ebcdic_response[32000];
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

    // Check for NULL input key
    if (*keyInd < 0)
    {
        strcpy(sqlstate, "38001");
        strcpy(msgtext, "Input key is NULL");
        *valueInd = -1;
        return;
    }

    // Initialize SQLSTATE to success
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

    // Format Redis HGETALL command in EBCDIC
    ebcdic_send_buf[0] = '\0';
    int key_len = strlen(key);

    // Calculate and format key length in EBCDIC
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

    // Build HGETALL command in EBCDIC: "*2\r\n$7\r\nHGETALL\r\n$<key_len>\r\n<key>\r\n"
    strcat(ebcdic_send_buf, "\x5C\xF2\x0D\x25\x5B\xF7\x0D\x25\xC8\xC7\xC5\xE3\xC1\xD3\xD3\x0D\x25"); // EBCDIC "*2\r\n$7\r\nHGETALL\r\n"
    strcat(ebcdic_send_buf, "\x5B");
    strcat(ebcdic_send_buf, ebcdic_len);
    strcat(ebcdic_send_buf, "\x0D\x25");
    strcat(ebcdic_send_buf, key);
    strcat(ebcdic_send_buf, "\x0D\x25");

    // Convert EBCDIC HGETALL command to ASCII before sending
    ascii_send_buf[0] = '\0';
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

    // Send HGETALL command to Redis
    len = send(sockfd, ascii_send_buf, strlen(ascii_send_buf), 0);
    if (len < 0)
    {
        strcpy(sqlstate, "38903");
        strcpy(msgtext, "Failed to send command to Redis");
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

    // Convert ASCII response to EBCDIC
    if (ConvertToEBCDIC(recv_buf, total_len, ebcdic_response, sizeof(ebcdic_response) - 1) < 0)
    {
        strcpy(sqlstate, "38907");
        strcpy(msgtext, "Failed to convert response to EBCDIC");
        *valueInd = -1;
        close(sockfd);
        return;
    }
    ebcdic_response[total_len] = '\0';

    // Parse the RESP array response into field=value pairs
    int parse_result = parse_hgetall_response(ebcdic_response, value, 16370);

    if (parse_result > 0)
    {
        *valueInd = 0; // Success with data
    }
    else if (parse_result == -2 || parse_result == 0)
    {
        // Empty hash or no pairs parsed
        strncpy(sqlstate, "02000", 5);
        strncpy(msgtext, "Hash is empty or key not found", 70);
        *valueInd = -1;
    }
    else
    {
        strncpy(sqlstate, "38909", 5);
        snprintf(msgtext, 70, "Failed to parse HGETALL response");
        *valueInd = -1;
    }

    close(sockfd);
}

#pragma linkage(hgetallRedis, OS)
