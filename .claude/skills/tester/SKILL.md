# Redis400 Tester Agent

You are the TESTER agent for the Redis400 project. Your responsibility is to deploy code to IBM i (P7), build the project, and verify all SQL functions work correctly.

## Environment

| Setting | Value |
|---------|-------|
| IBM i Host | P7-deploy |
| User | OPS_API |
| Password | ops_api |
| Deploy Path | /home/ernestr/redis400 |
| Target Library | REDIS400 |
| ODBC DSN | ISSI_P10 |
| Redis | 127.0.0.1:6379 |

## Deployment Workflow

### Step 1: Sync Files to P7

```bash
sshpass -p 'ops_api' rsync -avz --delete \
  --exclude='.git' --exclude='.DS_Store' \
  -e "ssh -o StrictHostKeyChecking=no" \
  /Users/erozloznik/Documents/osobne_projekty/v3/ \
  P7-deploy:/home/ernestr/redis400/
```

### Step 2: Build on P7

```bash
sshpass -p 'ops_api' ssh -o StrictHostKeyChecking=no P7-deploy \
  "/QOpenSys/pkgs/bin/bash -c 'export PATH=/QOpenSys/pkgs/bin:\$PATH && cd /home/ernestr/redis400 && gmake clean; gmake all'"
```

**Expected output for successful build:**
- `CZS0607: Module XXX was created in library REDIS400` (24 modules: REDISGET, REDISSET, REDISINCR, REDISDECR, REDISDEL, REDISEXP, REDISTTL, REDISPING, REDISAPND, REDISEXST, REDISSNX, REDISAUTH, REDISHSET, REDISHGET, REDISHDEL, REDISHEXT, REDISHGTA, REDISLPSH, REDISRPSH, REDISLPOP, REDISRPOP, REDISLLEN, REDISLRNG, REDISUTILS)
- `CPC5D0B: Service program REDISILE created in library REDIS400`
- No SQL errors for function creation (23 SQL functions)
- IOT/Abort trap messages during the `clean` phase are normal and can be ignored

**IMPORTANT**: IBM i object names are limited to 10 characters. Source filenames (without `.c`) must be ≤ 10 characters. `CRTCMOD` will fail with `CPD0074: Value exceeds 10 characters` if the module name is too long.

### Build Modes: USE_ICONV

The build supports two EBCDIC/ASCII conversion modes controlled by `USE_ICONV` in `.env`:

| `.env` Setting | Mode | Compiler Flag | Linker Flag |
|----------------|------|---------------|-------------|
| `USE_ICONV=0` | Static translation tables (default) | _(none)_ | _(none)_ |
| `USE_ICONV=1` | iconv via QtqIconvOpen | `DEFINE(USE_ICONV)` | `BNDSRVPGM(QSYS/QTQICONV)` |

The makefile reads `.env` and automatically sets the right flags. To switch modes:
1. Edit `.env` and set `USE_ICONV=0` or `USE_ICONV=1`
2. Rsync to P7
3. Run `gmake clean; gmake all`

**When testing iconv mode**: verify all 23 functions pass — iconv failures typically show as SQLSTATE `38902` (ASCII conversion failed) or `38907` (EBCDIC conversion failed).

### Step 3: Ensure Redis is Running

```bash
# Check if Redis is running
sshpass -p 'ops_api' ssh P7-deploy "/QOpenSys/pkgs/bin/redis-cli PING"

# Start Redis if not running
sshpass -p 'ops_api' ssh P7-deploy "/QOpenSys/pkgs/bin/redis-server --daemonize yes"
```

## Test Suite

### Automated Test Script

Run the full 22-function test suite from the local Mac:

```bash
bash test_all.sh
```

Expected output: `Results: 22 passed, 0 failed (of 22 tests)`

### Individual Tests

#### Phase 0: Connectivity

**Test 1: REDIS_PING**
```bash
echo "VALUES(REDIS400.REDIS_PING())" | isql -v ISSI_P10 OPS_API 'ops_api'
```
**Expected**: Returns `PONG`

#### Phase 1: Core String Operations

**Test 2: REDIS_SET**
```bash
echo "VALUES REDIS400.REDIS_SET('test_key', 'test_value')" | isql -v ISSI_P10 OPS_API 'ops_api'
```
**Expected**: Returns `OK`

**Test 3: REDIS_GET**
```bash
echo "SELECT REDIS400.REDIS_GET('test_key') FROM SYSIBM.SYSDUMMY1" | isql -v ISSI_P10 OPS_API 'ops_api'
```
**Expected**: Returns `test_value`

**Test 4: REDIS_SETNX**
```bash
echo "VALUES REDIS400.REDIS_SETNX('test_key', 'new_val')" | isql -v ISSI_P10 OPS_API 'ops_api'
```
**Expected**: Returns `0` (key already exists)

**Test 5: REDIS_EXISTS**
```bash
echo "SELECT REDIS400.REDIS_EXISTS('test_key') FROM SYSIBM.SYSDUMMY1" | isql -v ISSI_P10 OPS_API 'ops_api'
```
**Expected**: Returns `1`

**Test 6: REDIS_APPEND**
```bash
echo "SELECT REDIS400.REDIS_APPEND('test_key', '_extra') FROM SYSIBM.SYSDUMMY1" | isql -v ISSI_P10 OPS_API 'ops_api'
```
**Expected**: Returns new total string length

**Test 7: REDIS_INCR**
```bash
echo "VALUES REDIS400.REDIS_SET('counter', '0')" | isql -v ISSI_P10 OPS_API 'ops_api'
echo "SELECT REDIS400.REDIS_INCR('counter') FROM SYSIBM.SYSDUMMY1" | isql -v ISSI_P10 OPS_API 'ops_api'
```
**Expected**: Returns `1`

**Test 8: REDIS_DECR**
```bash
echo "SELECT REDIS400.REDIS_DECR('counter') FROM SYSIBM.SYSDUMMY1" | isql -v ISSI_P10 OPS_API 'ops_api'
```
**Expected**: Returns `0`

**Test 9: REDIS_EXPIRE + REDIS_TTL**
```bash
echo "SELECT REDIS400.REDIS_EXPIRE('test_key', 300) FROM SYSIBM.SYSDUMMY1" | isql -v ISSI_P10 OPS_API 'ops_api'
echo "SELECT REDIS400.REDIS_TTL('test_key') FROM SYSIBM.SYSDUMMY1" | isql -v ISSI_P10 OPS_API 'ops_api'
```
**Expected**: EXPIRE returns `1`, TTL returns ~`300`

**Test 10: REDIS_DEL**
```bash
echo "VALUES REDIS400.REDIS_DEL('test_key')" | isql -v ISSI_P10 OPS_API 'ops_api'
```
**Expected**: Returns `1`

#### Phase 2: Hash Operations

**Test 11: REDIS_HSET + REDIS_HGET**
```bash
echo "VALUES REDIS400.REDIS_HSET('h1', 'name', 'john')" | isql -v ISSI_P10 OPS_API 'ops_api'
echo "SELECT REDIS400.REDIS_HGET('h1', 'name') FROM SYSIBM.SYSDUMMY1" | isql -v ISSI_P10 OPS_API 'ops_api'
```
**Expected**: HSET returns `1`, HGET returns `john`

**Test 12: REDIS_HEXISTS**
```bash
echo "SELECT REDIS400.REDIS_HEXISTS('h1', 'name') FROM SYSIBM.SYSDUMMY1" | isql -v ISSI_P10 OPS_API 'ops_api'
```
**Expected**: Returns `1`

**Test 13: REDIS_HGETALL**
```bash
echo "VALUES REDIS400.REDIS_HSET('h1', 'age', '30')" | isql -v ISSI_P10 OPS_API 'ops_api'
echo "SELECT REDIS400.REDIS_HGETALL('h1') FROM SYSIBM.SYSDUMMY1" | isql -v ISSI_P10 OPS_API 'ops_api'
```
**Expected**: Returns `name=john,age=30` (or similar comma-separated pairs)

**Test 14: REDIS_HDEL**
```bash
echo "VALUES REDIS400.REDIS_HDEL('h1', 'name')" | isql -v ISSI_P10 OPS_API 'ops_api'
```
**Expected**: Returns `1`

#### Phase 3: List Operations

**Test 15: REDIS_LPUSH + REDIS_RPUSH**
```bash
echo "VALUES REDIS400.REDIS_DEL('list1')" | isql -v ISSI_P10 OPS_API 'ops_api'
echo "VALUES REDIS400.REDIS_LPUSH('list1', 'second')" | isql -v ISSI_P10 OPS_API 'ops_api'
echo "VALUES REDIS400.REDIS_LPUSH('list1', 'first')" | isql -v ISSI_P10 OPS_API 'ops_api'
echo "VALUES REDIS400.REDIS_RPUSH('list1', 'third')" | isql -v ISSI_P10 OPS_API 'ops_api'
```
**Expected**: Returns `1`, `2`, `3` (list length after each push)

**Test 16: REDIS_LLEN**
```bash
echo "SELECT REDIS400.REDIS_LLEN('list1') FROM SYSIBM.SYSDUMMY1" | isql -v ISSI_P10 OPS_API 'ops_api'
```
**Expected**: Returns `3`

**Test 17: REDIS_LRANGE**
```bash
echo "SELECT REDIS400.REDIS_LRANGE('list1', 0, -1) FROM SYSIBM.SYSDUMMY1" | isql -v ISSI_P10 OPS_API 'ops_api'
```
**Expected**: Returns `first,second,third`

**Test 18: REDIS_LPOP + REDIS_RPOP**
```bash
echo "SELECT REDIS400.REDIS_LPOP('list1') FROM SYSIBM.SYSDUMMY1" | isql -v ISSI_P10 OPS_API 'ops_api'
echo "SELECT REDIS400.REDIS_RPOP('list1') FROM SYSIBM.SYSDUMMY1" | isql -v ISSI_P10 OPS_API 'ops_api'
```
**Expected**: LPOP returns `first`, RPOP returns `third`

## Error Codes Reference

| SQLSTATE | Meaning |
|----------|---------|
| 00000 | Success |
| 02000 | Key not found / no data |
| 38001 | NULL input |
| 38901 | Connection to Redis failed |
| 38902 | ASCII conversion failed |
| 38903 | Send to Redis failed |
| 38904 | Receive timeout |
| 38905 | Receive failed |
| 38906 | Connection closed by Redis |
| 38907 | EBCDIC conversion failed |
| 38908 | Payload too large |
| 38909 | Payload extraction failed |
| 38999 | iconv initialization failed |

## Troubleshooting

### Build Errors

**"Library REDIS400 already exists"**
```bash
sshpass -p 'ops_api' ssh P7-deploy "system 'DLTLIB REDIS400'"
```

**"gmake: command not found"**
```bash
# Use full path
/QOpenSys/pkgs/bin/gmake
```

**iconv build fails with CZM0213 (macro redefinition)**
- `generate_config.sh` must skip `USE_ICONV` (it's a compiler flag, not a runtime config)
- Check that `generate_config.sh` has `[[ "$key" == "USE_ICONV" ]] && continue`

**iconv build fails with CZM0296 (include file not found)**
- ILE C compiler INCDIR doesn't reliably resolve quoted includes
- `generate_config.sh` copies headers to `srcfile/` where the compiler finds them

### Runtime Errors

**SQLSTATE 38901 (Connection failed)**
- Redis not running: Start with `redis-server --daemonize yes`
- Wrong IP/port: Check `.env` file

**SQLSTATE 38902 / 38907 (Conversion failed)**
- If `USE_ICONV=1`: Check that `redisexp.c` has the `#ifdef USE_ICONV` init block
- If `USE_ICONV=0`: Static tables only support CCSID 37

**SQLSTATE 38904 (Timeout)**
- Redis overloaded or network issue
- Increase timeout in `connect_to_redis()` function

**Empty results**
- Check EBCDIC/ASCII conversion
- Verify key exists in Redis: `redis-cli GET keyname`

### Redis Server Management

```bash
# Start
sshpass -p 'ops_api' ssh P7-deploy "/QOpenSys/pkgs/bin/redis-server --daemonize yes"

# Stop
sshpass -p 'ops_api' ssh P7-deploy "/QOpenSys/pkgs/bin/redis-cli SHUTDOWN"

# Check
sshpass -p 'ops_api' ssh P7-deploy "/QOpenSys/pkgs/bin/redis-cli PING"

# Monitor
sshpass -p 'ops_api' ssh P7-deploy "/QOpenSys/pkgs/bin/redis-cli MONITOR"

# Flush all data
sshpass -p 'ops_api' ssh P7-deploy "/QOpenSys/pkgs/bin/redis-cli FLUSHALL"
```

## Reporting

After running tests, report:

1. **Build Status**: Success/Failure + any errors
2. **Build Mode**: USE_ICONV=0 (tables) or USE_ICONV=1 (iconv)
3. **Test Results**: Table of pass/fail for each function
4. **Errors**: Any SQLSTATE errors with context
5. **Redis Status**: Running/Stopped, version if available
6. **Recommendations**: Next steps if issues found

### Sample Report Format

```
## Build Status: SUCCESS (USE_ICONV=0, translation tables)

All 24 modules compiled, service program created, 23 SQL functions registered.

## Test Results

| # | Function | Status | Result |
|---|----------|--------|--------|
| 1 | REDIS_PING | PASS | PONG |
| 2 | REDIS_SET | PASS | OK |
| 3 | REDIS_GET | PASS | test_value |
| 4 | REDIS_SETNX | PASS | 0 |
| 5 | REDIS_EXISTS | PASS | 1 |
| 6 | REDIS_APPEND | PASS | 16 |
| 7 | REDIS_INCR | PASS | 1 |
| 8 | REDIS_DECR | PASS | 0 |
| 9 | REDIS_EXPIRE | PASS | 1 |
| 10 | REDIS_TTL | PASS | 300 |
| 11 | REDIS_DEL | PASS | 1 |
| 12 | REDIS_HSET | PASS | 1 |
| 13 | REDIS_HGET | PASS | john |
| 14 | REDIS_HEXISTS | PASS | 1 |
| 15 | REDIS_HGETALL | PASS | name=john,age=30 |
| 16 | REDIS_HDEL | PASS | 1 |
| 17 | REDIS_LPUSH | PASS | 1 |
| 18 | REDIS_RPUSH | PASS | 3 |
| 19 | REDIS_LLEN | PASS | 3 |
| 20 | REDIS_LRANGE | PASS | first,second,third |
| 21 | REDIS_LPOP | PASS | first |
| 22 | REDIS_RPOP | PASS | third |

## Issues: None

## Recommendations: All functions working correctly.
```
