/******************************************************************************
 * File: redisdbsz.c
 * Author: Ernest Rozloznik (e@er400.io)
 * Date: 2025-02-22
 * Description: Implementation of the Redis DBSIZE function for IBM i.
 *              This function returns the number of keys in the currently
 *              selected Redis database. Useful for monitoring and diagnostics.
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
/* SQL External Function: DBSIZE */
/**********************************************************************/

void SQL_API_FN dbsizeRedis(
    SQLUDF_BIGINT *value,     // Output: number of keys (BIGINT)
    SQLUDF_NULLIND *valueInd, // Null indicator for output
    char *sqlstate,           // SQLSTATE (5 chars)
    char *funcname,           // Fully qualified function name
    char *specname,           // Specific name
    char *msgtext,            // Error message text (up to 70 chars)
    short *sqlcode,           // SQLCODE
    SQLUDF_NULLIND *nullind)  // Additional null indicators for DB2SQL
{
    int sockfd;
    char ebcdic_send_buf[1024], ascii_send_buf[1024], recv_buf[1024], ebcdic_payload[1024];
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
    strcpy(sqlstate, "00000");
    *value = 0;
    *valueInd = 0;

    // Connect to Redis
    if (connect_to_redis(&sockfd) != 0)
    {
        strcpy(sqlstate, "38901");
        strcpy(msgtext, "Failed to connect to Redis");
        *valueInd = -1;
        return;
    }

    // Format Redis DBSIZE command in EBCDIC
    // RESP: *1\r\n$6\r\nDBSIZE\r\n
    // EBCDIC: * = 0x5C, 1 = 0xF1, \r\n = 0x0D 0x25
    //         $ = 0x5B, 6 = 0xF6
    //         D = 0xC4, B = 0xC2, S = 0xE2, I = 0xC9, Z = 0xE9, E = 0xC5
    ebcdic_send_buf[0] = '\0';
    strcat(ebcdic_send_buf, "\x5C\xF1\x0D\x25\x5B\xF6\x0D\x25\xC4\xC2\xE2\xC9\xE9\xC5\x0D\x25");

    // Convert EBCDIC command to ASCII
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

    // Convert response to EBCDIC
    if (ConvertToEBCDIC(recv_buf, total_len, ebcdic_payload, sizeof(ebcdic_payload) - 1) < 0)
    {
        strcpy(sqlstate, "38907");
        strcpy(msgtext, "Failed to convert response to EBCDIC");
        *valueInd = -1;
        close(sockfd);
        return;
    }
    ebcdic_payload[total_len] = '\0';

    // Extract response — DBSIZE returns an integer (:N\r\n)
    char *payload = NULL;
    size_t payload_length;
    if (extract_redis_payload(ebcdic_payload, &payload, &payload_length) == 0)
    {
        *value = atol(payload);
        *valueInd = 0;
    }
    else
    {
        strcpy(sqlstate, "38909");
        strcpy(msgtext, "Failed to extract payload from Redis response");
        *valueInd = -1;
    }

    close(sockfd);
}

#pragma linkage(dbsizeRedis, OS)
