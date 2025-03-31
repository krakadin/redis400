/******************************************************************************
 * File: redisutils.c
 * Author: Ernest Rozloznik (e@er400.io)
 * Date: 2025-02-22
 * Description: Utility functions for interacting with a Redis server.
 *              This module provides helper functions for connecting to Redis,
 *              sending commands, and handling responses. It is designed to
 *              be used in conjunction with the redisget and redisset modules.
 * License: MIT (https://opensource.org/licenses/MIT)
 * Version: 1.0.0
 ******************************************************************************/

// #define USE_ICONV 1
#include "redis_utils.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#define _XOPEN_SOURCE 520
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <sys/time.h>
#ifdef USE_ICONV
#include <qtqiconv.h>
#endif

/**********************************************************************/
/* Conversion Functions */
/**********************************************************************/

/**
 * Function: ConvertToEBCDIC
 * Description: Converts a buffer from ASCII to EBCDIC encoding.
 * Parameters:
 *   - ibuf: Input buffer containing ASCII data.
 *   - ileft: Length of the input buffer.
 *   - obuf: Output buffer to store EBCDIC data.
 *   - oleft: Length of the output buffer.
 * Returns:
 *   - 0 on success, non-zero on failure.
 */
int ConvertToEBCDIC(char *ibuf, size_t ileft, char *obuf, size_t oleft)
{
    int rc;
#ifdef USE_ICONV
    rc = iconv(ecd, (const char **)&ibuf, &ileft, &obuf, &oleft);
#else
    rc = Translate((uchar *)ibuf, ileft, (uchar *)obuf, EbcdicTable);
#endif
    return rc;
}

/**
 * Function: ConvertToASCII
 * Description: Converts a buffer from EBCDIC to ASCII encoding.
 * Parameters:
 *   - ibuf: Input buffer containing EBCDIC data.
 *   - ileft: Length of the input buffer.
 *   - obuf: Output buffer to store ASCII data.
 *   - oleft: Length of the output buffer.
 * Returns:
 *   - 0 on success, non-zero on failure.
 */
int ConvertToASCII(char *ibuf, size_t ileft, char *obuf, size_t oleft)
{
    int rc;
#ifdef USE_ICONV
    rc = iconv(acd, (const char **)&ibuf, &ileft, &obuf, &oleft);
#else
    rc = Translate((uchar *)ibuf, ileft, (uchar *)obuf, AsciiTable);
#endif
    return rc;
}

#ifndef USE_ICONV
/**
 * Function: Translate
 * Description: Translates a buffer using a conversion table.
 * Parameters:
 *   - ip: Input buffer.
 *   - ilen: Length of the input buffer.
 *   - op: Output buffer.
 *   - table: Conversion table to use.
 * Returns:
 *   - 0 on success.
 */
int Translate(uchar *ip, size_t ilen, uchar *op, uchar *table)
{
    int index;
    for (index = 0; index < ilen; ++index)
    {
        *op = table[*ip];
        ip++;
        op++;
    }
    return 0;
}
#endif

/**********************************************************************/
/* Redis Connection */
/**********************************************************************/

/**
 * Function: connect_to_redis
 * Description: Establishes a TCP connection to a Redis server.
 * Parameters:
 *   - sockfd: Pointer to store the socket file descriptor.
 * Returns:
 *   - 0 on success, negative value on failure.
 */
int connect_to_redis(int *sockfd)
{
    struct sockaddr_in server_addr;

    *sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (*sockfd < 0)
        return -1;

    struct timeval timeout;
    timeout.tv_sec = 1;
    timeout.tv_usec = 0;
    if (setsockopt(*sockfd, SOL_SOCKET, SO_RCVTIMEO, (char *)&timeout, sizeof(timeout)) < 0)
    {
        close(*sockfd);
        return -2;
    }

    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(REDIS_SERVER_PORT);
    server_addr.sin_addr.s_addr = inet_addr(REDIS_SERVER_ADDR);

    if (connect(*sockfd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0)
    {
        close(*sockfd);
        return -3;
    }
    return 0;
}

/**********************************************************************/
/* Payload Extraction */
/**********************************************************************/

/**
 * Function: extract_redis_payload
 * Description: Extracts the payload from a Redis server response.
 * Parameters:
 *   - response: The response from the Redis server.
 *   - payload_out: Pointer to store the extracted payload.
 *   - length_out: Pointer to store the length of the payload.
 * Returns:
 *   - 0 on success, negative value on failure.
 */
int extract_redis_payload(char *response, char **payload_out, size_t *length_out)
{
    // Handle simple string responses (+<string>\r\n in EBCDIC, e.g., +PONG\r\n, +OK\r\n)
    if (response[0] == 0x4E) // EBCDIC '+' for simple strings
    {
        char *value_start = response + 1; // Skip the leading '+'
        char *crlf = strstr(response, "\x0D\x25"); // Find EBCDIC CRLF
        if (!crlf)
            return -1;

        size_t value_len = crlf - value_start; // Length of the string between '+' and '\r\n'
        if (value_len > 0)
        {
            *payload_out = (char *)malloc(value_len + 1);
            if (*payload_out)
            {
                strncpy(*payload_out, value_start, value_len);
                (*payload_out)[value_len] = '\0';
                *length_out = value_len;
                return 0; // Success
            }
            return -1; // Memory allocation failure
        }
        return -1; // Invalid simple string format
    }
    // Handle GET response or other bulk replies ($<length>\r\n<payload>\r\n in EBCDIC)
    else if (response[0] == 0x5B) // EBCDIC '$' for bulk replies
    {
        // Check for (nil) response ($-1\r\n)
        if (strncmp(response, "\x5B\x60\xF1\x0D\x25", 6) == 0)
        {
            return -2; // Special return value for (nil)
        }
        char *length_str = response + 1;
        char *crlf = strstr(response, "\x0D\x25"); // EBCDIC CRLF
        if (!crlf)
            return -1;

        char length_buf[10];
        size_t length_len = crlf - length_str;
        if (length_len >= sizeof(length_buf))
            return -1;

        strncpy(length_buf, length_str, length_len);
        length_buf[length_len] = '\0';

        int payload_length = atoi(length_buf);
        if (payload_length < 0)
            return -1;

        *payload_out = crlf + 2;
        *length_out = payload_length;

        if (strncmp(*payload_out + payload_length, "\x0D\x25", 2) != 0)
            return -1;

        return 0;
    }
    // Handle INCR response (:<value>\r\n in EBCDIC)
    else if (response[0] == 0x7A) // EBCDIC ':' for integer replies
    {
        char *value_start = response + 1;
        char *crlf = strstr(response, "\x0D\x25"); // EBCDIC CRLF
        if (!crlf)
            return -1;

        size_t value_len = crlf - value_start;
        if (value_len > 0)
        {
            *payload_out = (char *)malloc(value_len + 1);
            if (*payload_out)
            {
                strncpy(*payload_out, value_start, value_len);
                (*payload_out)[value_len] = '\0';
                *length_out = value_len;
                return 0; // Success
            }
        }
        return -1; // Memory allocation failure
    }
    // Handle SET error response (-ERR ...\r\n in EBCDIC)
    else if (strncmp(response, "\x60\xC5\xD6\xD6\x0D\x25", 7) == 0) // EBCDIC "-ERR\r\n" (split hex)
    {
        const char *error_start = response + 7;          // Skip "-ERR "
        const char *crlf = strstr(response, "\x0D\x25"); // Find EBCDIC CRLF
        if (!crlf)
            return -1;

        size_t error_len = crlf - error_start;
        if (error_len > 0)
        {
            *payload_out = (char *)malloc(error_len + 1);
            if (*payload_out)
            {
                strncpy(*payload_out, error_start, error_len);
                (*payload_out)[error_len] = '\0';
                *length_out = error_len;
                return 0; // Success with error message
            }
        }
        return -1; // Memory allocation failure
    }
    return -1; // Unknown or invalid response format
}

/**********************************************************************/
/* Initialization for USE_ICONV */
/**********************************************************************/

#ifdef USE_ICONV
/**
 * Function: initialize_conversion
 * Description: Initializes iconv conversion descriptors for EBCDIC/ASCII.
 */
void initialize_conversion(void)
{
    QtqCode_T from, to;
    from.CCSID = 0; // Current job's EBCDIC (e.g., 37)
    to.CCSID = 819; // ISO 8859-1 ASCII
    errno = 0;
    acd = QtqIconvOpen(&to, &from); // EBCDIC to ASCII
    if (errno != 0)
    {
        return;
    }

    errno = 0;
    ecd = QtqIconvOpen(&from, &to); // ASCII to EBCDIC
    if (errno != 0)
    {
        iconv_close(acd);
        return;
    }
}
#endif