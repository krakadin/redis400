/******************************************************************************
 * File: redishext.c
 * Author: Ernest Rozloznik (e@er400.io)
 * Date: 2025-02-22
 * Description: Implementation of the Redis HEXISTS function for IBM i.
 *              This function checks if a field exists in a Redis hash.
 *              Returns 1 if the field exists, 0 if it does not.
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
/* SQL External Function: HEXISTS */
/**********************************************************************/

/**
 * Function: hexistsRedisField
 * Description: SQL external function to check if a field exists in a Redis hash.
 * Parameters:
 *   - key: Input Redis key (VARCHAR(255), EBCDIC).
 *   - field: Input hash field name (VARCHAR(255), EBCDIC).
 *   - result: Output result (BIGINT, 1=exists, 0=not exists).
 *   - keyInd: Null indicator for the key.
 *   - fieldInd: Null indicator for the field.
 *   - resultInd: Null indicator for the output result.
 *   - sqlstate: SQLSTATE (5 chars, e.g., "00000").
 *   - funcname: Fully qualified function name.
 *   - specname: Specific name.
 *   - msgtext: Error message text (up to 70 chars).
 *   - sqlcode: SQLCODE (optional, not used here).
 *   - nullind: Additional null indicators for DB2SQL.
 */
void SQL_API_FN hexistsRedisField(
	SQLUDF_VARCHAR *key,		 // Input: Redis key (VARCHAR(255), EBCDIC)
	SQLUDF_VARCHAR *field,		 // Input: Hash field name (VARCHAR(255), EBCDIC)
	SQLUDF_BIGINT *result,		 // Output: 1 if exists, 0 if not (BIGINT)
	SQLUDF_NULLIND *keyInd,		 // Null indicator for key
	SQLUDF_NULLIND *fieldInd,	 // Null indicator for field
	SQLUDF_NULLIND *resultInd,	 // Null indicator for output result
	char *sqlstate,				 // SQLSTATE (5 chars, e.g., "00000")
	char *funcname,				 // Fully qualified function name
	char *specname,				 // Specific name
	char *msgtext,				 // Error message text (up to 70 chars)
	short *sqlcode,				 // SQLCODE (optional, not used here)
	SQLUDF_NULLIND *nullind)	 // Additional null indicators for DB2SQL
{
	int sockfd;
	char ebcdic_send_buf[1024], ascii_send_buf[1024], recv_buf[1024], ebcdic_payload[1024];
	char ebcdic_key_len[10] = {0}, ebcdic_field_len[10] = {0};
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
	if (*keyInd == -1 || *fieldInd == -1)
	{
		strncpy(sqlstate, "38001", 5);
		strncpy(msgtext, "Input key or field is NULL", 70);
		*resultInd = -1;
		return;
	}

	// Initialize result
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

	// Format Redis HEXISTS command in EBCDIC
	ebcdic_send_buf[0] = '\0';

	// Calculate and format key length in EBCDIC
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

	// Calculate and format field length in EBCDIC
	int field_len = strlen(field);
	if (field_len > 255)
		field_len = 255;
	if (field_len < 10)
	{
		ebcdic_field_len[0] = 0xF0 + field_len;
		ebcdic_field_len[1] = '\0';
	}
	else
	{
		int i = 0, temp_len = field_len;
		while (temp_len > 0)
		{
			ebcdic_field_len[i++] = 0xF0 + (temp_len % 10);
			temp_len /= 10;
		}
		ebcdic_field_len[i] = '\0';
		for (int j = 0; j < i / 2; j++)
		{
			char tmp = ebcdic_field_len[j];
			ebcdic_field_len[j] = ebcdic_field_len[i - 1 - j];
			ebcdic_field_len[i - 1 - j] = tmp;
		}
	}

	// Build HEXISTS command in EBCDIC:
	// 	"*3\r\n$7\r\nHEXISTS\r\n$<key_len>\r\n<key>\r\n$<field_len>\r\n<field>\r\n"
	strcat(ebcdic_send_buf, "\x5C\xF3\x0D\x25\x5B\xF7\x0D\x25\xC8\xC5\xE7\xC9\xE2\xE3\xE2\x0D\x25"); // EBCDIC "*3\r\n$7\r\nHEXISTS\r\n"
	strcat(ebcdic_send_buf, "\x5B");
	strcat(ebcdic_send_buf, ebcdic_key_len);
	strcat(ebcdic_send_buf, "\x0D\x25");
	strncat(ebcdic_send_buf, key, key_len);
	strcat(ebcdic_send_buf, "\x0D\x25\x5B");
	strcat(ebcdic_send_buf, ebcdic_field_len);
	strcat(ebcdic_send_buf, "\x0D\x25");
	strncat(ebcdic_send_buf, field, field_len);
	strcat(ebcdic_send_buf, "\x0D\x25");

	// Convert EBCDIC HEXISTS command to ASCII before sending
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

	// Send HEXISTS command to Redis
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
		// Parse the integer value from the payload (1=exists, 0=not exists)
		*result = atol(payload);
		*resultInd = 0;
	}
	else
	{
		strncpy(sqlstate, "38909", 5);
		snprintf(msgtext, 512, "Failed to extract payload from Redis response: EBCDIC=%.462s...", ebcdic_payload);
		*resultInd = -1;
	}

	close(sockfd);
}

#pragma linkage(hexistsRedisField, OS)
