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
    │   ├─ REDISDECR Module (redisdecr.c)
    │   ├─ REDISDEL Module (redisdel.c)
    │   ├─ REDISEXP Module (redisexp.c)
    │   ├─ REDISTTL Module (redisttl.c)
    │   ├─ REDISPING Module (redisping.c)
    │   ├─ REDISAPND Module (redisapnd.c)
    │   ├─ REDISEXST Module (redisexst.c)
    │   ├─ REDISSNX Module (redissnx.c)
    │   ├─ REDISAUTH Module (redisauth.c)
    │   ├─ REDISHSET Module (redishset.c)
    │   ├─ REDISHGET Module (redishget.c)
    │   ├─ REDISHDEL Module (redishdel.c)
    │   ├─ REDISHEXT Module (redishext.c)
    │   ├─ REDISHGTA Module (redishgta.c)
    │   ├─ REDISLPSH Module (redislpsh.c)
    │   ├─ REDISRPSH Module (redisrpsh.c)
    │   ├─ REDISLPOP Module (redislpop.c)
    │   ├─ REDISRPOP Module (redisrpop.c)
    │   ├─ REDISLLEN Module (redisllen.c)
    │   ├─ REDISLRNG Module (redislrng.c)
    │   ├─ REDISSADD Module (redissadd.c)
    │   ├─ REDISSREM Module (redissrem.c)
    │   ├─ REDISSISM Module (redissism.c)
    │   ├─ REDISSCRD Module (redisscrd.c)
    │   ├─ REDISSMEM Module (redissmem.c)
    │   ├─ REDISSETX Module (redissetx.c)
    │   ├─ REDISINCB Module (redisincb.c)
    │   ├─ REDISDECB Module (redisdecb.c)
    │   ├─ REDISPST Module (redispst.c)
    │   ├─ REDISTYPE Module (redistype.c)
    │   ├─ REDISSLEN Module (redisslen.c)
    │   ├─ REDISKEYS Module (rediskeys.c)
    │   ├─ REDISSCAN Module (redisscan.c)
    │   ├─ REDISZADD Module (rediszadd.c)
    │   ├─ REDISZREM Module (rediszrem.c)
    │   ├─ REDISZSCO Module (rediszsco.c)
    │   ├─ REDISZRNK Module (rediszrnk.c)
    │   ├─ REDISZCRD Module (rediszcrd.c)
    │   ├─ REDISZRNG Module (rediszrng.c)
    │   ├─ REDISZRBS Module (rediszrbs.c)
    │   ├─ REDISMGET Module (redismget.c)
    │   ├─ REDISMSET Module (redismset.c)
    │   ├─ REDISGSET Module (redisgset.c)
    │   ├─ REDISRNME Module (redisrnme.c)
    │   ├─ REDISHSCN Module (redishscn.c)
    │   ├─ REDISSSCN Module (redissscn.c)
    │   ├─ REDISDBSZ Module (redisdbsz.c)
    │   └─ REDISUTILS Module (redisutils.c)
    ↓ TCP/IP Socket
Redis Server (127.0.0.1:6379)
```

## Available SQL Functions

| Function | Description | Example |
|----------|-------------|---------|
| `REDIS_GET(key)` | Get value by key | `SELECT REDIS_GET('mykey') FROM SYSIBM.SYSDUMMY1` |
| `REDIS_SET(key, value)` | Set key-value pair | `VALUES REDIS_SET('mykey', 'myvalue')` |
| `REDIS_SETNX(key, value)` | Set only if key doesn't exist | `VALUES REDIS_SETNX('lock', '1')` |
| `REDIS_INCR(key)` | Increment integer value | `SET ORDER_NO = REDIS_INCR('ORDER#')` |
| `REDIS_DECR(key)` | Decrement integer value | `SELECT REDIS_DECR('stock') FROM SYSIBM.SYSDUMMY1` |
| `REDIS_DEL(key)` | Delete a key | `VALUES REDIS_DEL('mykey')` |
| `REDIS_EXISTS(key)` | Check if key exists (1/0) | `SELECT REDIS_EXISTS('mykey') FROM SYSIBM.SYSDUMMY1` |
| `REDIS_EXPIRE(key, ttl)` | Set expiration (seconds) | `SELECT REDIS_EXPIRE('mykey', 300) FROM SYSIBM.SYSDUMMY1` |
| `REDIS_TTL(key)` | Get remaining TTL | `SELECT REDIS_TTL('mykey') FROM SYSIBM.SYSDUMMY1` |
| `REDIS_APPEND(key, value)` | Append to string, returns new length | `SELECT REDIS_APPEND('mykey', 'more') FROM SYSIBM.SYSDUMMY1` |
| `REDIS_AUTH(password)` | Authenticate with Redis | `VALUES REDIS_AUTH('mypassword')` |
| `REDIS_HSET(key, field, value)` | Set hash field | `VALUES REDIS_HSET('user:1', 'name', 'John')` |
| `REDIS_HGET(key, field)` | Get hash field value | `SELECT REDIS_HGET('user:1', 'name') FROM SYSIBM.SYSDUMMY1` |
| `REDIS_HDEL(key, field)` | Delete hash field | `VALUES REDIS_HDEL('user:1', 'name')` |
| `REDIS_HEXISTS(key, field)` | Check if hash field exists (1/0) | `SELECT REDIS_HEXISTS('user:1', 'name') FROM SYSIBM.SYSDUMMY1` |
| `REDIS_HGETALL(key)` | Get all hash fields as field=value pairs | `SELECT REDIS_HGETALL('user:1') FROM SYSIBM.SYSDUMMY1` |
| `REDIS_LPUSH(key, value)` | Push to head of list, returns length | `VALUES REDIS_LPUSH('queue', 'item1')` |
| `REDIS_RPUSH(key, value)` | Push to tail of list, returns length | `VALUES REDIS_RPUSH('queue', 'item2')` |
| `REDIS_LPOP(key)` | Pop from head of list | `SELECT REDIS_LPOP('queue') FROM SYSIBM.SYSDUMMY1` |
| `REDIS_RPOP(key)` | Pop from tail of list | `SELECT REDIS_RPOP('queue') FROM SYSIBM.SYSDUMMY1` |
| `REDIS_LLEN(key)` | Get list length | `SELECT REDIS_LLEN('queue') FROM SYSIBM.SYSDUMMY1` |
| `REDIS_LRANGE(key, start, stop)` | Get range of elements (comma-separated) | `SELECT REDIS_LRANGE('queue', 0, -1) FROM SYSIBM.SYSDUMMY1` |
| `REDIS_SADD(key, member)` | Add member to set (1=added, 0=exists) | `VALUES REDIS_SADD('tags', 'redis')` |
| `REDIS_SREM(key, member)` | Remove member from set (1=removed, 0=not found) | `VALUES REDIS_SREM('tags', 'redis')` |
| `REDIS_SISMEMBER(key, member)` | Check if member exists in set (1/0) | `SELECT REDIS_SISMEMBER('tags', 'redis') FROM SYSIBM.SYSDUMMY1` |
| `REDIS_SCARD(key)` | Get number of members in set | `SELECT REDIS_SCARD('tags') FROM SYSIBM.SYSDUMMY1` |
| `REDIS_SMEMBERS(key)` | Get all members (comma-separated) | `SELECT REDIS_SMEMBERS('tags') FROM SYSIBM.SYSDUMMY1` |
| `REDIS_SETEX(key, ttl, value)` | Set key with expiration (atomic) | `VALUES REDIS_SETEX('session', 3600, 'data')` |
| `REDIS_INCRBY(key, increment)` | Increment by amount (returns new value) | `VALUES REDIS_INCRBY('counter', 10)` |
| `REDIS_DECRBY(key, decrement)` | Decrement by amount (returns new value) | `VALUES REDIS_DECRBY('counter', 3)` |
| `REDIS_PERSIST(key)` | Remove expiration from key (1/0) | `SELECT REDIS_PERSIST('mykey') FROM SYSIBM.SYSDUMMY1` |
| `REDIS_TYPE(key)` | Get key type (string/list/set/hash/zset/none) | `SELECT REDIS_TYPE('mykey') FROM SYSIBM.SYSDUMMY1` |
| `REDIS_STRLEN(key)` | Get string length (0 if key missing) | `SELECT REDIS_STRLEN('mykey') FROM SYSIBM.SYSDUMMY1` |
| `REDIS_KEYS(pattern)` | Get all keys matching pattern (comma-separated) | `SELECT REDIS_KEYS('user:*') FROM SYSIBM.SYSDUMMY1` |
| `REDIS_SCAN(cursor, pattern, count)` | Cursor-based key scan (returns cursor\|keys) | `SELECT REDIS_SCAN('0', 'user:*', 100) FROM SYSIBM.SYSDUMMY1` |
| `REDIS_ZADD(key, score, member)` | Add member with score to sorted set | `VALUES REDIS_ZADD('leaderboard', 100.0, 'player1')` |
| `REDIS_ZREM(key, member)` | Remove member from sorted set | `VALUES REDIS_ZREM('leaderboard', 'player1')` |
| `REDIS_ZSCORE(key, member)` | Get score of member in sorted set | `SELECT REDIS_ZSCORE('leaderboard', 'player1') FROM SYSIBM.SYSDUMMY1` |
| `REDIS_ZRANK(key, member)` | Get 0-based rank of member | `SELECT REDIS_ZRANK('leaderboard', 'player1') FROM SYSIBM.SYSDUMMY1` |
| `REDIS_ZCARD(key)` | Get number of members in sorted set | `SELECT REDIS_ZCARD('leaderboard') FROM SYSIBM.SYSDUMMY1` |
| `REDIS_ZRANGE(key, start, stop)` | Get range of members by index (comma-separated) | `SELECT REDIS_ZRANGE('leaderboard', 0, -1) FROM SYSIBM.SYSDUMMY1` |
| `REDIS_ZRANGEBYSCORE(key, min, max)` | Get members within score range (comma-separated) | `SELECT REDIS_ZRANGEBYSCORE('lb', '-inf', '+inf') FROM SYSIBM.SYSDUMMY1` |
| `REDIS_MGET(keys)` | Get multiple values (comma-separated keys in/out) | `SELECT REDIS_MGET('k1,k2,k3') FROM SYSIBM.SYSDUMMY1` |
| `REDIS_MSET(kvpairs)` | Set multiple key-value pairs atomically | `VALUES REDIS_MSET('k1=v1,k2=v2')` |
| `REDIS_GETSET(key, value)` | Set new value, return old value (atomic) | `SELECT REDIS_GETSET('counter', '0') FROM SYSIBM.SYSDUMMY1` |
| `REDIS_RENAME(oldkey, newkey)` | Rename a key | `VALUES REDIS_RENAME('old', 'new')` |
| `REDIS_HSCAN(key, cursor, pattern, count)` | Cursor-based hash field scan (cursor\|field=val,...) | `SELECT REDIS_HSCAN('h', '0', '*', 100) FROM SYSIBM.SYSDUMMY1` |
| `REDIS_SSCAN(key, cursor, pattern, count)` | Cursor-based set member scan (cursor\|member,...) | `SELECT REDIS_SSCAN('s', '0', '*', 100) FROM SYSIBM.SYSDUMMY1` |
| `REDIS_DBSIZE()` | Get number of keys in database | `VALUES(REDIS_DBSIZE())` |
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

# Build on P7 (PATH must include /QOpenSys/pkgs/bin for gmake)
sshpass -p 'ops_api' ssh -o StrictHostKeyChecking=no P7-deploy \
  "/QOpenSys/pkgs/bin/bash -c 'export PATH=/QOpenSys/pkgs/bin:\$PATH && cd /home/ernestr/redis400 && gmake clean; gmake all'"
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
# Run the full automated test suite (49 tests, all functions except AUTH)
bash test_all.sh

# Expected: Results: 49 passed, 0 failed (of 49 tests)

# Test individual function via ODBC
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
├── srcfile/                 # C source files (names ≤ 10 chars for IBM i!)
│   ├── redisget.c          # REDIS_GET implementation
│   ├── redisset.c          # REDIS_SET implementation
│   ├── redisincr.c         # REDIS_INCR implementation
│   ├── redisdel.c          # REDIS_DEL implementation
│   ├── redisexp.c          # REDIS_EXPIRE implementation
│   ├── redisttl.c          # REDIS_TTL implementation
│   ├── redisping.c         # REDIS_PING implementation
│   ├── redisapnd.c         # REDIS_APPEND implementation
│   ├── redisexst.c         # REDIS_EXISTS implementation
│   ├── redissnx.c          # REDIS_SETNX implementation
│   ├── redisdecr.c         # REDIS_DECR implementation
│   ├── redisauth.c         # REDIS_AUTH implementation
│   ├── redishset.c         # REDIS_HSET implementation
│   ├── redishget.c         # REDIS_HGET implementation
│   ├── redishdel.c         # REDIS_HDEL implementation
│   ├── redishext.c         # REDIS_HEXISTS implementation
│   ├── redishgta.c         # REDIS_HGETALL implementation
│   ├── redislpsh.c         # REDIS_LPUSH implementation
│   ├── redisrpsh.c         # REDIS_RPUSH implementation
│   ├── redislpop.c         # REDIS_LPOP implementation
│   ├── redisrpop.c         # REDIS_RPOP implementation
│   ├── redisllen.c         # REDIS_LLEN implementation
│   ├── redislrng.c         # REDIS_LRANGE implementation
│   ├── redissadd.c         # REDIS_SADD implementation
│   ├── redissrem.c         # REDIS_SREM implementation
│   ├── redissism.c         # REDIS_SISMEMBER implementation
│   ├── redisscrd.c         # REDIS_SCARD implementation
│   ├── redissmem.c         # REDIS_SMEMBERS implementation
│   ├── redissetx.c         # REDIS_SETEX implementation
│   ├── redisincb.c         # REDIS_INCRBY implementation
│   ├── redisdecb.c         # REDIS_DECRBY implementation
│   ├── redispst.c          # REDIS_PERSIST implementation
│   ├── redistype.c         # REDIS_TYPE implementation
│   ├── redisslen.c         # REDIS_STRLEN implementation
│   ├── rediskeys.c         # REDIS_KEYS implementation
│   ├── redisscan.c         # REDIS_SCAN implementation
│   ├── rediszadd.c         # REDIS_ZADD implementation
│   ├── rediszrem.c         # REDIS_ZREM implementation
│   ├── rediszsco.c         # REDIS_ZSCORE implementation
│   ├── rediszrnk.c         # REDIS_ZRANK implementation
│   ├── rediszcrd.c         # REDIS_ZCARD implementation
│   ├── rediszrng.c         # REDIS_ZRANGE implementation
│   ├── rediszrbs.c         # REDIS_ZRANGEBYSCORE implementation
│   ├── redismget.c         # REDIS_MGET implementation
│   ├── redismset.c         # REDIS_MSET implementation
│   ├── redisgset.c         # REDIS_GETSET implementation
│   ├── redisrnme.c         # REDIS_RENAME implementation
│   ├── redishscn.c         # REDIS_HSCAN implementation
│   ├── redissscn.c         # REDIS_SSCAN implementation
│   ├── redisdbsz.c         # REDIS_DBSIZE implementation
│   ├── redisbench.c        # Benchmark: table vs iconv conversion
│   └── redisutils.c        # Shared utilities (connection, conversion)
├── include/                 # Header files
│   ├── redis_utils.h       # Main header with function declarations
│   └── redis_config.h      # Generated from .env (DO NOT EDIT)
├── qsrvsrc/                 # Binding source
│   └── redisile.bnd        # Service program binding
├── .env                     # Redis + build configuration (IP, port, USE_ICONV)
├── generate_config.sh       # Generates redis_config.h from .env, copies headers to srcfile/
├── test_all.sh              # Automated test suite (49 tests via ODBC)
├── makefile                 # GNU Make build script
└── CLAUDE.md               # This file
```

## Key Technical Details

### EBCDIC/ASCII Conversion
- IBM i uses EBCDIC encoding, Redis uses ASCII
- Two conversion modes controlled by `USE_ICONV` in `.env`:

| `.env` Setting | Mode | Description |
|----------------|------|-------------|
| `USE_ICONV=0` | Static translation tables (default) | Hardcoded 256-byte tables for CCSID 37. Fastest for small payloads. |
| `USE_ICONV=1` | iconv via QtqIconvOpen | Auto-detects job CCSID. Required for non-English EBCDIC. |

- `ConvertToASCII()` - EBCDIC → ASCII for sending to Redis
- `ConvertToEBCDIC()` - ASCII → EBCDIC for receiving from Redis
- To switch modes: edit `.env`, then `gmake clean; gmake all`
- The makefile reads `.env` and sets `DEFINE(USE_ICONV)` and `BNDSRVPGM(QSYS/QTQICONV)` automatically

### Redis Protocol (RESP)
- Commands formatted in RESP protocol: `*2\r\n$3\r\nGET\r\n$<len>\r\n<key>\r\n`
- TCP socket connection to Redis server
- Timeout handling for network operations

### Configuration
- Redis connection and build options configured in `.env` file:
  - `REDIS_IP` - Redis server IP (default: `127.0.0.1`)
  - `REDIS_PORT` - Redis server port (default: `6379`)
  - `USE_ICONV` - EBCDIC/ASCII conversion mode (`0` = tables, `1` = iconv)
- `generate_config.sh` creates `include/redis_config.h` at build time (runtime config only, skips `USE_ICONV`)
- Headers are copied to `srcfile/` by `generate_config.sh` because ILE C `INCDIR` doesn't reliably resolve quoted includes

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
| `38999` | iconv initialization failed |

## Adding a New Redis Function

### CRITICAL: IBM i 10-Character Name Limit

IBM i system object names (modules, libraries, programs) are limited to **10 characters**. The source filename (minus `.c`) becomes the IBM i module name. Always verify the name fits before creating a file.

- ✓ `redisapnd.c` → Module `REDISAPND` (9 chars)
- ✗ `redisappend.c` → Module `REDISAPPEND` (11 chars) — **BUILD WILL FAIL**

Convention: `redis` (5 chars) + abbreviated command (≤ 5 chars)

### Steps

1. **Choose a short filename** (≤ 10 chars without `.c`). Create the C source file in `srcfile/` (e.g., `redisapnd.c` for APPEND)
2. Add module compilation target in `makefile`:
   ```makefile
   redisapnd.cle: redisapnd.cmodule redisapnd.bnd
   ```
3. Add module to service program dependencies:
   ```makefile
   redisile.srvpgm: ... redisapnd.cle
   ```
4. Add SQL function creation target:
   ```makefile
   redis_append.func:
       -system "RUNSQL SQL('CREATE OR REPLACE FUNCTION $(TGT_LIB).REDIS_APPEND ...') COMMIT(*NONE)"
   ```
5. Add target to `all:` rule
6. Export function in `qsrvsrc/redisile.bnd`
7. **Update documentation** (always do this as part of every new function):
   - Update `README.md`: add function to description list, project structure, makefile targets, usage examples
   - Update `CLAUDE.md`: add function to Available SQL Functions table and architecture diagram
   - Update `CHANGELOG.md`: add entry under current version

### Build Note

When running `gmake` via SSH, always set PATH first:
```bash
"/QOpenSys/pkgs/bin/bash -c 'export PATH=/QOpenSys/pkgs/bin:\$PATH && cd /home/ernestr/redis400 && gmake clean; gmake all'"
```

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
Use `REDIS_AUTH(password)` to authenticate before other commands:
```sql
VALUES REDIS_AUTH('your_password')
```

## Technology Stack

- **Language**: ILE C
- **Build System**: GNU Make
- **Target**: IBM i V7.4+
- **Redis Protocol**: RESP (REdis Serialization Protocol)
- **Encoding**: EBCDIC ↔ ASCII (static tables or iconv, configured via `.env`)

## Multi-Agent Architecture

This project uses specialized agents for different aspects of development.

### Available Skills

| Skill | Location | Expertise |
|-------|----------|-----------|
| Orchestrator | `.claude/skills/orchestrator/` | Coordinates all agents, delegates tasks |
| IBM i C Developer | `.claude/skills/ibmi-c-developer/` | C programming, ILE/PASE, EBCDIC, MI, shells |
| Tester | `.claude/skills/tester/` | Deploy to P7, build, run test suite |
| Redis Expert | `.claude/skills/redis-expert/` | RESP protocol, commands, authentication |

### Slash Commands

| Command | Description |
|---------|-------------|
| `/deploy` | Sync and build on P7 |
| `/test` | Run full test suite |

### Agent Workflow

1. **New Feature**: Redis Expert (design) → IBM i C Developer (implement) → Tester (verify)
2. **Bug Fix**: Tester (reproduce) → IBM i C Developer (fix) → Tester (verify)
3. **Protocol Issue**: Redis Expert (analyze) → IBM i C Developer (fix conversion)
