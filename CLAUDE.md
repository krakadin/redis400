# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Project Overview

Redis SQL Functions for IBM i - A C-based ILE service program that provides SQL User-Defined Functions (UDFs) for interacting with Redis from IBM i SQL. Enables Redis caching and storage operations directly from SQL queries.

## Architecture

```
SQL Query (IBM i)
    ↓ SQL UDF Call
REDIS400 Library
    ├─ REDISILE Service Program
    │   ├─ REDISGET Module (redisget.c)
    │   ├─ REDISSET Module (redisset.c)
    │   ├─ REDISINCR Module (redisincr.c)
    │   ├─ REDISDEL Module (redisdel.c)
    │   ├─ REDISEXP Module (redisexp.c)
    │   ├─ REDISTTL Module (redisttl.c)
    │   ├─ REDISPING Module (redisping.c)
    │   └─ REDISUTILS Module (redisutils.c)
    ↓ TCP/IP Socket
Redis Server (127.0.0.1:6379)
```

## Available SQL Functions

| Function | Description | Example |
|----------|-------------|---------|
| `REDIS_GET(key)` | Get value by key | `SELECT REDIS_GET('mykey') FROM SYSIBM.SYSDUMMY1` |
| `REDIS_SET(key, value)` | Set key-value pair | `VALUES REDIS_SET('mykey', 'myvalue')` |
| `REDIS_INCR(key)` | Increment integer value | `SET ORDER_NO = REDIS_INCR('ORDER#')` |
| `REDIS_DEL(key)` | Delete a key | `VALUES REDIS_DEL('mykey')` |
| `REDIS_EXPIRE(key, ttl)` | Set expiration (seconds) | `SELECT REDIS_EXPIRE('mykey', 300) FROM SYSIBM.SYSDUMMY1` |
| `REDIS_TTL(key)` | Get remaining TTL | `SELECT REDIS_TTL('mykey') FROM SYSIBM.SYSDUMMY1` |
| `REDIS_PING()` | Test connectivity | `VALUES(REDIS_PING())` |

## Development Commands

### Local Development (macOS)

```bash
# Edit source files locally
code srcfile/redisget.c

# Edit Redis configuration
vi .env
```

### Deploy to IBM i (P7)

```bash
# Sync files to P7
sshpass -p 'ops_api' rsync -avz --delete \
  --exclude='.git' --exclude='.DS_Store' \
  -e "ssh -o StrictHostKeyChecking=no" \
  /Users/erozloznik/Documents/osobne_projekty/v3/ \
  P7-deploy:/home/ernestr/redis400/

# Build on P7
sshpass -p 'ops_api' ssh -o StrictHostKeyChecking=no P7-deploy \
  "/QOpenSys/pkgs/bin/bash -c 'cd /home/ernestr/redis400 && gmake clean; gmake all'"
```

### Build Commands (on IBM i)

```bash
# Full build (clean + build)
gmake clean && gmake all

# Build only (assumes library doesn't exist)
gmake all

# Clean up (delete REDIS400 library)
gmake clean
```

### Testing

```bash
# Test Redis connectivity from IBM i
sshpass -p 'ops_api' ssh P7-deploy "redis-cli -h 127.0.0.1 -p 6379 PING"

# Test SQL function via ODBC
echo "VALUES(REDIS400.REDIS_PING())" | isql -v ISSI_P10 OPS_API 'ops_api'

# Test GET/SET
echo "VALUES REDIS400.REDIS_SET('test_key', 'hello world')" | isql -v ISSI_P10 OPS_API 'ops_api'
echo "SELECT REDIS400.REDIS_GET('test_key') FROM SYSIBM.SYSDUMMY1" | isql -v ISSI_P10 OPS_API 'ops_api'
```

### Redis Server Management (on IBM i)

```bash
# Start Redis
redis-server /QOpenSys/etc/redis.conf --daemonize yes

# Stop Redis
redis-cli SHUTDOWN

# Check Redis status
redis-cli PING
```

## Project Structure

```
/project-root/
├── srcfile/                 # C source files
│   ├── redisget.c          # REDIS_GET implementation
│   ├── redisset.c          # REDIS_SET implementation
│   ├── redisincr.c         # REDIS_INCR implementation
│   ├── redisdel.c          # REDIS_DEL implementation
│   ├── redisexp.c          # REDIS_EXPIRE implementation
│   ├── redisttl.c          # REDIS_TTL implementation
│   ├── redisping.c         # REDIS_PING implementation
│   └── redisutils.c        # Shared utilities (connection, conversion)
├── include/                 # Header files
│   ├── redis_utils.h       # Main header with function declarations
│   └── redis_config.h      # Generated from .env (DO NOT EDIT)
├── qsrvsrc/                 # Binding source
│   └── redisile.bnd        # Service program binding
├── .env                     # Redis configuration (IP, port)
├── generate_config.sh       # Generates redis_config.h from .env
├── makefile                 # GNU Make build script
└── CLAUDE.md               # This file
```

## Key Technical Details

### EBCDIC/ASCII Conversion
- IBM i uses EBCDIC encoding, Redis uses ASCII
- `redisutils.c` contains translation tables and conversion functions
- `ConvertToASCII()` - EBCDIC → ASCII for sending to Redis
- `ConvertToEBCDIC()` - ASCII → EBCDIC for receiving from Redis

### Redis Protocol (RESP)
- Commands formatted in RESP protocol: `*2\r\n$3\r\nGET\r\n$<len>\r\n<key>\r\n`
- TCP socket connection to Redis server
- Timeout handling for network operations

### Configuration
- Redis connection configured in `.env` file
- `generate_config.sh` creates `include/redis_config.h` at build time
- Default: `REDIS_IP=127.0.0.1`, `REDIS_PORT=6379`

### Error Handling (SQLSTATE codes)
| SQLSTATE | Description |
|----------|-------------|
| `00000` | Success |
| `02000` | Key not found / timeout |
| `38001` | NULL input |
| `38901` | Connection failed |
| `38902` | ASCII conversion failed |
| `38903` | Send failed |
| `38904` | Receive timeout |
| `38905` | Receive failed |
| `38906` | Connection closed |
| `38907` | EBCDIC conversion failed |
| `38908` | Payload too large |
| `38909` | Payload extraction failed |

## Adding a New Redis Function

1. Create new C source file in `srcfile/` (e.g., `redisappend.c`)
2. Add module compilation target in `makefile`:
   ```makefile
   redisappend.cle: redisappend.cmodule redisappend.bnd
   ```
3. Add module to service program dependencies:
   ```makefile
   redisile.srvpgm: ... redisappend.cle
   ```
4. Add SQL function creation target:
   ```makefile
   redis_append.func:
       -system "RUNSQL SQL('CREATE OR REPLACE FUNCTION $(TGT_LIB).REDIS_APPEND ...') COMMIT(*NONE)"
   ```
5. Add target to `all:` rule
6. Export function in `qsrvsrc/redisile.bnd`

## Prerequisites

- **IBM i Access**: System with ILE C compiler (5770WDS)
- **Redis Server**: Running on IBM i PASE or accessible network location
- **GNU Make**: `yum install make` in PASE
- **SSH Access**: P7-deploy host configured in ~/.ssh/config

## Troubleshooting

### Build Fails with CRLF Errors
```bash
git config --global core.autocrlf input
```

### Connection Timeout
- Verify Redis is running: `redis-cli PING`
- Check firewall/network connectivity
- Increase timeout in `connect_to_redis()` if needed

### Library Already Exists
```bash
gmake clean   # or manually: DLTLIB LIB(REDIS400)
```

### NOAUTH Authentication Required
Current version does not support Redis AUTH. Either:
- Disable `requirepass` in redis.conf
- Use unauthenticated Redis instance

## Technology Stack

- **Language**: ILE C
- **Build System**: GNU Make
- **Target**: IBM i V7.4+
- **Redis Protocol**: RESP (REdis Serialization Protocol)
- **Encoding**: EBCDIC ↔ ASCII translation tables
