/******************************************************************************
 * File: redisset.c
 * Author: Ernest Rozloznik (e@er400.io)
 * Date: 2025-02-22
 * Description: Implementation of the Redis SET function for IBM i.
 *              This module retrieves a value from a Redis server using a key.
 *              The function is designed to be used in an ILE environment.
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
/* SQL External Function: SET */
/**********************************************************************/

/**
 * Function: expireRedisKey
 * Description: SQL external function to set a value in Redis.
 * Parameters:
 *   - key: Input Redis key (VARCHAR(255), EBCDIC).
 *   - value: Input Redis value (VARCHAR(512), EBCDIC).
 *   - result: Output Redis result (VARCHAR(128), EBCDIC).
 *   - keyInd: Null indicator for the key.
 *   - valueInd: Null indicator for the value.
 *   - resultInd: Null indicator for the output result.
 *   - sqlstate: SQLSTATE (5 chars, e.g., "00000").
 *   - funcname: Fully qualified function name.
 *   - specname: Specific name.
 *   - msgtext: Error message text (up to 70 chars).
 *   - sqlcode: SQLCODE (optional, not used here).
 *   - nullind: Additional null indicators for DB2SQL.
 */
void SQL_API_FN expireRedisKey(
	SQLUDF_VARCHAR *key,		 // Input: Redis key (VARCHAR(255), EBCDIC)
	SQLUDF_VARCHAR *value,		 // Input: Redis value (VARCHAR(512), EBCDIC)
	SQLUDF_SMALLINT *result,	 // Output: Redis result (VARCHAR(128), EBCDIC)
	SQLUDF_NULLIND *keyInd,		 // Null indicator for key
	SQLUDF_NULLIND *valueInd,	 // Null indicator for value
	SQLUDF_NULLIND *resultInd, // Null indicator for output result
	char *sqlstate,				 // SQLSTATE (5 chars, e.g., "00000")
	char *funcname,				 // Fully qualified function name
	char *specname,				 // Specific name
	char *msgtext,				 // Error message text (up to 70 chars)
	short *sqlcode,				 // SQLCODE (optional, not used here)
	SQLUDF_NULLIND *nullind)	 // Additional null indicators for DB2SQL
{
	int sockfd;
	char ebcdic_send_buf[512], ascii_send_buf[512], recv_buf[1024], ebcdic_payload[1024];
	char ebcdic_key_len[10] = {0}, ebcdic_value_len[10] = {0};
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
		*resultInd = -1;
		return;
	}
#endif

	// Initialize SQLSTATE to success
	strncpy(sqlstate, "00000", 5);
	sqlstate[5] = '\0';
	msgtext[0] = '\0';

	// Check for NULL inputs
	if (*keyInd == -1 || *valueInd == -1)
	{
		strncpy(sqlstate, "38001", 5);
		strncpy(msgtext, "Input key or value is NULL", 70);
		*resultInd = -1;
		return;
	}

	// Initialize result
	result[0] = '\0';
	*resultInd = 0;

	// Connect to Redis and log
	if (connect_to_redis(&sockfd) != 0)
	{
		strncpy(sqlstate, "38901", 5);
		snprintf(msgtext, 70, "Failed to connect to Redis: errno=%d", errno);
		*resultInd = -1;
		close(sockfd);
		return;
	}
	// snprintf(msgtext, 70, "Connected to Redis socket=%d", sockfd);
	// strncpy(sqlstate, "00001", 5); // Debug state
	//*resultInd = 0;

	// Format Redis SET command in EBCDIC
	ebcdic_send_buf[0] = '\0';

	// Calculate and format key length in EBCDIC (up to 255 bytes)
	int key_len = strlen(key);
	if (key_len > 255)
		key_len = 255; // Truncate to match VARCHAR(255)
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

	// Calculate and format value length in EBCDIC (up to 512 bytes)
	int value_len = strlen(value);
	if (value_len > 512)
		value_len = 512; // Truncate to match VARCHAR(512)
	if (value_len < 10)
	{
		ebcdic_value_len[0] = 0xF0 + value_len;
		ebcdic_value_len[1] = '\0';
	}
	else
	{
		int i = 0, temp_len = value_len;
		while (temp_len > 0)
		{
			ebcdic_value_len[i++] = 0xF0 + (temp_len % 10);
			temp_len /= 10;
		}
		ebcdic_value_len[i] = '\0';
		for (int j = 0; j < i / 2; j++)
		{
			char tmp = ebcdic_value_len[j];
			ebcdic_value_len[j] = ebcdic_value_len[i - 1 - j];
			ebcdic_value_len[i - 1 - j] = tmp;
		}
	}

	// Build SET command in EBCDIC:
	// 	"*3\r\n$3\r\nSET\r\n$<key_len>\r\n<key>\r\n$<value_len>\r\n<value>\r\n"
	//  *3\r\n$6\r\nEXPIRE\r\n$<key_len>\r\n<key>\r\n$<ttl_len>\r\n<ttl>\r\n"
    strcat(ebcdic_send_buf, "\x5C\xF3\x0D\x25\x5B\xF6\x0D\x25\xC5\xE7\xD7\xC9\xD9\xC5\x0D\x25"); // EBCDIC "*3\r\n$6\r\nEXPIRE\r\n"
	strcat(ebcdic_send_buf, "\x5B");
	strcat(ebcdic_send_buf, ebcdic_key_len);
	strcat(ebcdic_send_buf, "\x0D\x25");
	strncat(ebcdic_send_buf, key, key_len); // Use strncat to limit to 255 bytes
	strcat(ebcdic_send_buf, "\x0D\x25\x5B");
	strcat(ebcdic_send_buf, ebcdic_value_len);
	strcat(ebcdic_send_buf, "\x0D\x25");
	strncat(ebcdic_send_buf, value, value_len); // Use strncat to limit to 512 bytes
	strcat(ebcdic_send_buf, "\x0D\x25");

	// Log EBCDIC command for debugging
	// snprintf(msgtext, 70, "EBCDIC Command: %.20s...", ebcdic_send_buf);
	// strncpy(sqlstate, "00001", 5); // Debug state
	//*resultInd = 0;

	// Convert EBCDIC SET command to ASCII before sending
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

	// Log ASCII command for debugging
	snprintf(msgtext, 70, "ASCII Command: %.20s...", ascii_send_buf);
	strncpy(sqlstate, "00001", 5); // Debug state
	*resultInd = 0;
    close(sockfd);
	return;
	// Send SET command to Redis
	len = send(sockfd, ascii_send_buf, strlen(ascii_send_buf), 0);
	if (len < 0)
	{
		strncpy(sqlstate, "38903", 5);
		snprintf(msgtext, 70, "Failed to send command to Redis: errno=%d", errno);
		*resultInd = -1;
		close(sockfd);
		return;
	}
	// snprintf(msgtext, 70, "Sent SET command, bytes sent=%d", len);
	// strncpy(sqlstate, "00001", 5); // Debug state
	//*resultInd = 0;

	// Receive result from Redis with detailed logging
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
		*resultInd = -1;
		close(sockfd);
		return;
	}
	else if (len == 0)
	{
		strncpy(sqlstate, "38906", 5);
		snprintf(msgtext, 70, "Connection closed by Redis, socket=%d", sockfd);
		*resultInd = -1;
		close(sockfd);
		return;
	}

	total_len = len;
	recv_buf[total_len] = '\0';

	// Log raw ASCII result
	// snprintf(msgtext, 70, "Raw ASCII Response: %.20s...", recv_buf);
	// strncpy(sqlstate, "00001", 5); // Debug state
	//*resultInd = 0;

	// Convert ASCII result to EBCDIC
	if (ConvertToEBCDIC(recv_buf, total_len, ebcdic_payload, sizeof(ebcdic_payload) - 1) < 0)
	{
		strncpy(sqlstate, "38907", 5);
		strncpy(msgtext, "Failed to convert result to EBCDIC", 70);
		*resultInd = -1;
		close(sockfd);
		return;
	}
	ebcdic_payload[total_len] = '\0';

	// Log EBCDIC result
	// snprintf(msgtext, 70, "EBCDIC Response: %.20s...", ebcdic_payload);
	// strncpy(sqlstate, "00001", 5); // Debug state
	//*resultInd = 0;

	// Extract Redis result
	char *payload = NULL;
	size_t payload_length;
	if (extract_redis_payload(ebcdic_payload, &payload, &payload_length) == 0)
	{
		if (payload_length < 128)
		{
			*result = atoi(payload); 
			*resultInd = 0;
		}
		else
		{
			strncpy(sqlstate, "38908", 5);
			strncpy(msgtext, "Response exceeds maximum length", 70);
			*resultInd = -1;
		}
	}
	else
	{
		strncpy(sqlstate, "38909", 5);
		snprintf(msgtext, 512, "Failed to extract payload from Redis result: EBCDIC=%.462s...", ebcdic_payload);
		*resultInd = -1;
	}
	//if (payload)
	//	free(payload);

	close(sockfd);
}

#pragma linkage(expireRedisKey, OS)
