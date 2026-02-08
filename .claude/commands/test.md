# /test

Run the Redis400 SQL function test suite on P7.

## Prerequisites

1. Project must be deployed (`/deploy`)
2. Redis server must be running

## Steps

### 1. Ensure Redis is Running

```bash
sshpass -p 'ops_api' ssh P7-deploy "/QOpenSys/pkgs/bin/redis-cli PING" || \
sshpass -p 'ops_api' ssh P7-deploy "/QOpenSys/pkgs/bin/redis-server --daemonize yes && sleep 2 && /QOpenSys/pkgs/bin/redis-cli PING"
```

### 2. Run Test Suite

Test each function via ODBC:

```bash
# PING
echo "VALUES(REDIS400.REDIS_PING())" | isql -v ISSI_P10 OPS_API 'ops_api'

# SET
echo "VALUES REDIS400.REDIS_SET('test_key', 'test_value')" | isql -v ISSI_P10 OPS_API 'ops_api'

# GET
echo "SELECT REDIS400.REDIS_GET('test_key') FROM SYSIBM.SYSDUMMY1" | isql -v ISSI_P10 OPS_API 'ops_api'

# INCR
echo "VALUES REDIS400.REDIS_SET('counter', '0')" | isql -v ISSI_P10 OPS_API 'ops_api'
echo "SELECT REDIS400.REDIS_INCR('counter') FROM SYSIBM.SYSDUMMY1" | isql -v ISSI_P10 OPS_API 'ops_api'

# DEL
echo "SELECT REDIS400.REDIS_DEL('test_key') FROM SYSIBM.SYSDUMMY1" | isql -v ISSI_P10 OPS_API 'ops_api'

# EXPIRE
echo "VALUES REDIS400.REDIS_SET('expire_test', 'temp')" | isql -v ISSI_P10 OPS_API 'ops_api'
echo "SELECT REDIS400.REDIS_EXPIRE('expire_test', 60) FROM SYSIBM.SYSDUMMY1" | isql -v ISSI_P10 OPS_API 'ops_api'

# TTL
echo "SELECT REDIS400.REDIS_TTL('expire_test') FROM SYSIBM.SYSDUMMY1" | isql -v ISSI_P10 OPS_API 'ops_api'

# APPEND (set, append, verify)
echo "VALUES REDIS400.REDIS_SET('append_test', 'Hello')" | isql -v ISSI_P10 OPS_API 'ops_api'
echo "SELECT REDIS400.REDIS_APPEND('append_test', ' World') FROM SYSIBM.SYSDUMMY1" | isql -v ISSI_P10 OPS_API 'ops_api'
echo "SELECT REDIS400.REDIS_GET('append_test') FROM SYSIBM.SYSDUMMY1" | isql -v ISSI_P10 OPS_API 'ops_api'
```

### 3. Cleanup

```bash
echo "SELECT REDIS400.REDIS_DEL('counter') FROM SYSIBM.SYSDUMMY1" | isql -v ISSI_P10 OPS_API 'ops_api'
echo "SELECT REDIS400.REDIS_DEL('expire_test') FROM SYSIBM.SYSDUMMY1" | isql -v ISSI_P10 OPS_API 'ops_api'
echo "SELECT REDIS400.REDIS_DEL('append_test') FROM SYSIBM.SYSDUMMY1" | isql -v ISSI_P10 OPS_API 'ops_api'
```

## Report Format

| Function | Expected | Actual | Status |
|----------|----------|--------|--------|
| REDIS_PING | PONG | | |
| REDIS_SET | OK | | |
| REDIS_GET | test_value | | |
| REDIS_INCR | 1 | | |
| REDIS_DEL | 1 | | |
| REDIS_EXPIRE | 1 | | |
| REDIS_TTL | >0 | | |
| REDIS_APPEND | 11 | | |

## Troubleshooting

- **SQLSTATE 38901**: Redis not running - start with `redis-server --daemonize yes`
- **Connection timeout**: Check VPN connectivity to P7
- **Empty results**: Check EBCDIC/ASCII conversion in source code
