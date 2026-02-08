/******************************************************************************
 * File: rediszrnk.c
 * Author: Ernest Rozloznik (e@er400.io)
 * Date: 2025-02-22
 * Description: Implementation of the Redis ZRANK function for IBM i.
 *              This function gets the rank of a member in a sorted set.
 *              Returns the 0-based rank (BIGINT), or NULL if the member
 *              does not exist in the sorted set.
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
/* SQL External Function: ZRANK */
/**********************************************************************/

/**
 * Function: zrankRedisSSet
 * Description: SQL external function to get the rank of a member in a Redis sorted set.
 * Parameters:
 *   - key: Input Redis key (VARCHAR(255), EBCDIC).
 *   - member: Input sorted set member (VARCHAR(255), EBCDIC).
 *   - result: Output rank (BIGINT, 0-based), or NULL if member not found.
 *   - keyInd: Null indicator for the key.
 *   - memberInd: Null indicator for the member.
 *   - resultInd: Null indicator for the output result.
 *   - sqlstate: SQLSTATE (5 chars, e.g., "00000").
 *   - funcname: Fully qualified function name.
 *   - specname: Specific name.
 *   - msgtext: Error message text (up to 70 chars).
 *   - sqlcode: SQLCODE (optional, not used here).
 *   - nullind: Additional null indicators for DB2SQL.
 */
void SQL_API_FN zrankRedisSSet(
	SQLUDF_VARCHAR *key,		 // Input: Redis key (VARCHAR(255), EBCDIC)
	SQLUDF_VARCHAR *member,		 // Input: Sorted set member (VARCHAR(255), EBCDIC)
	SQLUDF_BIGINT *result,		 // Output: rank (BIGINT, 0-based)
	SQLUDF_NULLIND *keyInd,		 // Null indicator for key
	SQLUDF_NULLIND *memberInd,	 // Null indicator for member
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
	char ebcdic_key_len[10] = {0}, ebcdic_member_len[10] = {0};
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
	if (*keyInd == -1 || *memberInd == -1)
	{
		strncpy(sqlstate, "38001", 5);
		strncpy(msgtext, "Input key or member is NULL", 70);
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

	// Format Redis ZRANK command in EBCDIC
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

	// Calculate and format member length in EBCDIC
	int member_len = strlen(member);
	if (member_len > 255)
		member_len = 255;
	if (member_len < 10)
	{
		ebcdic_member_len[0] = 0xF0 + member_len;
		ebcdic_member_len[1] = '\0';
	}
	else
	{
		int i = 0, temp_len = member_len;
		while (temp_len > 0)
		{
			ebcdic_member_len[i++] = 0xF0 + (temp_len % 10);
			temp_len /= 10;
		}
		ebcdic_member_len[i] = '\0';
		for (int j = 0; j < i / 2; j++)
		{
			char tmp = ebcdic_member_len[j];
			ebcdic_member_len[j] = ebcdic_member_len[i - 1 - j];
			ebcdic_member_len[i - 1 - j] = tmp;
		}
	}

	// Build ZRANK command in EBCDIC:
	// 	"*3\r\n$5\r\nZRANK\r\n$<key_len>\r\n<key>\r\n$<member_len>\r\n<member>\r\n"
	strcat(ebcdic_send_buf, "\x5C\xF3\x0D\x25\x5B\xF5\x0D\x25\xE9\xD9\xC1\xD5\xD2\x0D\x25"); // EBCDIC "*3\r\n$5\r\nZRANK\r\n"
	strcat(ebcdic_send_buf, "\x5B");
	strcat(ebcdic_send_buf, ebcdic_key_len);
	strcat(ebcdic_send_buf, "\x0D\x25");
	strncat(ebcdic_send_buf, key, key_len);
	strcat(ebcdic_send_buf, "\x0D\x25\x5B");
	strcat(ebcdic_send_buf, ebcdic_member_len);
	strcat(ebcdic_send_buf, "\x0D\x25");
	strncat(ebcdic_send_buf, member, member_len);
	strcat(ebcdic_send_buf, "\x0D\x25");

	// Convert EBCDIC ZRANK command to ASCII before sending
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

	// Send ZRANK command to Redis
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
	int rc = extract_redis_payload(ebcdic_payload, &payload, &payload_length);
	if (rc == 0)
	{
		// Parse the integer value from the payload (0-based rank)
		*result = atol(payload);
		*resultInd = 0;
	}
	else if (rc == -2)
	{
		// Member not found in sorted set (nil response)
		strncpy(sqlstate, "02000", 5);
		strncpy(msgtext, "Member not found in sorted set", 70);
		*resultInd = -1;
	}
	else
	{
		strncpy(sqlstate, "38909", 5);
		snprintf(msgtext, 512, "Failed to extract payload from Redis response: EBCDIC=%.462s...", ebcdic_payload);
		*resultInd = -1;
	}

	close(sockfd);
}

#pragma linkage(zrankRedisSSet, OS)
