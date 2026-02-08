/******************************************************************************
 * File: redisrnme.c
 * Author: Ernest Rozloznik (e@er400.io)
 * Date: 2025-02-22
 * Description: Implementation of the Redis RENAME function for IBM i.
 *              Renames a key from oldkey to newkey.
 *              Returns "OK" on success, error if oldkey doesn't exist.
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
/* SQL External Function: RENAME                                      */
/**********************************************************************/

/**
 * Function: renameRedisKey
 * Description: SQL external function to rename a Redis key.
 * Parameters:
 *   - oldkey: Input old key name (VARCHAR(255), EBCDIC).
 *   - newkey: Input new key name (VARCHAR(255), EBCDIC).
 *   - response: Output Redis response (VARCHAR(128), EBCDIC).
 *   - oldkeyInd: Null indicator for the old key.
 *   - newkeyInd: Null indicator for the new key.
 *   - responseInd: Null indicator for the output response.
 *   - sqlstate: SQLSTATE (5 chars, e.g., "00000").
 *   - funcname: Fully qualified function name.
 *   - specname: Specific name.
 *   - msgtext: Error message text (up to 70 chars).
 *   - sqlcode: SQLCODE (optional, not used here).
 *   - nullind: Additional null indicators for DB2SQL.
 */
void SQL_API_FN renameRedisKey(
	SQLUDF_VARCHAR *oldkey,		 // Input: old key name (VARCHAR(255), EBCDIC)
	SQLUDF_VARCHAR *newkey,		 // Input: new key name (VARCHAR(255), EBCDIC)
	SQLUDF_VARCHAR *response,	 // Output: Redis response (VARCHAR(128), EBCDIC)
	SQLUDF_NULLIND *oldkeyInd,	 // Null indicator for old key
	SQLUDF_NULLIND *newkeyInd,	 // Null indicator for new key
	SQLUDF_NULLIND *responseInd, // Null indicator for output response
	char *sqlstate,				 // SQLSTATE (5 chars, e.g., "00000")
	char *funcname,				 // Fully qualified function name
	char *specname,				 // Specific name
	char *msgtext,				 // Error message text (up to 70 chars)
	short *sqlcode,				 // SQLCODE (optional, not used here)
	SQLUDF_NULLIND *nullind)	 // Additional null indicators for DB2SQL
{
	int sockfd;
	char ebcdic_send_buf[1024], ascii_send_buf[1024], recv_buf[1024], ebcdic_payload[1024];
	char ebcdic_oldkey_len[10] = {0}, ebcdic_newkey_len[10] = {0};
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
		*responseInd = -1;
		return;
	}
#endif

	// Initialize SQLSTATE to success
	strncpy(sqlstate, "00000", 5);
	sqlstate[5] = '\0';
	msgtext[0] = '\0';

	// Check for NULL inputs
	if (*oldkeyInd == -1 || *newkeyInd == -1)
	{
		strncpy(sqlstate, "38001", 5);
		strncpy(msgtext, "Input old key or new key is NULL", 70);
		*responseInd = -1;
		return;
	}

	// Initialize response
	response[0] = '\0';
	*responseInd = 0;

	// Connect to Redis
	if (connect_to_redis(&sockfd) != 0)
	{
		strncpy(sqlstate, "38901", 5);
		snprintf(msgtext, 70, "Failed to connect to Redis: errno=%d", errno);
		*responseInd = -1;
		close(sockfd);
		return;
	}

	// Format Redis RENAME command in EBCDIC
	ebcdic_send_buf[0] = '\0';

	// Calculate and format old key length in EBCDIC (up to 255 bytes)
	int oldkey_len = strlen(oldkey);
	if (oldkey_len > 255)
		oldkey_len = 255;
	if (oldkey_len < 10)
	{
		ebcdic_oldkey_len[0] = 0xF0 + oldkey_len;
		ebcdic_oldkey_len[1] = '\0';
	}
	else
	{
		int i = 0, temp_len = oldkey_len;
		while (temp_len > 0)
		{
			ebcdic_oldkey_len[i++] = 0xF0 + (temp_len % 10);
			temp_len /= 10;
		}
		ebcdic_oldkey_len[i] = '\0';
		for (int j = 0; j < i / 2; j++)
		{
			char tmp = ebcdic_oldkey_len[j];
			ebcdic_oldkey_len[j] = ebcdic_oldkey_len[i - 1 - j];
			ebcdic_oldkey_len[i - 1 - j] = tmp;
		}
	}

	// Calculate and format new key length in EBCDIC (up to 255 bytes)
	int newkey_len = strlen(newkey);
	if (newkey_len > 255)
		newkey_len = 255;
	if (newkey_len < 10)
	{
		ebcdic_newkey_len[0] = 0xF0 + newkey_len;
		ebcdic_newkey_len[1] = '\0';
	}
	else
	{
		int i = 0, temp_len = newkey_len;
		while (temp_len > 0)
		{
			ebcdic_newkey_len[i++] = 0xF0 + (temp_len % 10);
			temp_len /= 10;
		}
		ebcdic_newkey_len[i] = '\0';
		for (int j = 0; j < i / 2; j++)
		{
			char tmp = ebcdic_newkey_len[j];
			ebcdic_newkey_len[j] = ebcdic_newkey_len[i - 1 - j];
			ebcdic_newkey_len[i - 1 - j] = tmp;
		}
	}

	// Build RENAME command in EBCDIC:
	// 	"*3\r\n$6\r\nRENAME\r\n$<oklen>\r\n<oldkey>\r\n$<nklen>\r\n<newkey>\r\n"
	strcat(ebcdic_send_buf, "\x5C\xF3\x0D\x25\x5B\xF6\x0D\x25\xD9\xC5\xD5\xC1\xD4\xC5\x0D\x25"); // EBCDIC "*3\r\n$6\r\nRENAME\r\n"
	strcat(ebcdic_send_buf, "\x5B");
	strcat(ebcdic_send_buf, ebcdic_oldkey_len);
	strcat(ebcdic_send_buf, "\x0D\x25");
	strncat(ebcdic_send_buf, oldkey, oldkey_len);
	strcat(ebcdic_send_buf, "\x0D\x25\x5B");
	strcat(ebcdic_send_buf, ebcdic_newkey_len);
	strcat(ebcdic_send_buf, "\x0D\x25");
	strncat(ebcdic_send_buf, newkey, newkey_len);
	strcat(ebcdic_send_buf, "\x0D\x25");

	// Convert EBCDIC RENAME command to ASCII before sending
	size_t ebcdic_len_size = strlen(ebcdic_send_buf);
	if (ConvertToASCII(ebcdic_send_buf, ebcdic_len_size, ascii_send_buf, sizeof(ascii_send_buf) - 1) < 0)
	{
		strncpy(sqlstate, "38902", 5);
		strncpy(msgtext, "Failed to convert command to ASCII", 70);
		*responseInd = -1;
		close(sockfd);
		return;
	}
	ascii_send_buf[ebcdic_len_size] = '\0';

	// Send RENAME command to Redis
	len = send(sockfd, ascii_send_buf, strlen(ascii_send_buf), 0);
	if (len < 0)
	{
		strncpy(sqlstate, "38903", 5);
		snprintf(msgtext, 70, "Failed to send command to Redis: errno=%d", errno);
		*responseInd = -1;
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
		*responseInd = -1;
		close(sockfd);
		return;
	}
	else if (len == 0)
	{
		strncpy(sqlstate, "38906", 5);
		snprintf(msgtext, 70, "Connection closed by Redis, socket=%d", sockfd);
		*responseInd = -1;
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
		*responseInd = -1;
		close(sockfd);
		return;
	}
	ebcdic_payload[total_len] = '\0';

	// Extract Redis response (expects +OK or -ERR)
	char *payload = NULL;
	size_t payload_length;
	if (extract_redis_payload(ebcdic_payload, &payload, &payload_length) == 0)
	{
		if (payload_length < 128)
		{
			strncpy(response, payload, payload_length);
			response[payload_length] = '\0';
			*responseInd = 0;
		}
		else
		{
			strncpy(sqlstate, "38908", 5);
			strncpy(msgtext, "Response exceeds maximum length", 70);
			*responseInd = -1;
		}
	}
	else
	{
		strncpy(sqlstate, "38909", 5);
		snprintf(msgtext, 512, "Failed to extract payload from Redis response: EBCDIC=%.462s...", ebcdic_payload);
		*responseInd = -1;
	}

	close(sockfd);
}

#pragma linkage(renameRedisKey, OS)
