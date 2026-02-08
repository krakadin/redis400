# Changelog

All notable changes to the Redis SQL Functions for IBM i project.

## [Unreleased]

### Added
- **Server operations (Phase 10: Database Monitoring)**:
  - `REDIS_DBSIZE()` - Returns the number of keys in the currently selected Redis database (no parameters, returns BIGINT)
- **Hash/set scanning (Phase 9: Cursor-Based Collection Scanning)**:
  - `REDIS_HSCAN(key, cursor, pattern, count)` - Cursor-based hash field iteration (returns "cursor|field1=value1,field2=value2" format)
  - `REDIS_SSCAN(key, cursor, pattern, count)` - Cursor-based set member iteration (returns "cursor|member1,member2" format)
- Custom nested RESP array parsers in `redishscn.c` and `redissscn.c` for HSCAN/SSCAN two-level responses
- First 7-argument RESP commands (*7): HSCAN and SSCAN with key, cursor, MATCH, pattern, COUNT, count
- **Multi-key operations (Phase 8: Batch & Atomic Ops)**:
  - `REDIS_MGET(keys)` - Get multiple values at once (comma-separated keys in, comma-separated values out)
  - `REDIS_MSET(kvpairs)` - Set multiple key-value pairs atomically (format: "key1=val1,key2=val2")
  - `REDIS_GETSET(key, value)` - Atomically set new value and return old value
  - `REDIS_RENAME(oldkey, newkey)` - Rename a key (returns "OK")
- Variable-length RESP command builder for MGET/MSET (dynamic argument count based on comma-separated input parsing)
- EBCDIC comma (0x6B) and equals (0x7E) parsing for MGET/MSET input format
- **Sorted set operations (Phase 7: Ranked Collections)**:
  - `REDIS_ZADD(key, score, member)` - Add member with score to sorted set (returns 1 if added, 0 if updated)
  - `REDIS_ZREM(key, member)` - Remove member from sorted set (returns 1 if removed, 0 if not found)
  - `REDIS_ZSCORE(key, member)` - Get score of a member (returns score as string, NULL if not found)
  - `REDIS_ZRANK(key, member)` - Get 0-based rank of member in sorted set (NULL if not found)
  - `REDIS_ZCARD(key)` - Get number of members in sorted set (0 if key doesn't exist)
  - `REDIS_ZRANGE(key, start, stop)` - Get range of members by index as comma-separated string (supports negative indices)
  - `REDIS_ZRANGEBYSCORE(key, min, max)` - Get members within score range as comma-separated string (supports -inf, +inf, exclusive ranges)
- DOUBLE score conversion for ZADD: `snprintf` to ASCII then character-by-character EBCDIC mapping
- Custom RESP array parsers in `rediszrng.c` and `rediszrbs.c` for ZRANGE/ZRANGEBYSCORE multi-bulk responses
- **Key scanning & pattern matching (Phase 6: Key Discovery)**:
  - `REDIS_KEYS(pattern)` - Get all keys matching a pattern as comma-separated string (blocks Redis on large datasets)
  - `REDIS_SCAN(cursor, pattern, count)` - Cursor-based key iteration with MATCH/COUNT support (returns "cursor|key1,key2" format, production-safe)
- First 6-argument RESP command (*6) in the project: SCAN with cursor, MATCH, pattern, COUNT, count
- Custom nested RESP array parser in `redisscan.c` for SCAN's two-level response (cursor + keys array)
- Pipe separator (`|`) in SCAN output to delimit cursor from key list
- **Extended key & string operations (Phase 5: Key Management & Counters)**:
  - `REDIS_SETEX(key, ttl, value)` - Set key-value pair with expiration in one atomic operation (returns "OK")
  - `REDIS_INCRBY(key, increment)` - Increment integer value by specified amount (returns new value as BIGINT)
  - `REDIS_DECRBY(key, decrement)` - Decrement integer value by specified amount (returns new value as BIGINT)
  - `REDIS_PERSIST(key)` - Remove expiration from a key (returns 1 if timeout removed, 0 otherwise)
  - `REDIS_TYPE(key)` - Get the type of value stored at key (returns "string", "list", "set", "hash", "zset", or "none")
  - `REDIS_STRLEN(key)` - Get the length of string stored at key (returns 0 if key doesn't exist)
- First 4-argument RESP command (*4) in the project: SETEX with key, TTL, and value
- BIGINT increment/decrement support with EBCDIC negative sign (0x60) handling
- **Set operations (Phase 4: Unique Collections)**:
  - `REDIS_SADD(key, member)` - Add member to set (returns 1 if added, 0 if already member)
  - `REDIS_SREM(key, member)` - Remove member from set (returns 1 if removed, 0 if not member)
  - `REDIS_SISMEMBER(key, member)` - Check if member exists in set (returns 1 or 0)
  - `REDIS_SCARD(key)` - Get number of members in set (0 if key doesn't exist)
  - `REDIS_SMEMBERS(key)` - Get all set members as comma-separated string (order may vary)
- Custom RESP array parser in `redissmem.c` for SMEMBERS multi-bulk responses
- **List operations (Phase 3: Queues & Ordered Data)**:
  - `REDIS_LPUSH(key, value)` - Push value to head of list (returns new length)
  - `REDIS_RPUSH(key, value)` - Push value to tail of list (returns new length)
  - `REDIS_LPOP(key)` - Remove and return first element (NULL if empty)
  - `REDIS_RPOP(key)` - Remove and return last element (NULL if empty)
  - `REDIS_LLEN(key)` - Get list length (0 if key doesn't exist)
  - `REDIS_LRANGE(key, start, stop)` - Get range of elements as comma-separated string (supports negative indices)
- Custom RESP array parser in `redislrng.c` for LRANGE multi-bulk responses
- EBCDIC negative integer formatting helper (`int_to_ebcdic()`) for LRANGE indices
- **Hash operations (Phase 2: Structured Data)**:
  - `REDIS_HSET(key, field, value)` - Set a hash field (returns 1 if new, 0 if updated)
  - `REDIS_HGET(key, field)` - Get a hash field value
  - `REDIS_HDEL(key, field)` - Delete a hash field (returns 1 if deleted, 0 if not found)
  - `REDIS_HEXISTS(key, field)` - Check if a hash field exists (returns 1 or 0)
  - `REDIS_HGETALL(key)` - Get all hash fields as comma-separated field=value pairs
- RESP array parser in `redishgta.c` for multi-bulk responses
- **Core gap functions (Phase 1)**:
  - `REDIS_EXISTS(key)` - Check if a key exists (returns 1 or 0)
  - `REDIS_SETNX(key, value)` - Set key only if it doesn't already exist (atomic)
  - `REDIS_DECR(key)` - Decrement an integer value by 1
  - `REDIS_AUTH(password)` - Authenticate with a password-protected Redis server
- `redisbench.c` - Benchmark program comparing static table vs iconv conversion
- Fixed `USE_ICONV` support: added missing variable declarations, zeroed `QtqCode_T` struct fields
- `test_all.sh` automated test suite — runs all 48 function tests via local ODBC
- Documentation step added to new-function development workflow
- EBCDIC/ASCII conversion section in README with benchmark results and decision table

### Changed
- **Build system: `.env`-driven USE_ICONV toggle** — `USE_ICONV` is now set in `.env` (not in source code). The makefile reads `.env` via `include .env` and conditionally sets `DEFINE(USE_ICONV)` compiler flag and `BNDSRVPGM(QSYS/QTQICONV)` linker flag.
- **`generate_config.sh` improvements**:
  - Skips `USE_ICONV` when generating `redis_config.h` (it's a compiler flag, not a runtime config) to avoid `CZM0213` macro redefinition errors
  - Copies headers (`redis_utils.h`, `redis_config.h`) to `srcfile/` because ILE C `INCDIR` doesn't reliably resolve quoted includes
- **Removed `INCDIR` from makefile** `CRTCMOD` command — headers are now found in `srcfile/` alongside source files
- Updated `makefile` with 41 new module targets and SQL function targets (Phase 1-9)
- Updated `qsrvsrc/redisile.bnd` with 41 new exports (Phase 1-9)
- Updated `README.md` with all 49 functions, examples, iconv documentation, and `.env`-based USE_ICONV toggle
- Updated `CLAUDE.md` with new architecture, SQL functions table, USE_ICONV build modes, and `test_all.sh`
- Updated orchestrator and tester agent skills with all 49 functions and build mode documentation

## [1.0.0] - 2025-02-22

### Added
- `REDIS_GET(key)` - Get value by key
- `REDIS_SET(key, value)` - Set key-value pair
- `REDIS_INCR(key)` - Increment integer value
- `REDIS_DEL(key)` - Delete a key
- `REDIS_EXPIRE(key, ttl)` - Set key expiration in seconds
- `REDIS_TTL(key)` - Get remaining time-to-live
- `REDIS_PING()` - Test Redis connectivity
- `REDIS_APPEND(key, value)` - Append to string value
- EBCDIC/ASCII static translation tables in `redisutils.c`
- GNU Make build system for IBM i ILE C compilation
- `.env`-based Redis configuration with `generate_config.sh`
- RESP protocol implementation over TCP sockets
- SQLSTATE error codes for all failure modes
