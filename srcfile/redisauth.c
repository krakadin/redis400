/******************************************************************************
 * File: redisauth.c
 * Author: Ernest Rozloznik (e@er400.io)
 * Date: 2025-02-22
 * Description: Implementation of the Redis AUTH function for IBM i.
 *              This function authenticates with a Redis server using a password.
 *              Returns "OK" on successful authentication.
 *              The function is designed to be used in an ILE environment and
 *              interacts with a Redis server via TCP/IP.
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
/* SQL External Function */
/**********************************************************************/

/**
 * Function: authRedis
 * Description: SQL external function to authenticate with a Redis server.
 * Parameters:
 *   - password: Input Redis password (VARCHAR(255), EBCDIC).
 *   - value: Output Redis response (VARCHAR(128), EBCDIC, expected "OK").
 *   - passwordInd: Null indicator for the input password.
 *   - valueInd: Null indicator for the output value.
 *   - sqlstate: SQLSTATE (5 chars, e.g., "00000").
 *   - funcname: Fully qualified function name.
 *   - specname: Specific name.
 *   - msgtext: Error message text (up to 70 chars).
 *   - sqlcode: SQLCODE (optional, not used here).
 *   - nullind: Additional null indicators for DB2SQL.
 */
void SQL_API_FN authRedis(
    SQLUDF_VARCHAR *password, // Input: Redis password (VARCHAR(255), EBCDIC)
    SQLUDF_VARCHAR *value,    // Output: Redis response (EBCDIC, "OK", max 128 chars)
    SQLUDF_NULLIND *passwordInd, // Null indicator for input
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
    char ebcdic_len[10] = {0};
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

    // Check for NULL input password
    if (*passwordInd < 0)
    {
        strcpy(sqlstate, "38001");
        strcpy(msgtext, "Input password is NULL");
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

    // Format Redis AUTH command in EBCDIC
    ebcdic_send_buf[0] = '\0';
    int pw_len = strlen(password);

    // Calculate and format password length in EBCDIC
    if (pw_len < 10)
    {
        ebcdic_len[0] = 0xF0 + pw_len;
        ebcdic_len[1] = '\0';
    }
    else
    {
        int i = 0, temp_len = pw_len;
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

    // Build AUTH command in EBCDIC: "*2\r\n$4\r\nAUTH\r\n$<pw_len>\r\n<password>\r\n"
    strcat(ebcdic_send_buf, "\x5C\xF2\x0D\x25\x5B\xF4\x0D\x25\xC1\xE4\xE3\xC8\x0D\x25"); // EBCDIC "*2\r\n$4\r\nAUTH\r\n"
    strcat(ebcdic_send_buf, "\x5B");
    strcat(ebcdic_send_buf, ebcdic_len);
    strcat(ebcdic_send_buf, "\x0D\x25");
    strcat(ebcdic_send_buf, password);
    strcat(ebcdic_send_buf, "\x0D\x25");

    // Convert EBCDIC AUTH command to ASCII before sending
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

    // Send AUTH command to Redis
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

    // Extract Redis response
    char *payload = NULL;
    size_t payload_length;
    int extract_result = extract_redis_payload(ebcdic_payload, &payload, &payload_length);

    if (extract_result == 0) // Success
    {
        if (payload_length <= 128) // Ensure it fits in VARCHAR(128)
        {
            strncpy(value, payload, payload_length);
            value[payload_length] = '\0';
            *valueInd = 0;
        }
        else
        {
            strcpy(sqlstate, "38908");
            strcpy(msgtext, "Payload exceeds maximum length of 128");
            *valueInd = -1;
        }
    }
    else if (extract_result == -2) // Specific error (timeout or not found)
    {
        strncpy(sqlstate, "02000", 5);
        strncpy(msgtext, "Redis AUTH failed or no response", 70);
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

#pragma linkage(authRedis, OS)
