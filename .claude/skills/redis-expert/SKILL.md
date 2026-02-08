# Redis Expert Agent

You are a Redis protocol and implementation expert. You understand the Redis Serialization Protocol (RESP), all Redis commands, authentication mechanisms, and how to implement Redis clients in low-level languages like C.

## RESP Protocol (Redis Serialization Protocol)

### RESP2 Data Types

| Prefix | Type | Example |
|--------|------|---------|
| `+` | Simple String | `+OK\r\n` |
| `-` | Error | `-ERR unknown command\r\n` |
| `:` | Integer | `:1000\r\n` |
| `$` | Bulk String | `$5\r\nhello\r\n` |
| `*` | Array | `*2\r\n$3\r\nfoo\r\n$3\r\nbar\r\n` |

### RESP3 Additional Types (Redis 6+)

| Prefix | Type | Example |
|--------|------|---------|
| `_` | Null | `_\r\n` |
| `,` | Double | `,3.14159\r\n` |
| `#` | Boolean | `#t\r\n` or `#f\r\n` |
| `!` | Blob Error | `!21\r\nSYNTAX invalid syntax\r\n` |
| `=` | Verbatim String | `=15\r\ntxt:Some string\r\n` |
| `(` | Big Number | `(3492890328409238509324850943850943825024385\r\n` |
| `%` | Map | `%2\r\n+first\r\n:1\r\n+second\r\n:2\r\n` |
| `~` | Set | `~3\r\n+orange\r\n+apple\r\n+banana\r\n` |
| `>` | Push | `>3\r\n+message\r\n+channel\r\n+payload\r\n` |

### Command Format

All commands are sent as RESP arrays:

```
*<number of arguments>\r\n
$<length of arg1>\r\n
<arg1>\r\n
$<length of arg2>\r\n
<arg2>\r\n
...
```

### Examples

**PING**
```
*1\r\n$4\r\nPING\r\n
```
Response: `+PONG\r\n`

**SET key value**
```
*3\r\n$3\r\nSET\r\n$5\r\nmykey\r\n$7\r\nmyvalue\r\n
```
Response: `+OK\r\n`

**GET key**
```
*2\r\n$3\r\nGET\r\n$5\r\nmykey\r\n
```
Response: `$7\r\nmyvalue\r\n` or `$-1\r\n` (nil)

**INCR key**
```
*2\r\n$4\r\nINCR\r\n$7\r\ncounter\r\n
```
Response: `:42\r\n`

**DEL key**
```
*2\r\n$3\r\nDEL\r\n$5\r\nmykey\r\n
```
Response: `:1\r\n` (number of keys deleted)

## Authentication

### Redis < 6.0 (AUTH command only)

```
AUTH password
```

RESP format:
```
*2\r\n$4\r\nAUTH\r\n$8\r\npassword\r\n
```

Response:
- Success: `+OK\r\n`
- Failure: `-ERR invalid password\r\n` or `-WRONGPASS invalid username-password pair\r\n`

### Redis 6.0+ (ACL with username/password)

```
AUTH username password
```

RESP format:
```
*3\r\n$4\r\nAUTH\r\n$7\r\ndefault\r\n$8\r\npassword\r\n
```

### HELLO Command (Redis 6.0+)

Authenticate and switch protocol version:

```
HELLO 3 AUTH username password
```

RESP format:
```
*5\r\n$5\r\nHELLO\r\n$1\r\n3\r\n$4\r\nAUTH\r\n$7\r\ndefault\r\n$8\r\npassword\r\n
```

Response (RESP3 map):
```
%7\r\n
+server\r\n+redis\r\n
+version\r\n+7.0.0\r\n
+proto\r\n:3\r\n
+id\r\n:10\r\n
+mode\r\n+standalone\r\n
+role\r\n+master\r\n
+modules\r\n*0\r\n
```

### ACL Commands (Redis 6.0+)

```
ACL LIST                    # List all users
ACL WHOAMI                  # Current user
ACL SETUSER user ...        # Create/modify user
ACL DELUSER user            # Delete user
ACL CAT [category]          # List command categories
ACL GENPASS [bits]          # Generate secure password
```

### Authentication Flow in C

```c
int authenticate_redis(int sockfd, const char *username, const char *password) {
    char send_buf[512];
    char recv_buf[256];

    // Build AUTH command
    if (username && strlen(username) > 0) {
        // Redis 6+ with username
        snprintf(send_buf, sizeof(send_buf),
            "*3\r\n$4\r\nAUTH\r\n$%zu\r\n%s\r\n$%zu\r\n%s\r\n",
            strlen(username), username,
            strlen(password), password);
    } else {
        // Redis < 6 or default user
        snprintf(send_buf, sizeof(send_buf),
            "*2\r\n$4\r\nAUTH\r\n$%zu\r\n%s\r\n",
            strlen(password), password);
    }

    // Send command
    send(sockfd, send_buf, strlen(send_buf), 0);

    // Receive response
    int len = recv(sockfd, recv_buf, sizeof(recv_buf) - 1, 0);
    recv_buf[len] = '\0';

    // Check response
    if (recv_buf[0] == '+') {
        return 0;  // Success (+OK)
    } else {
        return -1; // Error (-ERR or -WRONGPASS)
    }
}
```

## Common Redis Commands

### String Operations

| Command | Description | RESP Args |
|---------|-------------|-----------|
| `SET key value` | Set string value | 3 |
| `GET key` | Get string value | 2 |
| `INCR key` | Increment by 1 | 2 |
| `INCRBY key n` | Increment by n | 3 |
| `DECR key` | Decrement by 1 | 2 |
| `APPEND key value` | Append to string | 3 |
| `STRLEN key` | Get string length | 2 |
| `MSET k1 v1 k2 v2` | Set multiple | 2n+1 |
| `MGET k1 k2` | Get multiple | n+1 |

### Key Operations

| Command | Description | Returns |
|---------|-------------|---------|
| `DEL key` | Delete key | Integer (count) |
| `EXISTS key` | Check existence | Integer (0/1) |
| `EXPIRE key seconds` | Set TTL | Integer (0/1) |
| `TTL key` | Get remaining TTL | Integer (seconds, -1, -2) |
| `PEXPIRE key ms` | Set TTL in ms | Integer (0/1) |
| `PTTL key` | Get TTL in ms | Integer |
| `PERSIST key` | Remove expiration | Integer (0/1) |
| `TYPE key` | Get type | Simple String |
| `RENAME key newkey` | Rename key | OK |

### TTL Return Values

| Value | Meaning |
|-------|---------|
| > 0 | Remaining seconds/ms |
| -1 | Key exists, no expiration |
| -2 | Key does not exist |

### Hash Operations

| Command | Description |
|---------|-------------|
| `HSET key field value` | Set hash field |
| `HGET key field` | Get hash field |
| `HMSET key f1 v1 f2 v2` | Set multiple fields |
| `HMGET key f1 f2` | Get multiple fields |
| `HGETALL key` | Get all fields/values |
| `HDEL key field` | Delete field |
| `HEXISTS key field` | Check field exists |
| `HINCRBY key field n` | Increment field |

### List Operations

| Command | Description |
|---------|-------------|
| `LPUSH key value` | Push to head |
| `RPUSH key value` | Push to tail |
| `LPOP key` | Pop from head |
| `RPOP key` | Pop from tail |
| `LRANGE key start stop` | Get range |
| `LLEN key` | Get length |

### Set Operations

| Command | Description |
|---------|-------------|
| `SADD key member` | Add member |
| `SREM key member` | Remove member |
| `SMEMBERS key` | Get all members |
| `SISMEMBER key member` | Check membership |
| `SCARD key` | Get cardinality |

## Error Responses

| Error | Meaning |
|-------|---------|
| `-ERR unknown command` | Command not recognized |
| `-WRONGTYPE` | Wrong data type for command |
| `-NOAUTH` | Authentication required |
| `-WRONGPASS` | Invalid password |
| `-NOPERM` | No permission for command |
| `-OOM` | Out of memory |
| `-BUSY` | Server busy |
| `-READONLY` | Replica is read-only |

## Implementing New Commands

### 0. Check IBM i Naming Constraints

**CRITICAL**: IBM i module names are limited to 10 characters. The source filename (minus `.c`) becomes the module name.

- File must be: `redis` (5 chars) + abbreviated command (≤ 5 chars) + `.c`
- Example: `redisapnd.c` for APPEND (not `redisappend.c` — 11 chars exceeds limit)
- The C function name inside the file has no length limit (e.g., `appendRedisValue` is fine)
- The SQL function name also has no limit (e.g., `REDIS_APPEND` is fine)
- Only the **module/file name** is constrained

### 1. Define RESP Format

For `APPEND key value`:
```
*3\r\n$6\r\nAPPEND\r\n$<keylen>\r\n<key>\r\n$<vallen>\r\n<value>\r\n
```

### EBCDIC Conversion for RESP

On IBM i, RESP commands must be built in EBCDIC and converted to ASCII before sending.

Key EBCDIC hex mappings:
| ASCII | EBCDIC | Usage |
|-------|--------|-------|
| `*` | `\x5C` | RESP array prefix |
| `$` | `\x5B` | RESP bulk string prefix |
| `:` | `\x7A` | RESP integer prefix |
| `+` | `\x4E` | RESP simple string prefix |
| `-` | `\x60` | RESP error prefix |
| `\r` | `\x0D` | Carriage return |
| `\n` | `\x25` | Line feed (Note: EBCDIC LF ≠ ASCII LF) |
| `0`-`9` | `\xF0`-`\xF9` | Digit encoding for lengths |

When designing a new command, provide the complete EBCDIC hex string for the command prefix. For example, APPEND is:
```
"\x5C\xF3\x0D\x25\x5B\xF6\x0D\x25\xC1\xD7\xD7\xC5\xD5\xC4\x0D\x25"
 *    3    \r   \n   $    6    \r   \n   A    P    P    E    N    D    \r   \n
```

### 2. C Implementation Pattern

```c
void SQL_API_FN appendRedisValue(
    SQLUDF_VARCHAR *key,
    SQLUDF_VARCHAR *value,
    SQLUDF_BIGINT *result,      // Returns new string length
    SQLUDF_NULLIND *keyInd,
    SQLUDF_NULLIND *valueInd,
    SQLUDF_NULLIND *resultInd,
    char *sqlstate,
    char *funcname,
    char *specname,
    char *msgtext,
    short *sqlcode,
    SQLUDF_NULLIND *nullind)
{
    int sockfd;
    char send_buf[17000], recv_buf[256];

    // Validate inputs
    if (*keyInd < 0 || *valueInd < 0) {
        strcpy(sqlstate, "38001");
        strcpy(msgtext, "NULL input not allowed");
        *resultInd = -1;
        return;
    }

    // Connect (with optional auth)
    if (connect_to_redis(&sockfd) != 0) {
        strcpy(sqlstate, "38901");
        strcpy(msgtext, "Failed to connect to Redis");
        *resultInd = -1;
        return;
    }

    // Build APPEND command in EBCDIC, convert to ASCII
    // ... (similar to existing pattern)

    // Parse integer response
    if (recv_buf[0] == ':') {
        *result = atoll(recv_buf + 1);
        *resultInd = 0;
        strcpy(sqlstate, "00000");
    } else if (recv_buf[0] == '-') {
        strcpy(sqlstate, "38910");
        strncpy(msgtext, recv_buf + 1, 69);
        *resultInd = -1;
    }

    close(sockfd);
}

#pragma linkage(appendRedisValue, OS)
```

### 3. SQL Function Definition

```sql
CREATE OR REPLACE FUNCTION REDIS400.REDIS_APPEND (
    KEY VARCHAR(255),
    VALUE VARCHAR(16370)
)
RETURNS BIGINT
LANGUAGE C
SPECIFIC REDIS_APPEND
NOT DETERMINISTIC
NO SQL
RETURNS NULL ON NULL INPUT
DISALLOW PARALLEL
NOT FENCED
EXTERNAL NAME 'REDIS400/REDISILE(appendRedisValue)'
PARAMETER STYLE DB2SQL;
```

## Connection Configuration

### redis.conf Settings

```conf
# Binding
bind 127.0.0.1
port 6379

# Authentication (Redis < 6)
requirepass yourpassword

# ACL (Redis 6+)
user default on >yourpassword ~* &* +@all

# Timeout
timeout 0

# Keep-alive
tcp-keepalive 300
```

### .env Configuration

```bash
REDIS_IP=127.0.0.1
REDIS_PORT=6379
REDIS_USER=default
REDIS_PASSWORD=yourpassword
```

## Performance Considerations

1. **Connection pooling**: Reuse connections when possible
2. **Pipelining**: Send multiple commands without waiting for responses
3. **RESP3**: Use binary-safe protocol for large data
4. **Timeout tuning**: Balance between responsiveness and tolerance

## Security Best Practices

1. Always use authentication in production
2. Bind to localhost or specific IPs only
3. Use TLS for remote connections (Redis 6+)
4. Apply least-privilege ACLs
5. Disable dangerous commands (FLUSHALL, DEBUG, etc.)

## Debugging Tips

```bash
# Monitor all commands
redis-cli MONITOR

# Check server info
redis-cli INFO

# Slow log
redis-cli SLOWLOG GET 10

# Check memory
redis-cli MEMORY STATS
```
