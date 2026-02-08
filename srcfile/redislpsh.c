/******************************************************************************
 * File: redislpsh.c
 * Author: Ernest Rozloznik (e@er400.io)
 * Date: 2025-02-22
 * Description: Implementation of the Redis LPUSH function for IBM i.
 *              Pushes a value to the head of a Redis list.
 *              Returns the length of the list after the push operation.
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
/* SQL External Function: LPUSH */
/**********************************************************************/

void SQL_API_FN lpushRedisList(
	SQLUDF_VARCHAR *key,		 // Input: Redis key (VARCHAR(255), EBCDIC)
	SQLUDF_VARCHAR *value,		 // Input: Value to push (VARCHAR(16370), EBCDIC)
	SQLUDF_BIGINT *result,		 // Output: list length after push (BIGINT)
	SQLUDF_NULLIND *keyInd,		 // Null indicator for key
	SQLUDF_NULLIND *valueInd,	 // Null indicator for value
	SQLUDF_NULLIND *resultInd,	 // Null indicator for output result
	char *sqlstate,
	char *funcname,
	char *specname,
	char *msgtext,
	short *sqlcode,
	SQLUDF_NULLIND *nullind)
{
	int sockfd;
	char ebcdic_send_buf[33000], ascii_send_buf[33000], recv_buf[1024], ebcdic_payload[1024];
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

	// Initialize
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

	*result = 0;
	*resultInd = 0;

	// Connect to Redis
	if (connect_to_redis(&sockfd) != 0)
	{
		strncpy(sqlstate, "38901", 5);
		snprintf(msgtext, 70, "Failed to connect to Redis: errno=%d", errno);
		*resultInd = -1;
		close(sockfd);
		return;
	}

	ebcdic_send_buf[0] = '\0';

	// Format key length in EBCDIC
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

	// Format value length in EBCDIC
	int value_len = strlen(value);
	if (value_len > 16370)
		value_len = 16370;
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

	// Build LPUSH command: "*3\r\n$5\r\nLPUSH\r\n$<key_len>\r\n<key>\r\n$<value_len>\r\n<value>\r\n"
	strcat(ebcdic_send_buf, "\x5C\xF3\x0D\x25\x5B\xF5\x0D\x25\xD3\xD7\xE4\xE2\xC8\x0D\x25"); // *3\r\n$5\r\nLPUSH\r\n
	strcat(ebcdic_send_buf, "\x5B");
	strcat(ebcdic_send_buf, ebcdic_key_len);
	strcat(ebcdic_send_buf, "\x0D\x25");
	strncat(ebcdic_send_buf, key, key_len);
	strcat(ebcdic_send_buf, "\x0D\x25\x5B");
	strcat(ebcdic_send_buf, ebcdic_value_len);
	strcat(ebcdic_send_buf, "\x0D\x25");
	strncat(ebcdic_send_buf, value, value_len);
	strcat(ebcdic_send_buf, "\x0D\x25");

	// Convert to ASCII
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

	// Send command
	len = send(sockfd, ascii_send_buf, strlen(ascii_send_buf), 0);
	if (len < 0)
	{
		strncpy(sqlstate, "38903", 5);
		snprintf(msgtext, 70, "Failed to send command to Redis: errno=%d", errno);
		*resultInd = -1;
		close(sockfd);
		return;
	}

	// Receive response
	len = recv(sockfd, recv_buf, sizeof(recv_buf) - 1, 0);
	if (len < 0)
	{
		strncpy(sqlstate, errno == EWOULDBLOCK || errno == EAGAIN ? "38904" : "38905", 5);
		snprintf(msgtext, 70, "Failed to receive data from Redis: errno=%d", errno);
		*resultInd = -1;
		close(sockfd);
		return;
	}
	else if (len == 0)
	{
		strncpy(sqlstate, "38906", 5);
		strncpy(msgtext, "Connection closed by Redis", 70);
		*resultInd = -1;
		close(sockfd);
		return;
	}

	total_len = len;
	recv_buf[total_len] = '\0';

	// Convert response to EBCDIC
	if (ConvertToEBCDIC(recv_buf, total_len, ebcdic_payload, sizeof(ebcdic_payload) - 1) < 0)
	{
		strncpy(sqlstate, "38907", 5);
		strncpy(msgtext, "Failed to convert response to EBCDIC", 70);
		*resultInd = -1;
		close(sockfd);
		return;
	}
	ebcdic_payload[total_len] = '\0';

	// Extract response (integer: list length)
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

#pragma linkage(lpushRedisList, OS)
