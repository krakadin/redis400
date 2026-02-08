/******************************************************************************
 * File: redishscn.c
 * Author: Ernest Rozloznik (e@er400.io)
 * Date: 2025-02-22
 * Description: Implementation of the Redis HSCAN function for IBM i.
 *              Cursor-based scanning of hash fields matching a pattern.
 *              Safe for production use (non-blocking).
 *              Returns "cursor|field1=val1,field2=val2" format.
 *              Cursor "0" means scan is complete.
 *              Example: HSCAN('user:1', '0', 'name*', 100) → "17|name=John,nickname=JD"
 *                       HSCAN('user:1', '17', 'name*', 100) → "0|namefull=John Doe"
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
/* Helper: Convert integer to EBCDIC string                            */
/**********************************************************************/

static int int_to_ebcdic(int val, char *ebcdic_str, size_t buf_size)
{
    char ascii_str[12] = {0};
    int is_negative = 0;
    int abs_val = val;

    if (val < 0)
    {
        is_negative = 1;
        abs_val = -val;
    }

    snprintf(ascii_str, sizeof(ascii_str), "%d", abs_val);
    int digit_len = strlen(ascii_str);

    if ((size_t)(digit_len + is_negative + 1) > buf_size)
        return -1;

    int offset = 0;
    if (is_negative)
        ebcdic_str[offset++] = 0x60; // EBCDIC '-'

    int i;
    for (i = 0; i < digit_len; i++)
        ebcdic_str[offset++] = ascii_str[i] - '0' + 0xF0;

    ebcdic_str[offset] = '\0';
    return offset; // total length including sign
}

/**********************************************************************/
/* Helper: Parse nested RESP array for HSCAN                           */
/* HSCAN returns: *2\r\n$<clen>\r\n<cursor>\r\n*N\r\n...fields/vals.. */
/* Output format: "cursor|field1=val1,field2=val2"                     */
/**********************************************************************/

static int parse_hscan_response(char *ebcdic_response, char *output, size_t output_size)
{
    char *pos = ebcdic_response;
    size_t out_offset = 0;

    // Step 1: Parse outer array (*2)
    if (*pos != 0x5C) // EBCDIC '*'
        return -1;
    pos++;

    char count_buf[10];
    char *crlf = strstr(pos, "\x0D\x25"); // EBCDIC \r\n
    if (!crlf)
        return -1;

    size_t count_len = crlf - pos;
    if (count_len >= sizeof(count_buf))
        return -1;
    strncpy(count_buf, pos, count_len);
    count_buf[count_len] = '\0';

    int outer_count = atoi(count_buf);
    if (outer_count != 2)
        return -1; // HSCAN always returns 2 elements

    pos = crlf + 2; // Skip past \r\n

    // Step 2: Extract cursor (first bulk string)
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

    size_t cursor_length = atoi(len_buf);
    pos = crlf + 2; // Skip to cursor data

    // Copy cursor to output
    if (cursor_length >= output_size)
        return -1;
    strncpy(output, pos, cursor_length);
    out_offset = cursor_length;
    pos += cursor_length;

    // Skip cursor's trailing \r\n
    if (strncmp(pos, "\x0D\x25", 2) != 0)
        return -1;
    pos += 2;

    // Step 3: Append pipe separator
    if (out_offset + 1 >= output_size)
        return -1;
    output[out_offset++] = 0x4F; // EBCDIC '|'

    // Step 4: Parse inner array (*N) for field/value pairs
    if (*pos != 0x5C) // EBCDIC '*'
        return -1;
    pos++;

    crlf = strstr(pos, "\x0D\x25");
    if (!crlf)
        return -1;

    count_len = crlf - pos;
    if (count_len >= sizeof(count_buf))
        return -1;
    strncpy(count_buf, pos, count_len);
    count_buf[count_len] = '\0';

    int inner_count = atoi(count_buf);
    pos = crlf + 2; // Skip past \r\n

    if (inner_count <= 0)
    {
        // No fields in this iteration
        output[out_offset] = '\0';
        return 0; // Success, 0 pairs
    }

    // Step 5: Parse field/value pairs (inner_count is 2*N: field1,val1,field2,val2,...)
    int pair_count = 0;
    int i;
    for (i = 0; i < inner_count; i += 2)
    {
        // Parse field bulk string: $len\r\ndata\r\n
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

        size_t field_length = atoi(len_buf);
        pos = crlf + 2; // Skip to field data
        char *field_start = pos;
        pos += field_length;

        // Skip field's trailing \r\n
        if (strncmp(pos, "\x0D\x25", 2) != 0)
            return -1;
        pos += 2;

        // Parse value bulk string: $len\r\ndata\r\n
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

        size_t val_length = atoi(len_buf);
        pos = crlf + 2; // Skip to value data
        char *val_start = pos;
        pos += val_length;

        // Skip value's trailing \r\n
        if (strncmp(pos, "\x0D\x25", 2) != 0)
            return -1;
        pos += 2;

        // Append comma separator (if not first pair)
        if (pair_count > 0)
        {
            if (out_offset + 1 >= output_size)
                break;
            output[out_offset++] = 0x6B; // EBCDIC ','
        }

        // Check if we have room for field=value
        if (out_offset + field_length + 1 + val_length >= output_size)
            break;

        // Append field
        strncpy(output + out_offset, field_start, field_length);
        out_offset += field_length;

        // Append '=' separator
        output[out_offset++] = 0x7E; // EBCDIC '='

        // Append value
        strncpy(output + out_offset, val_start, val_length);
        out_offset += val_length;

        pair_count++;
    }

    output[out_offset] = '\0';
    return pair_count;
}

/**********************************************************************/
/* SQL External Function: HSCAN                                        */
/**********************************************************************/

void SQL_API_FN hscanRedisHash(
    SQLUDF_VARCHAR *key,          // Input: Redis key (VARCHAR(255), EBCDIC)
    SQLUDF_VARCHAR *cursor,       // Input: cursor string (VARCHAR(20), EBCDIC)
    SQLUDF_VARCHAR *pattern,      // Input: MATCH pattern (VARCHAR(255), EBCDIC)
    SQLUDF_INTEGER *count,        // Input: COUNT hint (INTEGER)
    SQLUDF_VARCHAR *value,        // Output: cursor|field1=val1,field2=val2 (VARCHAR(16370), EBCDIC)
    SQLUDF_NULLIND *keyInd,
    SQLUDF_NULLIND *cursorInd,
    SQLUDF_NULLIND *patternInd,
    SQLUDF_NULLIND *countInd,
    SQLUDF_NULLIND *valueInd,
    char *sqlstate,
    char *funcname,
    char *specname,
    char *msgtext,
    short *sqlcode,
    SQLUDF_NULLIND *nullind)
{
    int sockfd;
    char ebcdic_send_buf[2048], ascii_send_buf[2048];
    char recv_buf[32000], ebcdic_response[32000];
    char ebcdic_key_len[10] = {0};
    char ebcdic_cursor_len[10] = {0};
    char ebcdic_pat_len[10] = {0};
    char ebcdic_count_str[12] = {0};
    char ebcdic_count_len[10] = {0};
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

    // Check for NULL inputs
    if (*keyInd < 0 || *cursorInd < 0 || *patternInd < 0 || *countInd < 0)
    {
        strcpy(sqlstate, "38001");
        strcpy(msgtext, "Input parameter is NULL");
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

    // Format key length in EBCDIC
    ebcdic_send_buf[0] = '\0';
    int key_len = strlen(key);

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

    // Format cursor length in EBCDIC
    int cursor_len = strlen(cursor);

    if (cursor_len < 10)
    {
        ebcdic_cursor_len[0] = 0xF0 + cursor_len;
        ebcdic_cursor_len[1] = '\0';
    }
    else
    {
        int i = 0, temp_len = cursor_len;
        while (temp_len > 0)
        {
            ebcdic_cursor_len[i++] = 0xF0 + (temp_len % 10);
            temp_len /= 10;
        }
        ebcdic_cursor_len[i] = '\0';
        for (int j = 0; j < i / 2; j++)
        {
            char tmp = ebcdic_cursor_len[j];
            ebcdic_cursor_len[j] = ebcdic_cursor_len[i - 1 - j];
            ebcdic_cursor_len[i - 1 - j] = tmp;
        }
    }

    // Format pattern length in EBCDIC
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

    // Format count as EBCDIC string
    int count_str_len = int_to_ebcdic(*count, ebcdic_count_str, sizeof(ebcdic_count_str));
    if (count_str_len < 0)
    {
        strcpy(sqlstate, "38902");
        strcpy(msgtext, "Failed to format COUNT parameter");
        *valueInd = -1;
        close(sockfd);
        return;
    }

    // Format count string length in EBCDIC
    if (count_str_len < 10)
    {
        ebcdic_count_len[0] = 0xF0 + count_str_len;
        ebcdic_count_len[1] = '\0';
    }
    else
    {
        int i = 0, temp_len = count_str_len;
        while (temp_len > 0)
        {
            ebcdic_count_len[i++] = 0xF0 + (temp_len % 10);
            temp_len /= 10;
        }
        ebcdic_count_len[i] = '\0';
        for (int j = 0; j < i / 2; j++)
        {
            char tmp = ebcdic_count_len[j];
            ebcdic_count_len[j] = ebcdic_count_len[i - 1 - j];
            ebcdic_count_len[i - 1 - j] = tmp;
        }
    }

    // Build HSCAN command:
    // "*7\r\n$5\r\nHSCAN\r\n$<klen>\r\n<key>\r\n$<clen>\r\n<cursor>\r\n$5\r\nMATCH\r\n$<plen>\r\n<pattern>\r\n$5\r\nCOUNT\r\n$<cnt_len>\r\n<count>\r\n"
    strcat(ebcdic_send_buf, "\x5C\xF7\x0D\x25\x5B\xF5\x0D\x25\xC8\xE2\xC3\xC1\xD5\x0D\x25"); // *7\r\n$5\r\nHSCAN\r\n
    strcat(ebcdic_send_buf, "\x5B");                                                              // $
    strcat(ebcdic_send_buf, ebcdic_key_len);                                                      // key length
    strcat(ebcdic_send_buf, "\x0D\x25");                                                          // \r\n
    strcat(ebcdic_send_buf, key);                                                                  // key value
    strcat(ebcdic_send_buf, "\x0D\x25");                                                          // \r\n
    strcat(ebcdic_send_buf, "\x5B");                                                              // $
    strcat(ebcdic_send_buf, ebcdic_cursor_len);                                                   // cursor length
    strcat(ebcdic_send_buf, "\x0D\x25");                                                          // \r\n
    strcat(ebcdic_send_buf, cursor);                                                               // cursor value
    strcat(ebcdic_send_buf, "\x0D\x25");                                                          // \r\n
    strcat(ebcdic_send_buf, "\x5B\xF5\x0D\x25\xD4\xC1\xE3\xC3\xC8\x0D\x25");                   // $5\r\nMATCH\r\n
    strcat(ebcdic_send_buf, "\x5B");                                                              // $
    strcat(ebcdic_send_buf, ebcdic_pat_len);                                                      // pattern length
    strcat(ebcdic_send_buf, "\x0D\x25");                                                          // \r\n
    strcat(ebcdic_send_buf, pattern);                                                              // pattern value
    strcat(ebcdic_send_buf, "\x0D\x25");                                                          // \r\n
    strcat(ebcdic_send_buf, "\x5B\xF5\x0D\x25\xC3\xD6\xE4\xD5\xE3\x0D\x25");                   // $5\r\nCOUNT\r\n
    strcat(ebcdic_send_buf, "\x5B");                                                              // $
    strcat(ebcdic_send_buf, ebcdic_count_len);                                                    // count length
    strcat(ebcdic_send_buf, "\x0D\x25");                                                          // \r\n
    strcat(ebcdic_send_buf, ebcdic_count_str);                                                    // count value
    strcat(ebcdic_send_buf, "\x0D\x25");                                                          // \r\n

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

    // Parse the nested RESP array response
    int parse_result = parse_hscan_response(ebcdic_response, value, 16370);

    if (parse_result >= 0)
    {
        *valueInd = 0; // Success (0 pairs is valid for HSCAN)
    }
    else
    {
        strncpy(sqlstate, "38909", 5);
        snprintf(msgtext, 70, "Failed to parse HSCAN response");
        *valueInd = -1;
    }

    close(sockfd);
}

#pragma linkage(hscanRedisHash, OS)
