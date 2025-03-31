/******************************************************************************
 * File: redis_ping.c
 * Author: Ernest Rozloznik (e@er400.io)
 * Date: 2025-03-30
 * Description: Implementation of the Redis PING function for IBM i.
 *              This function sends a PING command to a Redis server and
 *              returns the response (PONG) in EBCDIC format.
 * License: MIT (https://opensource.org/licenses/MIT)
 * Version: 1.0.0
 ******************************************************************************/

// #define USE_ICONV 1
#include "redis_utils.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#ifdef USE_ICONV
#include <qtqiconv.h>
#endif

/**********************************************************************/
/* SQL External Function */
/**********************************************************************/

/**
 * Function: pingRedis
 * Description: SQL external function to send a PING command to Redis and retrieve the response.
 * Parameters:
 *   - value: Output Redis response (EBCDIC, expected to be "PONG", max 10 chars).
 *   - valueInd: Null indicator for the output value.
 *   - sqlstate: SQLSTATE (5 chars, e.g., "00000").
 *   - funcname: Fully qualified function name.
 *   - specname: Specific name.
 *   - msgtext: Error message text (up to 70 chars).
 *   - sqlcode: SQLCODE (optional, not used here).
 *   - nullind: Additional null indicators for DB2SQL.
 */
void SQL_API_FN pingRedis(
    SQLUDF_VARCHAR *value,    // Output: Redis response (EBCDIC, "PONG", max 10 chars)
    SQLUDF_NULLIND *valueInd, // Null indicator for output
    char *sqlstate,           // SQLSTATE (5 chars, e.g., "00000")
    char *funcname,           // Fully qualified function name
    char *specname,           // Specific name
    char *msgtext,            // Error message text (up to 70 chars)
    short *sqlcode,           // SQLCODE (optional, not used here)
    SQLUDF_NULLIND *nullind)  // Additional null indicators for DB2SQL
{
    int sockfd;
    char ebcdic_send_buf[1024], ascii_send_buf[1024], recv_buf[1024], ebcdic_payload[1024];
    int len, total_len = 0;

#ifdef USE_ICONV
    // Initialize iconv conversion descriptors if not already initialized
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

    // Format Redis PING command in EBCDIC
    ebcdic_send_buf[0] = '\0';
    // Build PING command in EBCDIC: "*1\r\n$4\r\nPING\r\n"
    strcat(ebcdic_send_buf, "\x5C\xF1\x0D\x25\x5B\xF4\x0D\x25\xD7\xC9\xD5\xC7\x0D\x25");

    // Convert EBCDIC PING command to ASCII before sending
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

    // Send PING command to Redis
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
    if (ConvertToEBCDIC(recv_buf, total_len, ebcdic_payload, sizeof(ebcdic_payload) - 1) < 0)
    {
        strcpy(sqlstate, "38907");
        strcpy(msgtext, "Failed to convert response to EBCDIC");
        *valueInd = -1;
        close(sockfd);
        return;
    }
    ebcdic_payload[total_len] = '\0';

/*     // Log EBCDIC response
    snprintf(msgtext, 70, "EBCDIC Response: %.20s...", ebcdic_payload);
    strncpy(sqlstate, "00001", 5); // Debug state
    *valueInd = 0;
    close(sockfd);
    return; */

    // Extract Redis response
    char *payload = NULL;
    size_t payload_length;
    int extract_result = extract_redis_payload(ebcdic_payload, &payload, &payload_length);

    if (extract_result == 0) // Success
    {
        if (payload_length <= 10) // Ensure it fits in VARCHAR(10)
        {
            strncpy(value, payload, payload_length);
            value[payload_length] = '\0';
            *valueInd = 0;
        }
        else
        {
            strcpy(sqlstate, "38908");
            strcpy(msgtext, "Payload exceeds maximum length of 10");
            *valueInd = -1;
        }
    }
    else if (extract_result == -2) // Specific error (timeout or not found)
    {
        strncpy(sqlstate, "02000", 5); // No data found
        strncpy(msgtext, "Redis operation timed out or no response", 70);
        *valueInd = -1;
    }
    else // All other errors
    {
        strcpy(sqlstate, "38909");
        strcpy(msgtext, "Failed to extract payload from Redis response");
        *valueInd = -1;
    }

    close(sockfd);
}

#pragma linkage(pingRedis, OS)