/******************************************************************************
 * File: redisexpire.c
 * Author: Ernest Rozloznik (e@er400.io)
 * Date: 2025-03-12
 * Description: Implementation of the Redis EXPIRE function for IBM i.
 *              This function sets an expiration time (TTL) in seconds for a Redis key.
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

/**********************************************************************/
/* SQL External Function: EXPIRE */
/**********************************************************************/

/**
 * Function: expireRedisKey
 * Description: SQL external function to set an expiration time for a Redis key.
 * Parameters:
 *   - key: Input Redis key (VARCHAR(255), EBCDIC).
 *   - ttl: Input time-to-live in seconds (INTEGER).
 *   - result: Output result (SMALLINT, 1 if expiration set, 0 if key does not exist).
 *   - keyInd: Null indicator for the input key.
 *   - ttlInd: Null indicator for the input TTL.
 *   - resultInd: Null indicator for the output result.
 *   - sqlstate: SQLSTATE (5 chars, e.g., "00000").
 *   - funcname: Fully qualified function name.
 *   - specname: Specific name.
 *   - msgtext: Error message text (up to 70 chars).
 *   - sqlcode: SQLCODE (optional, not used here).
 *   - nullind: Additional null indicators for DB2SQL.
 */
void SQL_API_FN expireRedisKey(
	SQLUDF_VARCHAR *key,	   // Input: Redis key (VARCHAR(255), EBCDIC)
	SQLUDF_INTEGER *ttl,	   // Input: TTL in seconds (INTEGER)
	SQLUDF_SMALLINT *result,   // Output: Result (SMALLINT, 1 or 0)
	SQLUDF_NULLIND *keyInd,	   // Null indicator for input key
	SQLUDF_NULLIND *ttlInd,	   // Null indicator for input TTL
	SQLUDF_NULLIND *resultInd, // Null indicator for output result
	char *sqlstate,			   // SQLSTATE (5 chars, e.g., "00000")
	char *funcname,			   // Fully qualified function name
	char *specname,			   // Specific name
	char *msgtext,			   // Error message text (up to 70 chars)
	short *sqlcode,			   // SQLCODE (optional, not used here)
	SQLUDF_NULLIND *nullind)   // Additional null indicators for DB2SQL
{
	int sockfd;
	char ebcdic_send_buf[512], ascii_send_buf[512], recv_buf[1024], ebcdic_payload[1024];
	char ebcdic_key_len[10] = {0}, ebcdic_ttl_len[10] = {0};
	int len, total_len = 0;

	// Initialize SQLSTATE to success
	strncpy(sqlstate, "00000", 5);
	sqlstate[5] = '\0';
	msgtext[0] = '\0';
	*result = 0;
	*resultInd = 0;

	// Check for NULL or invalid inputs
	if (*keyInd < 0)
	{
		strncpy(sqlstate, "38001", 5);
		strncpy(msgtext, "Input key is NULL", 70);
		*resultInd = -1;
		return;
	}
	if (*ttlInd < 0)
	{
		strncpy(sqlstate, "38002", 5);
		strncpy(msgtext, "Input TTL is NULL", 70);
		*resultInd = -1;
		return;
	}
	if (*ttl < 0)
	{
		strncpy(sqlstate, "38003", 5);
		strncpy(msgtext, "TTL must be non-negative", 70);
		*resultInd = -1;
		return;
	}

	// Connect to Redis
	if (connect_to_redis(&sockfd) != 0)
	{
		strncpy(sqlstate, "38901", 5);
		snprintf(msgtext, 70, "Failed to connect to Redis: errno=%d", errno);
		*resultInd = -1;
		return;
	}

	// Format Redis EXPIRE command in EBCDIC
	ebcdic_send_buf[0] = '\0';
	int key_len = strlen(key);
	if (key_len > 255)
		key_len = 255;							 // Truncate to match VARCHAR(255)
	int ttl_len = snprintf(NULL, 0, "%d", *ttl); // Get TTL string length

	// Format key length in EBCDIC
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

	// Format TTL length in EBCDIC
	if (ttl_len < 10)
	{
		ebcdic_ttl_len[0] = 0xF0 + ttl_len;
		ebcdic_ttl_len[1] = '\0';
	}
	else
	{
		int i = 0, temp_len = ttl_len;
		while (temp_len > 0)
		{
			ebcdic_ttl_len[i++] = 0xF0 + (temp_len % 10);
			temp_len /= 10;
		}
		ebcdic_ttl_len[i] = '\0';
		for (int j = 0; j < i / 2; j++)
		{
			char tmp = ebcdic_ttl_len[j];
			ebcdic_ttl_len[j] = ebcdic_ttl_len[i - 1 - j];
			ebcdic_ttl_len[i - 1 - j] = tmp;
		}
	}

	// Build EXPIRE command in EBCDIC: "*3\r\n$6\r\nEXPIRE\r\n$<key_len>\r\n<key>\r\n$<ttl_len>\r\n<ttl>\r\n"
	strcat(ebcdic_send_buf, "\x5C\xF3\x0D\x25\x5B\xF6\x0D\x25\xC5\xE7\xD7\xC9\xD9\xC5\x0D\x25"); // EBCDIC "*3\r\n$6\r\nEXPIRE\r\n"
	strcat(ebcdic_send_buf, "\x5B");
	strcat(ebcdic_send_buf, ebcdic_key_len);
	strcat(ebcdic_send_buf, "\x0D\x25");
	strncat(ebcdic_send_buf, key, key_len); // Limit to 255 bytes
	strcat(ebcdic_send_buf, "\x0D\x25\x5B");
	strcat(ebcdic_send_buf, ebcdic_ttl_len);
	strcat(ebcdic_send_buf, "\x0D\x25");
	char ttl_str[11];
	snprintf(ttl_str, sizeof(ttl_str), "%d", *ttl);
	for (int i = 0; ttl_str[i]; i++)
		ttl_str[i] = ttl_str[i] - '0' + 0xF0; // ASCII to EBCDIC digits
	strcat(ebcdic_send_buf, ttl_str);
	strcat(ebcdic_send_buf, "\x0D\x25");

	// Convert to ASCII
	ascii_send_buf[0] = '\0';
	size_t ebcdic_len_size = strlen(ebcdic_send_buf);
	if (ConvertToASCII(ebcdic_send_buf, ebcdic_len_size, ascii_send_buf, sizeof(ascii_send_buf) - 1) < 0)
	{
		strncpy(sqlstate, "38902", 5);
		strncpy(msgtext, "Failed to convert command to ASCII", 70);
		*resultInd = -1;
		close(sockfd);
		return;
	}
	ascii_send_buf[ebcdic_len_size] = '\0';

	// Send EXPIRE command to Redis
	len = send(sockfd, ascii_send_buf, strlen(ascii_send_buf), 0);
	if (len < 0)
	{
		strncpy(sqlstate, "38903", 5);
		snprintf(msgtext, 70, "Failed to send command to Redis: errno=%d", errno);
		*resultInd = -1;
		close(sockfd);
		return;
	}

	// Receive response from Redis
	len = recv(sockfd, recv_buf, sizeof(recv_buf) - 1, 0);
	if (len < 0)
	{
		strncpy(sqlstate, errno == EWOULDBLOCK || errno == EAGAIN ? "38904" : "38905", 5);
		snprintf(msgtext, 70, "Receive %s from Redis: errno=%d",
				 errno == EWOULDBLOCK || errno == EAGAIN ? "timeout" : "error", errno);
		*resultInd = -1;
		close(sockfd);
		return;
	}
	if (len == 0)
	{
		strncpy(sqlstate, "38906", 5);
		snprintf(msgtext, 70, "Connection closed by Redis");
		*resultInd = -1;
		close(sockfd);
		return;
	}

	total_len = len;
	recv_buf[total_len] = '\0';

	// Convert ASCII response to EBCDIC
	if (ConvertToEBCDIC(recv_buf, total_len, ebcdic_payload, sizeof(ebcdic_payload) - 1) < 0)
	{
		strncpy(sqlstate, "38907", 5);
		snprintf(msgtext, 70, "Failed to convert response to EBCDIC, errno=%d", errno);
		*resultInd = -1;
		close(sockfd);
		return;
	}
	ebcdic_payload[total_len] = '\0';

	// Extract Redis response
	char *payload = NULL;
	size_t payload_length;
	if (extract_redis_payload(ebcdic_payload, &payload, &payload_length) == 0)
	{
		if (payload_length == 1)
		{
			if (payload[0] == '\xF1')
				*result = 1; // EBCDIC '1'
			else if (payload[0] == '\xF0')
				*result = 0; // EBCDIC '0'
			else
			{
				strncpy(sqlstate, "38908", 5);
				snprintf(msgtext, 70, "Unexpected Redis response: %.1s", payload);
				*resultInd = -1;
			}
		}
		else
		{
			strncpy(sqlstate, "38908", 5);
			snprintf(msgtext, 70, "Invalid response length: %d", (int)payload_length);
			*resultInd = -1;
		}
		*resultInd = 0;
		free(payload);
	}
	else
	{
		strncpy(sqlstate, "38909", 5);
		snprintf(msgtext, 70, "Failed to extract payload: %.20s", ebcdic_payload);
		*resultInd = -1;
	}

	close(sockfd);
}

#pragma linkage(expireRedisKey, OS)