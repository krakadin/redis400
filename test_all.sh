#!/bin/bash
# Full test suite for all 50 Redis SQL functions
# Usage: bash test_all.sh

ISQL_CMD="isql -v ISSI_P10 OPS_API ops_api"
PASS=0
FAIL=0

run_test() {
    local name="$1"
    local sql="$2"
    local expect="$3"

    result=$(echo "$sql" | $ISQL_CMD 2>&1)
    if echo "$result" | grep -q "$expect"; then
        echo "PASS: $name"
        PASS=$((PASS + 1))
    else
        echo "FAIL: $name (expected '$expect')"
        echo "  Got: $(echo "$result" | tail -5)"
        FAIL=$((FAIL + 1))
    fi
}

echo "========================================="
echo "Redis SQL Functions - Full Test Suite"
echo "========================================="
echo ""

# Cleanup first
echo "VALUES REDIS400.REDIS_DEL('t_key')" | $ISQL_CMD > /dev/null 2>&1
echo "VALUES REDIS400.REDIS_DEL('t_ctr')" | $ISQL_CMD > /dev/null 2>&1
echo "VALUES REDIS400.REDIS_DEL('t_app')" | $ISQL_CMD > /dev/null 2>&1
echo "VALUES REDIS400.REDIS_DEL('t_hash')" | $ISQL_CMD > /dev/null 2>&1
echo "VALUES REDIS400.REDIS_DEL('t_list')" | $ISQL_CMD > /dev/null 2>&1
echo "VALUES REDIS400.REDIS_DEL('t_set')" | $ISQL_CMD > /dev/null 2>&1
echo "VALUES REDIS400.REDIS_DEL('t_ext')" | $ISQL_CMD > /dev/null 2>&1
echo "VALUES REDIS400.REDIS_DEL('t_scan1')" | $ISQL_CMD > /dev/null 2>&1
echo "VALUES REDIS400.REDIS_DEL('t_scan2')" | $ISQL_CMD > /dev/null 2>&1
echo "VALUES REDIS400.REDIS_DEL('t_zset')" | $ISQL_CMD > /dev/null 2>&1
echo "VALUES REDIS400.REDIS_DEL('t_mget1')" | $ISQL_CMD > /dev/null 2>&1
echo "VALUES REDIS400.REDIS_DEL('t_mget2')" | $ISQL_CMD > /dev/null 2>&1
echo "VALUES REDIS400.REDIS_DEL('t_gset')" | $ISQL_CMD > /dev/null 2>&1
echo "VALUES REDIS400.REDIS_DEL('t_rnold')" | $ISQL_CMD > /dev/null 2>&1
echo "VALUES REDIS400.REDIS_DEL('t_rnnew')" | $ISQL_CMD > /dev/null 2>&1
echo "VALUES REDIS400.REDIS_DEL('t_hscn')" | $ISQL_CMD > /dev/null 2>&1
echo "VALUES REDIS400.REDIS_DEL('t_sscn')" | $ISQL_CMD > /dev/null 2>&1

# 1. PING
run_test "REDIS_PING" \
    "VALUES(REDIS400.REDIS_PING())" \
    "PONG"

# 2. SET
run_test "REDIS_SET" \
    "VALUES REDIS400.REDIS_SET('t_key', 'hello')" \
    "OK"

# 3. GET
run_test "REDIS_GET" \
    "SELECT REDIS400.REDIS_GET('t_key') FROM SYSIBM.SYSDUMMY1" \
    "hello"

# 4. SETNX (key exists → 0)
run_test "REDIS_SETNX (existing key → 0)" \
    "VALUES REDIS400.REDIS_SETNX('t_key', 'new')" \
    "0"

# 5. EXISTS
run_test "REDIS_EXISTS" \
    "SELECT REDIS400.REDIS_EXISTS('t_key') FROM SYSIBM.SYSDUMMY1" \
    "1"

# 6. APPEND
run_test "REDIS_APPEND" \
    "SELECT REDIS400.REDIS_APPEND('t_key', '_world') FROM SYSIBM.SYSDUMMY1" \
    "11"

# 7. EXPIRE
run_test "REDIS_EXPIRE" \
    "SELECT REDIS400.REDIS_EXPIRE('t_key', 300) FROM SYSIBM.SYSDUMMY1" \
    "1"

# 8. TTL
run_test "REDIS_TTL (expect ~300)" \
    "SELECT REDIS400.REDIS_TTL('t_key') FROM SYSIBM.SYSDUMMY1" \
    "300"

# 9. INCR
echo "VALUES REDIS400.REDIS_DEL('t_ctr')" | $ISQL_CMD > /dev/null 2>&1
run_test "REDIS_INCR" \
    "VALUES REDIS400.REDIS_INCR('t_ctr')" \
    "1"

# 10. DECR
run_test "REDIS_DECR" \
    "VALUES REDIS400.REDIS_DECR('t_ctr')" \
    "0"

# 11. DEL
run_test "REDIS_DEL" \
    "VALUES REDIS400.REDIS_DEL('t_key')" \
    "1"

# 12. AUTH (no password set → expect error or ERR)
# Skip auth test if Redis has no password

# 13. HSET
run_test "REDIS_HSET" \
    "VALUES REDIS400.REDIS_HSET('t_hash', 'name', 'john')" \
    "1"

# 14. HGET
run_test "REDIS_HGET" \
    "SELECT REDIS400.REDIS_HGET('t_hash', 'name') FROM SYSIBM.SYSDUMMY1" \
    "john"

# 15. HEXISTS
run_test "REDIS_HEXISTS" \
    "SELECT REDIS400.REDIS_HEXISTS('t_hash', 'name') FROM SYSIBM.SYSDUMMY1" \
    "1"

# 16. HGETALL
echo "VALUES REDIS400.REDIS_HSET('t_hash', 'age', '30')" | $ISQL_CMD > /dev/null 2>&1
run_test "REDIS_HGETALL" \
    "SELECT REDIS400.REDIS_HGETALL('t_hash') FROM SYSIBM.SYSDUMMY1" \
    "name=john"

# 17. HDEL
run_test "REDIS_HDEL" \
    "VALUES REDIS400.REDIS_HDEL('t_hash', 'name')" \
    "1"

# 18. LPUSH
run_test "REDIS_LPUSH" \
    "VALUES REDIS400.REDIS_LPUSH('t_list', 'second')" \
    "1"

# 19. RPUSH
echo "VALUES REDIS400.REDIS_LPUSH('t_list', 'first')" | $ISQL_CMD > /dev/null 2>&1
run_test "REDIS_RPUSH" \
    "VALUES REDIS400.REDIS_RPUSH('t_list', 'third')" \
    "3"

# 20. LLEN
run_test "REDIS_LLEN" \
    "SELECT REDIS400.REDIS_LLEN('t_list') FROM SYSIBM.SYSDUMMY1" \
    "3"

# 21. LRANGE
run_test "REDIS_LRANGE" \
    "SELECT REDIS400.REDIS_LRANGE('t_list', 0, -1) FROM SYSIBM.SYSDUMMY1" \
    "first,second,third"

# 22. LPOP
run_test "REDIS_LPOP" \
    "SELECT REDIS400.REDIS_LPOP('t_list') FROM SYSIBM.SYSDUMMY1" \
    "first"

# 23. RPOP
run_test "REDIS_RPOP" \
    "SELECT REDIS400.REDIS_RPOP('t_list') FROM SYSIBM.SYSDUMMY1" \
    "third"

# 24. SADD
run_test "REDIS_SADD" \
    "VALUES REDIS400.REDIS_SADD('t_set', 'apple')" \
    "1"

# 25. SADD duplicate (expect 0)
echo "VALUES REDIS400.REDIS_SADD('t_set', 'banana')" | $ISQL_CMD > /dev/null 2>&1
run_test "REDIS_SISMEMBER" \
    "SELECT REDIS400.REDIS_SISMEMBER('t_set', 'apple') FROM SYSIBM.SYSDUMMY1" \
    "1"

# 26. SCARD
run_test "REDIS_SCARD" \
    "SELECT REDIS400.REDIS_SCARD('t_set') FROM SYSIBM.SYSDUMMY1" \
    "2"

# 27. SMEMBERS
run_test "REDIS_SMEMBERS" \
    "SELECT REDIS400.REDIS_SMEMBERS('t_set') FROM SYSIBM.SYSDUMMY1" \
    "apple"

# 28. SREM
run_test "REDIS_SREM" \
    "VALUES REDIS400.REDIS_SREM('t_set', 'apple')" \
    "1"

# --- Phase 5: Extended Key & String Operations ---

# 29. SETEX
run_test "REDIS_SETEX" \
    "VALUES REDIS400.REDIS_SETEX('t_ext', 300, 'temp')" \
    "OK"

# 30. TYPE
run_test "REDIS_TYPE" \
    "SELECT REDIS400.REDIS_TYPE('t_ext') FROM SYSIBM.SYSDUMMY1" \
    "string"

# 31. STRLEN
run_test "REDIS_STRLEN" \
    "SELECT REDIS400.REDIS_STRLEN('t_ext') FROM SYSIBM.SYSDUMMY1" \
    "4"

# 32. PERSIST
run_test "REDIS_PERSIST" \
    "SELECT REDIS400.REDIS_PERSIST('t_ext') FROM SYSIBM.SYSDUMMY1" \
    "1"

# 33. INCRBY
echo "VALUES REDIS400.REDIS_SET('t_ctr', '0')" | $ISQL_CMD > /dev/null 2>&1
run_test "REDIS_INCRBY" \
    "VALUES REDIS400.REDIS_INCRBY('t_ctr', 10)" \
    "10"

# 34. DECRBY
run_test "REDIS_DECRBY" \
    "VALUES REDIS400.REDIS_DECRBY('t_ctr', 3)" \
    "7"

# --- Phase 6: Key Scanning & Pattern Matching ---

# 35. KEYS
echo "VALUES REDIS400.REDIS_SET('t_scan1', 'val1')" | $ISQL_CMD > /dev/null 2>&1
echo "VALUES REDIS400.REDIS_SET('t_scan2', 'val2')" | $ISQL_CMD > /dev/null 2>&1
run_test "REDIS_KEYS" \
    "SELECT REDIS400.REDIS_KEYS('t_scan*') FROM SYSIBM.SYSDUMMY1" \
    "t_scan"

# 36. SCAN
run_test "REDIS_SCAN" \
    "SELECT REDIS400.REDIS_SCAN('0', 't_scan*', 10) FROM SYSIBM.SYSDUMMY1" \
    "t_scan"

# --- Phase 7: Sorted Set Operations ---

# 37. ZADD
run_test "REDIS_ZADD" \
    "VALUES REDIS400.REDIS_ZADD('t_zset', 1.0, 'alice')" \
    "1"

# 38. ZADD (second member)
echo "VALUES REDIS400.REDIS_ZADD('t_zset', 2.0, 'bob')" | $ISQL_CMD > /dev/null 2>&1
echo "VALUES REDIS400.REDIS_ZADD('t_zset', 3.0, 'charlie')" | $ISQL_CMD > /dev/null 2>&1

# 38. ZSCORE
run_test "REDIS_ZSCORE" \
    "SELECT REDIS400.REDIS_ZSCORE('t_zset', 'alice') FROM SYSIBM.SYSDUMMY1" \
    "1"

# 39. ZRANK
run_test "REDIS_ZRANK" \
    "SELECT REDIS400.REDIS_ZRANK('t_zset', 'alice') FROM SYSIBM.SYSDUMMY1" \
    "0"

# 40. ZCARD
run_test "REDIS_ZCARD" \
    "SELECT REDIS400.REDIS_ZCARD('t_zset') FROM SYSIBM.SYSDUMMY1" \
    "3"

# 41. ZRANGE
run_test "REDIS_ZRANGE" \
    "SELECT REDIS400.REDIS_ZRANGE('t_zset', 0, -1) FROM SYSIBM.SYSDUMMY1" \
    "alice,bob,charlie"

# 42. ZRANGEBYSCORE
run_test "REDIS_ZRANGEBYSCORE" \
    "SELECT REDIS400.REDIS_ZRANGEBYSCORE('t_zset', '1', '2') FROM SYSIBM.SYSDUMMY1" \
    "alice,bob"

# 43. ZREM
run_test "REDIS_ZREM" \
    "VALUES REDIS400.REDIS_ZREM('t_zset', 'charlie')" \
    "1"

# --- Phase 8: Multi-key Operations ---

# 44. MSET
run_test "REDIS_MSET" \
    "VALUES REDIS400.REDIS_MSET('t_mget1=val1,t_mget2=val2')" \
    "OK"

# 45. MGET
run_test "REDIS_MGET" \
    "SELECT REDIS400.REDIS_MGET('t_mget1,t_mget2') FROM SYSIBM.SYSDUMMY1" \
    "val1,val2"

# 46. GETSET
echo "VALUES REDIS400.REDIS_SET('t_gset', 'old')" | $ISQL_CMD > /dev/null 2>&1
run_test "REDIS_GETSET" \
    "SELECT REDIS400.REDIS_GETSET('t_gset', 'new') FROM SYSIBM.SYSDUMMY1" \
    "old"

# 47. RENAME
echo "VALUES REDIS400.REDIS_SET('t_rnold', 'rename_me')" | $ISQL_CMD > /dev/null 2>&1
run_test "REDIS_RENAME" \
    "VALUES REDIS400.REDIS_RENAME('t_rnold', 't_rnnew')" \
    "OK"

# --- Phase 9: Hash/Set Scanning ---

# 48. HSCAN
echo "VALUES REDIS400.REDIS_HSET('t_hscn', 'f1', 'v1')" | $ISQL_CMD > /dev/null 2>&1
echo "VALUES REDIS400.REDIS_HSET('t_hscn', 'f2', 'v2')" | $ISQL_CMD > /dev/null 2>&1
run_test "REDIS_HSCAN" \
    "SELECT REDIS400.REDIS_HSCAN('t_hscn', '0', '*', 10) FROM SYSIBM.SYSDUMMY1" \
    "f1=v1"

# 49. SSCAN
echo "VALUES REDIS400.REDIS_SADD('t_sscn', 'mem1')" | $ISQL_CMD > /dev/null 2>&1
echo "VALUES REDIS400.REDIS_SADD('t_sscn', 'mem2')" | $ISQL_CMD > /dev/null 2>&1
run_test "REDIS_SSCAN" \
    "SELECT REDIS400.REDIS_SSCAN('t_sscn', '0', '*', 10) FROM SYSIBM.SYSDUMMY1" \
    "mem"

# --- Phase 10: Server Operations ---

# 50. DBSIZE (returns number of keys in DB; at this point we have test keys so > 0)
run_test "REDIS_DBSIZE" \
    "VALUES(REDIS400.REDIS_DBSIZE())" \
    "1"

# Cleanup
echo "VALUES REDIS400.REDIS_DEL('t_scan1')" | $ISQL_CMD > /dev/null 2>&1
echo "VALUES REDIS400.REDIS_DEL('t_scan2')" | $ISQL_CMD > /dev/null 2>&1
echo "VALUES REDIS400.REDIS_DEL('t_ctr')" | $ISQL_CMD > /dev/null 2>&1
echo "VALUES REDIS400.REDIS_DEL('t_app')" | $ISQL_CMD > /dev/null 2>&1
echo "VALUES REDIS400.REDIS_DEL('t_hash')" | $ISQL_CMD > /dev/null 2>&1
echo "VALUES REDIS400.REDIS_DEL('t_list')" | $ISQL_CMD > /dev/null 2>&1
echo "VALUES REDIS400.REDIS_DEL('t_set')" | $ISQL_CMD > /dev/null 2>&1
echo "VALUES REDIS400.REDIS_DEL('t_ext')" | $ISQL_CMD > /dev/null 2>&1
echo "VALUES REDIS400.REDIS_DEL('t_zset')" | $ISQL_CMD > /dev/null 2>&1
echo "VALUES REDIS400.REDIS_DEL('t_mget1')" | $ISQL_CMD > /dev/null 2>&1
echo "VALUES REDIS400.REDIS_DEL('t_mget2')" | $ISQL_CMD > /dev/null 2>&1
echo "VALUES REDIS400.REDIS_DEL('t_gset')" | $ISQL_CMD > /dev/null 2>&1
echo "VALUES REDIS400.REDIS_DEL('t_rnold')" | $ISQL_CMD > /dev/null 2>&1
echo "VALUES REDIS400.REDIS_DEL('t_rnnew')" | $ISQL_CMD > /dev/null 2>&1
echo "VALUES REDIS400.REDIS_DEL('t_hscn')" | $ISQL_CMD > /dev/null 2>&1
echo "VALUES REDIS400.REDIS_DEL('t_sscn')" | $ISQL_CMD > /dev/null 2>&1

echo ""
echo "========================================="
echo "Results: $PASS passed, $FAIL failed (of $((PASS + FAIL)) tests)"
echo "========================================="
