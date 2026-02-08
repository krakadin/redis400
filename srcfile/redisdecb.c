/******************************************************************************
 * File: redisdecb.c
 * Author: Ernest Rozloznik (e@er400.io)
 * Date: 2025-02-22
 * Description: Implementation of the Redis DECRBY function for IBM i.
 *              This function decrements the integer value of a key by a
 *              specified amount. Returns the new value after decrementing.
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
/* SQL External Function: DECRBY */
/**********************************************************************/

void SQL_API_FN decrbyRedisValue(
	SQLUDF_VARCHAR *key,	   // Input: Redis key (VARCHAR(255), EBCDIC)
	SQLUDF_BIGINT *decrement,  // Input: Decrement amount (BIGINT)
	SQLUDF_BIGINT *result,	   // Output: New value after decrement (BIGINT)
	SQLUDF_NULLIND *keyInd,	   // Null indicator for key
	SQLUDF_NULLIND *decrInd,   // Null indicator for decrement
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
	char ebcdic_key_len[10] = {0}, ebcdic_decr_len[10] = {0};
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
	*result = 0;
	*resultInd = 0;

	// Check for NULL inputs
	if (*keyInd < 0 || *decrInd < 0)
	{
		strncpy(sqlstate, "38001", 5);
		strncpy(msgtext, "Input key or decrement is NULL", 70);
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

	// Format Redis DECRBY command in EBCDIC
	ebcdic_send_buf[0] = '\0';
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

	// Format decrement as EBCDIC digits
	char decr_str[21] = {0};
	snprintf(decr_str, sizeof(decr_str), "%lld", *decrement);
	int decr_len = strlen(decr_str);
	// Convert ASCII digits to EBCDIC, handle negative sign
	for (int i = 0; i < decr_len; i++)
	{
		if (decr_str[i] >= '0' && decr_str[i] <= '9')
			decr_str[i] = decr_str[i] - '0' + 0xF0;
		else if (decr_str[i] == '-')
			decr_str[i] = 0x60; // EBCDIC '-'
	}

	if (decr_len < 10)
	{
		ebcdic_decr_len[0] = 0xF0 + decr_len;
		ebcdic_decr_len[1] = '\0';
	}
	else
	{
		int i = 0, temp_len = decr_len;
		while (temp_len > 0)
		{
			ebcdic_decr_len[i++] = 0xF0 + (temp_len % 10);
			temp_len /= 10;
		}
		ebcdic_decr_len[i] = '\0';
		for (int j = 0; j < i / 2; j++)
		{
			char tmp = ebcdic_decr_len[j];
			ebcdic_decr_len[j] = ebcdic_decr_len[i - 1 - j];
			ebcdic_decr_len[i - 1 - j] = tmp;
		}
	}

	// Build DECRBY command in EBCDIC:
	// "*3\r\n$6\r\nDECRBY\r\n$<klen>\r\n<key>\r\n$<decr_len>\r\n<decr>\r\n"
	strcat(ebcdic_send_buf, "\x5C\xF3\x0D\x25\x5B\xF6\x0D\x25\xC4\xC5\xC3\xD9\xC2\xE8\x0D\x25"); // *3\r\n$6\r\nDECRBY\r\n
	strcat(ebcdic_send_buf, "\x5B");
	strcat(ebcdic_send_buf, ebcdic_key_len);
	strcat(ebcdic_send_buf, "\x0D\x25");
	strncat(ebcdic_send_buf, key, key_len);
	strcat(ebcdic_send_buf, "\x0D\x25\x5B");
	strcat(ebcdic_send_buf, ebcdic_decr_len);
	strcat(ebcdic_send_buf, "\x0D\x25");
	strcat(ebcdic_send_buf, decr_str);
	strcat(ebcdic_send_buf, "\x0D\x25");

	// Convert EBCDIC command to ASCII
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

	// Send DECRBY command to Redis
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

	// Convert ASCII response to EBCDIC
	if (ConvertToEBCDIC(recv_buf, total_len, ebcdic_payload, sizeof(ebcdic_payload) - 1) < 0)
	{
		strncpy(sqlstate, "38907", 5);
		strncpy(msgtext, "Failed to convert response to EBCDIC", 70);
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
		*result = atol(payload);
		*resultInd = 0;
	}
	else
	{
		strncpy(sqlstate, "38909", 5);
		snprintf(msgtext, 70, "Failed to extract payload from Redis response");
		*resultInd = -1;
	}

	close(sockfd);
}

#pragma linkage(decrbyRedisValue, OS)
