# Redis SQL Functions for IBM i

## Description

This project provides Redis utility functions for IBM i, enabling seamless interaction with a Redis server through SQL User-Defined Functions (UDFs). It includes the following functions:

1. **`REDIS_GET`**: Retrieves a value from Redis using a specified key.
2. **`REDIS_SET`**: Sets a value in Redis using a key-value pair.
3. **`REDIS_INCR`**: Increments the integer value of a key by 1. If the key does not exist, it is set to 0 before performing the operation.
4. **`REDIS_DEL`**: Deletes a key from Redis. Returns 1 if the key was deleted, or 0 if the key did not exist.
5. **`REDIS_EXPIRE`**: Sets an expiration time (TTL) in seconds for a Redis key.
6. **`REDIS_TTL`**: Retrieves the remaining time-to-live (TTL) of a Redis key in seconds.
7. **`REDIS_PING`**: Sends a `PING` command to the Redis server and returns the response (`PONG`).
8. **`REDIS_APPEND`**: Appends a value to an existing string. Returns the new string length.
9. **`REDIS_EXISTS`**: Checks if a key exists. Returns 1 if yes, 0 if no.
10. **`REDIS_SETNX`**: Sets a key-value pair only if the key does not already exist (atomic). Returns 1 if set, 0 if key already exists.
11. **`REDIS_DECR`**: Decrements the integer value of a key by 1. Returns the new value (can be negative).
12. **`REDIS_AUTH`**: Authenticates with a password-protected Redis server. Returns "OK" on success.
13. **`REDIS_HSET`**: Sets a field in a Redis hash. Returns 1 if the field is new, 0 if it was updated.
14. **`REDIS_HGET`**: Gets the value of a field from a Redis hash.
15. **`REDIS_HDEL`**: Deletes a field from a Redis hash. Returns 1 if deleted, 0 if not found.
16. **`REDIS_HEXISTS`**: Checks if a field exists in a Redis hash. Returns 1 if yes, 0 if no.
17. **`REDIS_HGETALL`**: Gets all fields and values from a Redis hash as comma-separated field=value pairs.
18. **`REDIS_LPUSH`**: Pushes a value to the head (left) of a list. Returns the new list length.
19. **`REDIS_RPUSH`**: Pushes a value to the tail (right) of a list. Returns the new list length.
20. **`REDIS_LPOP`**: Removes and returns the first element of a list. Returns NULL if empty.
21. **`REDIS_RPOP`**: Removes and returns the last element of a list. Returns NULL if empty.
22. **`REDIS_LLEN`**: Gets the length of a list. Returns 0 if the key doesn't exist.
23. **`REDIS_LRANGE`**: Gets a range of elements from a list as a comma-separated string. Supports negative indices.
24. **`REDIS_SADD`**: Adds a member to a set. Returns 1 if the member was added, 0 if already a member.
25. **`REDIS_SREM`**: Removes a member from a set. Returns 1 if removed, 0 if not a member.
26. **`REDIS_SISMEMBER`**: Checks if a member exists in a set. Returns 1 if yes, 0 if no.
27. **`REDIS_SCARD`**: Gets the number of members in a set. Returns 0 if the key doesn't exist.
28. **`REDIS_SMEMBERS`**: Gets all members of a set as a comma-separated string. Order may vary.
29. **`REDIS_SETEX`**: Sets a key-value pair with an expiration time (TTL) in one atomic operation. Returns "OK".
30. **`REDIS_INCRBY`**: Increments the integer value of a key by a specified amount. Returns the new value (BIGINT).
31. **`REDIS_DECRBY`**: Decrements the integer value of a key by a specified amount. Returns the new value (BIGINT).
32. **`REDIS_PERSIST`**: Removes the expiration from a key. Returns 1 if the timeout was removed, 0 otherwise.
33. **`REDIS_TYPE`**: Returns the type of value stored at a key ("string", "list", "set", "hash", "zset", or "none").
34. **`REDIS_STRLEN`**: Returns the length of the string value stored at a key. Returns 0 if the key doesn't exist.
35. **`REDIS_KEYS`**: Returns all keys matching a glob-style pattern as a comma-separated string. Warning: blocks Redis on large datasets — use SCAN for production.
36. **`REDIS_SCAN`**: Cursor-based key iteration with MATCH/COUNT support. Returns "cursor|key1,key2,..." format. Production-safe alternative to KEYS.
37. **`REDIS_ZADD`**: Adds a member with a score to a sorted set. Returns 1 if the member was added, 0 if the score was updated.
38. **`REDIS_ZREM`**: Removes a member from a sorted set. Returns 1 if removed, 0 if not found.
39. **`REDIS_ZSCORE`**: Gets the score of a member in a sorted set. Returns the score as a string, or NULL if not found.
40. **`REDIS_ZRANK`**: Gets the 0-based rank of a member in a sorted set (lowest score = rank 0). Returns NULL if not found.
41. **`REDIS_ZCARD`**: Gets the number of members in a sorted set. Returns 0 if the key doesn't exist.
42. **`REDIS_ZRANGE`**: Gets a range of members from a sorted set by index as a comma-separated string. Supports negative indices.
43. **`REDIS_ZRANGEBYSCORE`**: Gets members within a score range as a comma-separated string. Supports `-inf`, `+inf`, and exclusive ranges with `(`.
44. **`REDIS_MGET`**: Gets multiple values at once. Takes comma-separated keys, returns comma-separated values (empty string for missing keys).
45. **`REDIS_MSET`**: Sets multiple key-value pairs atomically. Takes "key1=val1,key2=val2" format. Returns "OK".
46. **`REDIS_GETSET`**: Atomically sets a new value and returns the old value. Returns NULL if the key didn't exist.
47. **`REDIS_RENAME`**: Renames a key. Returns "OK". Fails if the source key doesn't exist.
48. **`REDIS_HSCAN`**: Cursor-based hash field iteration. Returns "cursor|field1=value1,field2=value2" format. Production-safe.
49. **`REDIS_SSCAN`**: Cursor-based set member iteration. Returns "cursor|member1,member2" format. Production-safe.
50. **`REDIS_DBSIZE`**: Returns the number of keys in the currently selected Redis database. Takes no parameters. Useful for monitoring.

Built with a Makefile, the project automates compilation, binding, and deployment of these functions, making it easy to integrate Redis caching or storage into IBM i applications.

---

## Table of Contents

1. [Prerequisites](#prerequisites)
2. [Project Structure](#project-structure)
3. [Makefile Targets](#makefile-targets)
4. [Building the Project](#building-the-project)
5. [Cleaning Up](#cleaning-up)
6. [Usage](#usage)
7. [Redis Configuration](#redis-configuration)
8. [License](#license)
9. [Contributing](#contributing)
10. [Authors](#authors)

---

## Prerequisites

Before building and using the project, ensure you have the following:

- **IBM i Access**: Access to an IBM i system with development tools (e.g., ILE C compiler, SQL).
- **Redis Server**: A Redis server running and accessible from the IBM i system (default: `127.0.0.1:6379`).
- **Compiler - ILE C**: Required for compiling C modules in ILE. (5770WDS 51 2911 Compiler - ILE C)
- **Makefile Support**: Ensure the `make` command is available (install via `yum install make` in PASE, if needed).
- **Git**: For cloning the repository (optional, but recommended).

---

## Project Structure

The project is organized as follows:

- `/project-root/`
  - `srcfile/`              # Source files for C modules
    - `redisget.c`        # Source for REDIS_GET function
    - `redisset.c`        # Source for REDIS_SET function
    - `redisincr.c`       # Source for REDIS_INCR function
    - `redisdel.c`        # Source for REDIS_DEL function
    - `redisexp.c`        # Source for REDIS_EXPIRE function
    - `redisttl.c`        # Source for REDIS_TTL function
    - `redisping.c`       # Source for REDIS_PING function
    - `redisapnd.c`       # Source for REDIS_APPEND function
    - `redisexst.c`       # Source for REDIS_EXISTS function
    - `redissnx.c`        # Source for REDIS_SETNX function
    - `redisdecr.c`       # Source for REDIS_DECR function
    - `redisauth.c`       # Source for REDIS_AUTH function
    - `redishset.c`       # Source for REDIS_HSET function
    - `redishget.c`       # Source for REDIS_HGET function
    - `redishdel.c`       # Source for REDIS_HDEL function
    - `redishext.c`       # Source for REDIS_HEXISTS function
    - `redishgta.c`       # Source for REDIS_HGETALL function
    - `redislpsh.c`       # Source for REDIS_LPUSH function
    - `redisrpsh.c`       # Source for REDIS_RPUSH function
    - `redislpop.c`       # Source for REDIS_LPOP function
    - `redisrpop.c`       # Source for REDIS_RPOP function
    - `redisllen.c`       # Source for REDIS_LLEN function
    - `redislrng.c`       # Source for REDIS_LRANGE function
    - `redissadd.c`       # Source for REDIS_SADD function
    - `redissrem.c`       # Source for REDIS_SREM function
    - `redissism.c`       # Source for REDIS_SISMEMBER function
    - `redisscrd.c`       # Source for REDIS_SCARD function
    - `redissmem.c`       # Source for REDIS_SMEMBERS function
    - `redissetx.c`       # Source for REDIS_SETEX function
    - `redisincb.c`       # Source for REDIS_INCRBY function
    - `redisdecb.c`       # Source for REDIS_DECRBY function
    - `redispst.c`        # Source for REDIS_PERSIST function
    - `redistype.c`       # Source for REDIS_TYPE function
    - `redisslen.c`       # Source for REDIS_STRLEN function
    - `rediskeys.c`       # Source for REDIS_KEYS function
    - `redisscan.c`       # Source for REDIS_SCAN function
    - `rediszadd.c`       # Source for REDIS_ZADD function
    - `rediszrem.c`       # Source for REDIS_ZREM function
    - `rediszsco.c`       # Source for REDIS_ZSCORE function
    - `rediszrnk.c`       # Source for REDIS_ZRANK function
    - `rediszcrd.c`       # Source for REDIS_ZCARD function
    - `rediszrng.c`       # Source for REDIS_ZRANGE function
    - `rediszrbs.c`       # Source for REDIS_ZRANGEBYSCORE function
    - `redismget.c`       # Source for REDIS_MGET function
    - `redismset.c`       # Source for REDIS_MSET function
    - `redisgset.c`       # Source for REDIS_GETSET function
    - `redisrnme.c`       # Source for REDIS_RENAME function
    - `redishscn.c`       # Source for REDIS_HSCAN function
    - `redissscn.c`       # Source for REDIS_SSCAN function
    - `redisdbsz.c`       # Source for REDIS_DBSIZE function
    - `redisutils.c`      # Shared utility functions
    - `redisbench.c`      # EBCDIC/ASCII conversion benchmark
  - `qsrvsrc/`              # Binding source files
    - `redisile.bnd`      # Binding source
  - `include/`              # Header files
    - `redis_utils.h`     # Header for shared utilities
  - `Makefile`              # Makefile for building the project
  - `README.md`             # This file

---

## Makefile Targets

The Makefile provides the following targets to manage the build process:

| Target            | Description                                                                |
| ----------------- | -------------------------------------------------------------------------- |
| `all`             | Builds everything: library, service program, and SQL functions.            |
| `preflight`       | Checks if the target library (`REDIS400`) exists and prompts for deletion. |
| `$(TGT_LIB).lib`  | Creates the target library (`REDIS400` by default).                        |
| `redisile.srvpgm` | Creates the service program from compiled modules.                         |
| `redis_get.func`  | Creates or replaces the `REDIS_GET` SQL function.                          |
| `redis_set.func`  | Creates or replaces the `REDIS_SET` SQL function.                          |
| `redis_incr.func` | Creates or replaces the `REDIS_INCR` SQL function.                         |
| `redis_del.func`  | Creates or replaces the `REDIS_DEL` SQL function.                          |
| `redis_exp.func`  | Creates or replaces the `REDIS_EXPIRE` SQL function (added March 12, 2025).|
| `redis_ttl.func`  | Creates or replaces the `REDIS_TTL` SQL function (added March 12, 2025).   |
| `redis_ping.func` | Creates or replaces the `REDIS_PING` SQL function.                          |
| `redis_append.func` | Creates or replaces the `REDIS_APPEND` SQL function.                     |
| `redis_exists.func` | Creates or replaces the `REDIS_EXISTS` SQL function.                     |
| `redis_setnx.func` | Creates or replaces the `REDIS_SETNX` SQL function.                       |
| `redis_decr.func` | Creates or replaces the `REDIS_DECR` SQL function.                          |
| `redis_auth.func` | Creates or replaces the `REDIS_AUTH` SQL function.                          |
| `redis_hset.func` | Creates or replaces the `REDIS_HSET` SQL function.                          |
| `redis_hget.func` | Creates or replaces the `REDIS_HGET` SQL function.                          |
| `redis_hdel.func` | Creates or replaces the `REDIS_HDEL` SQL function.                          |
| `redis_hexists.func` | Creates or replaces the `REDIS_HEXISTS` SQL function.                    |
| `redis_hgetall.func` | Creates or replaces the `REDIS_HGETALL` SQL function.                    |
| `redis_lpush.func` | Creates or replaces the `REDIS_LPUSH` SQL function.                       |
| `redis_rpush.func` | Creates or replaces the `REDIS_RPUSH` SQL function.                       |
| `redis_lpop.func` | Creates or replaces the `REDIS_LPOP` SQL function.                         |
| `redis_rpop.func` | Creates or replaces the `REDIS_RPOP` SQL function.                         |
| `redis_llen.func` | Creates or replaces the `REDIS_LLEN` SQL function.                         |
| `redis_lrange.func` | Creates or replaces the `REDIS_LRANGE` SQL function.                     |
| `redis_sadd.func` | Creates or replaces the `REDIS_SADD` SQL function.                        |
| `redis_srem.func` | Creates or replaces the `REDIS_SREM` SQL function.                        |
| `redis_sismember.func` | Creates or replaces the `REDIS_SISMEMBER` SQL function.              |
| `redis_scard.func` | Creates or replaces the `REDIS_SCARD` SQL function.                      |
| `redis_smembers.func` | Creates or replaces the `REDIS_SMEMBERS` SQL function.                |
| `redis_setex.func` | Creates or replaces the `REDIS_SETEX` SQL function.                      |
| `redis_incrby.func` | Creates or replaces the `REDIS_INCRBY` SQL function.                    |
| `redis_decrby.func` | Creates or replaces the `REDIS_DECRBY` SQL function.                    |
| `redis_persist.func` | Creates or replaces the `REDIS_PERSIST` SQL function.                  |
| `redis_type.func` | Creates or replaces the `REDIS_TYPE` SQL function.                        |
| `redis_strlen.func` | Creates or replaces the `REDIS_STRLEN` SQL function.                    |
| `redis_keys.func` | Creates or replaces the `REDIS_KEYS` SQL function.                        |
| `redis_scan.func` | Creates or replaces the `REDIS_SCAN` SQL function.                        |
| `redis_zadd.func` | Creates or replaces the `REDIS_ZADD` SQL function.                        |
| `redis_zrem.func` | Creates or replaces the `REDIS_ZREM` SQL function.                        |
| `redis_zscore.func` | Creates or replaces the `REDIS_ZSCORE` SQL function.                    |
| `redis_zrank.func` | Creates or replaces the `REDIS_ZRANK` SQL function.                      |
| `redis_zcard.func` | Creates or replaces the `REDIS_ZCARD` SQL function.                      |
| `redis_zrange.func` | Creates or replaces the `REDIS_ZRANGE` SQL function.                    |
| `redis_zrangebyscore.func` | Creates or replaces the `REDIS_ZRANGEBYSCORE` SQL function.      |
| `redis_mget.func` | Creates or replaces the `REDIS_MGET` SQL function.                        |
| `redis_mset.func` | Creates or replaces the `REDIS_MSET` SQL function.                        |
| `redis_getset.func` | Creates or replaces the `REDIS_GETSET` SQL function.                    |
| `redis_rename.func` | Creates or replaces the `REDIS_RENAME` SQL function.                    |
| `redis_hscan.func` | Creates or replaces the `REDIS_HSCAN` SQL function.                      |
| `redis_sscan.func` | Creates or replaces the `REDIS_SSCAN` SQL function.                      |
| `redis_dbsize.func` | Creates or replaces the `REDIS_DBSIZE` SQL function.                    |
| `bench`           | Builds the EBCDIC/ASCII conversion benchmark program.                      |
| `clean`           | Deletes the target library and all associated objects.                      |

---

## Building the Project

Follow these steps to build and deploy the Redis utility functions on IBM i:

### Prerequisites Check

Ensure all prerequisites are installed and configured as described above.

### Steps

1. **Clone the Repository**:

   ```bash
   git clone https://github.com/krakadin/redis400.git
   cd redis400
   ```

2. **Run the Makefile**:

   ```bash
   gmake
   ```

## Build Steps

When you run `gmake`, the following steps are executed:

1. **Create the Target Library (`REDIS400`)**:

   - The target library `REDIS400` is created to store all the compiled objects.

2. **Compile the C Modules into ILE Modules**:

   - The C source files are compiled into ILE modules:
     - `REDISGET`, `REDISSET`, `REDISINCR`, `REDISDEL`, `REDISEXP`, `REDISTTL`, `REDISPING`, `REDISAPND`, `REDISEXST`, `REDISSNX`, `REDISDECR`, `REDISAUTH`, `REDISHSET`, `REDISHGET`, `REDISHDEL`, `REDISHEXT`, `REDISHGTA`, `REDISLPSH`, `REDISRPSH`, `REDISLPOP`, `REDISRPOP`, `REDISLLEN`, `REDISLRNG`, `REDISSADD`, `REDISSREM`, `REDISSISM`, `REDISSCRD`, `REDISSMEM`, `REDISSETX`, `REDISINCB`, `REDISDECB`, `REDISPST`, `REDISTYPE`, `REDISSLEN`, `REDISKEYS`, `REDISSCAN`, `REDISZADD`, `REDISZREM`, `REDISZSCO`, `REDISZRNK`, `REDISZCRD`, `REDISZRNG`, `REDISZRBS`, `REDISMGET`, `REDISMSET`, `REDISGSET`, `REDISRNME`, `REDISHSCN`, `REDISSSCN`, `REDISDBSZ`, `REDISUTILS`

3. **Create the Service Program (`redisile.srvpgm`)**:

   - The compiled modules are bound together to create the service program `REDISILE`.

4. **Create the SQL Functions**:
   - All 50 SQL functions (`REDIS_GET`, `REDIS_SET`, `REDIS_INCR`, `REDIS_DEL`, `REDIS_EXPIRE`, `REDIS_TTL`, `REDIS_PING`, `REDIS_APPEND`, `REDIS_EXISTS`, `REDIS_SETNX`, `REDIS_DECR`, `REDIS_AUTH`, `REDIS_HSET`, `REDIS_HGET`, `REDIS_HDEL`, `REDIS_HEXISTS`, `REDIS_HGETALL`, `REDIS_LPUSH`, `REDIS_RPUSH`, `REDIS_LPOP`, `REDIS_RPOP`, `REDIS_LLEN`, `REDIS_LRANGE`, `REDIS_SADD`, `REDIS_SREM`, `REDIS_SISMEMBER`, `REDIS_SCARD`, `REDIS_SMEMBERS`, `REDIS_SETEX`, `REDIS_INCRBY`, `REDIS_DECRBY`, `REDIS_PERSIST`, `REDIS_TYPE`, `REDIS_STRLEN`, `REDIS_KEYS`, `REDIS_SCAN`, `REDIS_ZADD`, `REDIS_ZREM`, `REDIS_ZSCORE`, `REDIS_ZRANK`, `REDIS_ZCARD`, `REDIS_ZRANGE`, `REDIS_ZRANGEBYSCORE`, `REDIS_MGET`, `REDIS_MSET`, `REDIS_GETSET`, `REDIS_RENAME`, `REDIS_HSCAN`, `REDIS_SSCAN`, `REDIS_DBSIZE`) are created or replaced in the target library.

**Note**: If the execution failes due errors in `generate_config.sh`, you may need to modify git settings `core.autocrlf`

```bash
git config --global core.autocrlf input
```

---

## Verify the Build

After the build process completes, verify the following objects in the target library (`REDIS400`):

### Modules

- `REDISGET`, `REDISSET`, `REDISINCR`, `REDISDEL`, `REDISEXP`, `REDISTTL`, `REDISPING`, `REDISAPND`, `REDISEXST`, `REDISSNX`, `REDISDECR`, `REDISAUTH`, `REDISHSET`, `REDISHGET`, `REDISHDEL`, `REDISHEXT`, `REDISHGTA`, `REDISLPSH`, `REDISRPSH`, `REDISLPOP`, `REDISRPOP`, `REDISLLEN`, `REDISLRNG`, `REDISSADD`, `REDISSREM`, `REDISSISM`, `REDISSCRD`, `REDISSMEM`, `REDISSETX`, `REDISINCB`, `REDISDECB`, `REDISPST`, `REDISTYPE`, `REDISSLEN`, `REDISKEYS`, `REDISSCAN`, `REDISZADD`, `REDISZREM`, `REDISZSCO`, `REDISZRNK`, `REDISZCRD`, `REDISZRNG`, `REDISZRBS`, `REDISMGET`, `REDISMSET`, `REDISGSET`, `REDISRNME`, `REDISHSCN`, `REDISSSCN`, `REDISDBSZ`, `REDISUTILS`

### Service Program

- `REDISILE`

### SQL Functions

- `REDIS_GET`, `REDIS_SET`, `REDIS_INCR`, `REDIS_DEL`, `REDIS_EXPIRE`, `REDIS_TTL`, `REDIS_PING`, `REDIS_APPEND`, `REDIS_EXISTS`, `REDIS_SETNX`, `REDIS_DECR`, `REDIS_AUTH`, `REDIS_HSET`, `REDIS_HGET`, `REDIS_HDEL`, `REDIS_HEXISTS`, `REDIS_HGETALL`, `REDIS_LPUSH`, `REDIS_RPUSH`, `REDIS_LPOP`, `REDIS_RPOP`, `REDIS_LLEN`, `REDIS_LRANGE`, `REDIS_SADD`, `REDIS_SREM`, `REDIS_SISMEMBER`, `REDIS_SCARD`, `REDIS_SMEMBERS`, `REDIS_SETEX`, `REDIS_INCRBY`, `REDIS_DECRBY`, `REDIS_PERSIST`, `REDIS_TYPE`, `REDIS_STRLEN`, `REDIS_KEYS`, `REDIS_SCAN`, `REDIS_ZADD`, `REDIS_ZREM`, `REDIS_ZSCORE`, `REDIS_ZRANK`, `REDIS_ZCARD`, `REDIS_ZRANGE`, `REDIS_ZRANGEBYSCORE`, `REDIS_MGET`, `REDIS_MSET`, `REDIS_GETSET`, `REDIS_RENAME`, `REDIS_HSCAN`, `REDIS_SSCAN`, `REDIS_DBSIZE`

---

1. **Check Modules**:

   ```CL
      DSPOBJD OBJ(REDIS400/REDISGET) OBJTYPE(*MODULE)
      DSPOBJD OBJ(REDIS400/REDISSET) OBJTYPE(*MODULE)
      DSPOBJD OBJ(REDIS400/REDISINCR) OBJTYPE(*MODULE)
      DSPOBJD OBJ(REDIS400/REDISDEL) OBJTYPE(*MODULE)
      DSPOBJD OBJ(REDIS400/REDISEXP) OBJTYPE(*MODULE)
      DSPOBJD OBJ(REDIS400/REDISTTL) OBJTYPE(*MODULE)
      DSPOBJD OBJ(REDIS400/REDISPING) OBJTYPE(*MODULE)
      DSPOBJD OBJ(REDIS400/REDISUTILS) OBJTYPE(*MODULE)
   ```

2. **Check Service Program**:

   ```CL
   DSPOBJD OBJ(REDIS400/REDISILE) OBJTYPE(\*SRVPGM)
   ```

3. **Check SQL Functions**:

   ```sql
   SELECT * FROM QSYS2.SYSFUNCS WHERE SPECIFIC_NAME = 'REDIS_GET');
   SELECT * FROM QSYS2.SYSFUNCS WHERE SPECIFIC_NAME = 'REDIS_SET');
   SELECT * FROM QSYS2.SYSFUNCS WHERE SPECIFIC_NAME = 'REDIS_INCR');
   SELECT * FROM QSYS2.SYSFUNCS WHERE SPECIFIC_NAME = 'REDIS_DEL');
   SELECT * FROM QSYS2.SYSFUNCS WHERE SPECIFIC_NAME = 'REDIS_EXPIRE';
   SELECT * FROM QSYS2.SYSFUNCS WHERE SPECIFIC_NAME = 'REDIS_TTL';
   SELECT * FROM QSYS2.SYSFUNCS WHERE SPECIFIC_NAME = 'REDIS_PING';
   ```

## Cleaning Up

To clean up the project and remove all generated objects, run:

```bash
make clean
```

This deletes the REDIS400 library and all its contents.

## Usage

Once built, use the SQL functions in your IBM i SQL queries to interact with Redis.

### Basic Usage

- Retrieve a Value (REDIS_GET): Use this function to get a value associated with a key from Redis.
- Set a Value (REDIS_SET): Use this function to set a key-value pair in Redis.
- Increment a Value (REDIS_INCR): Use this function to increment the integer value of a key by 1.
- Delete a Key (REDIS_DEL): Remove a key from Redis.
- Set Expiration (REDIS_EXPIRE): Assign a time-to-live (TTL) to a key.
- Check TTL (REDIS_TTL): Retrieve the remaining time-to-live of a key.
- Test Connectivity (REDIS_PING): Send a PING command to verify the Redis server is responsive.

### Examples

#### Using REDIS_GET

```sql
SELECT REDIS_GET('API_KEY') AS value FROM SYSIBM.SYSDUMMY1;
```

#### Using REDIS_SET

```sql
VALUES REDIS_SET('API_KEY', 'my_value');
```

#### Using REDIS_INCR

```sql
SET ORDER_NO = REDIS_INCR('ORDER#');
```

#### Using REDIS_DEL

```sql
VALUES REDIS_DEL('API_KEY');
```

#### Using REDIS_EXPIRE

```sql
SELECT REDIS_EXPIRE('API_KEY', 300) FROM SYSIBM.SYSDUMMY1;
```

- Sets API_KEY to expire in 300 seconds (5 minutes). Returns 1 if successful, 0 if the key doesn’t exist.

#### Using REDIS_TTL

```sql
SELECT REDIS_TTL('API_KEY') FROM SYSIBM.SYSDUMMY1;
```

- Returns the remaining TTL in seconds (e.g., 298), -1 if no expiration, or -2 if the key doesn’t exist.

#### Using REDIS_PING

```sql
VALUES(REDIS_PING());
```

- Returns `PONG` if the Redis server is responsive. Useful for testing connectivity.

#### Using REDIS_APPEND

```sql
SELECT REDIS_APPEND('mykey', ' world') FROM SYSIBM.SYSDUMMY1;
```

- Appends " world" to the existing value of `mykey`. Returns the new total string length.

#### Using REDIS_EXISTS

```sql
SELECT REDIS_EXISTS('mykey') FROM SYSIBM.SYSDUMMY1;
```

- Returns 1 if the key exists, 0 if it does not.

#### Using REDIS_SETNX

```sql
VALUES REDIS_SETNX('lock_key', 'locked');
```

- Sets `lock_key` only if it doesn't already exist (atomic). Returns 1 if the key was set, 0 if it already existed. Useful for distributed locks.

#### Using REDIS_DECR

```sql
SELECT REDIS_DECR('counter') FROM SYSIBM.SYSDUMMY1;
```

- Decrements the integer value of `counter` by 1. Returns the new value (can go negative).

#### Using REDIS_AUTH

```sql
VALUES REDIS_AUTH('your_redis_password');
```

- Authenticates with the Redis server. Returns "OK" on success. Required before other commands if Redis has `requirepass` set.

#### Using REDIS_HSET

```sql
VALUES REDIS_HSET('user:1', 'name', 'John Doe');
VALUES REDIS_HSET('user:1', 'email', 'john@example.com');
VALUES REDIS_HSET('user:1', 'age', '30');
```

- Sets a field in a hash. Returns 1 if the field is new, 0 if an existing field was updated.

#### Using REDIS_HGET

```sql
SELECT REDIS_HGET('user:1', 'name') FROM SYSIBM.SYSDUMMY1;
```

- Retrieves the value of a specific field from a hash. Returns NULL if the field or key does not exist.

#### Using REDIS_HDEL

```sql
VALUES REDIS_HDEL('user:1', 'email');
```

- Deletes a field from a hash. Returns 1 if the field was deleted, 0 if the field did not exist.

#### Using REDIS_HEXISTS

```sql
SELECT REDIS_HEXISTS('user:1', 'name') FROM SYSIBM.SYSDUMMY1;
```

- Returns 1 if the field exists in the hash, 0 if it does not.

#### Using REDIS_HGETALL

```sql
SELECT REDIS_HGETALL('user:1') FROM SYSIBM.SYSDUMMY1;
```

- Returns all fields and values as comma-separated pairs: `name=John Doe,age=30,city=NYC`. Returns NULL for empty or non-existent hashes.

#### Using REDIS_LPUSH / REDIS_RPUSH

```sql
-- Build a queue: push to head (left) or tail (right)
VALUES REDIS_LPUSH('queue', 'second');  -- Returns 1 (list length)
VALUES REDIS_LPUSH('queue', 'first');   -- Returns 2
VALUES REDIS_RPUSH('queue', 'third');   -- Returns 3
```

- LPUSH adds to the head (left), RPUSH adds to the tail (right). Returns the new list length after the push.

#### Using REDIS_LPOP / REDIS_RPOP

```sql
SELECT REDIS_LPOP('queue') FROM SYSIBM.SYSDUMMY1;  -- Returns 'first'
SELECT REDIS_RPOP('queue') FROM SYSIBM.SYSDUMMY1;  -- Returns 'third'
```

- LPOP removes and returns the first element, RPOP removes and returns the last. Returns NULL if the list is empty.

#### Using REDIS_LLEN

```sql
SELECT REDIS_LLEN('queue') FROM SYSIBM.SYSDUMMY1;
```

- Returns the number of elements in the list. Returns 0 if the key doesn't exist.

#### Using REDIS_LRANGE

```sql
-- Get all elements
SELECT REDIS_LRANGE('queue', 0, -1) FROM SYSIBM.SYSDUMMY1;
-- Returns: 'first,second,third'

-- Get first 2 elements
SELECT REDIS_LRANGE('queue', 0, 1) FROM SYSIBM.SYSDUMMY1;
-- Returns: 'first,second'
```

- Returns elements as a comma-separated string. Supports negative indices (-1 = last element). Returns NULL for empty or non-existent lists.

#### Using REDIS_SADD

```sql
-- Build a set of unique tags
VALUES REDIS_SADD('tags', 'redis');     -- Returns 1 (added)
VALUES REDIS_SADD('tags', 'ibmi');      -- Returns 1 (added)
VALUES REDIS_SADD('tags', 'redis');     -- Returns 0 (already a member)
```

- Adds a member to a set. Returns 1 if the member was newly added, 0 if it was already present. Sets never contain duplicates.

#### Using REDIS_SREM

```sql
VALUES REDIS_SREM('tags', 'redis');     -- Returns 1 (removed)
VALUES REDIS_SREM('tags', 'unknown');   -- Returns 0 (not a member)
```

- Removes a member from a set. Returns 1 if the member was removed, 0 if it was not a member.

#### Using REDIS_SISMEMBER

```sql
SELECT REDIS_SISMEMBER('tags', 'ibmi') FROM SYSIBM.SYSDUMMY1;
```

- Returns 1 if the member exists in the set, 0 if it does not.

#### Using REDIS_SCARD

```sql
SELECT REDIS_SCARD('tags') FROM SYSIBM.SYSDUMMY1;
```

- Returns the number of members in the set. Returns 0 if the key doesn't exist.

#### Using REDIS_SMEMBERS

```sql
SELECT REDIS_SMEMBERS('tags') FROM SYSIBM.SYSDUMMY1;
-- Returns: 'redis,ibmi' (order may vary)
```

- Returns all members of the set as a comma-separated string. Since Redis sets are unordered, the member order may vary between calls. Returns NULL for empty or non-existent sets.

#### Using REDIS_SETEX

```sql
VALUES REDIS_SETEX('session:abc', 3600, 'user_data');
```

- Sets key-value pair with expiration in one atomic operation. Equivalent to SET + EXPIRE but atomic. TTL is in seconds. Returns "OK".

#### Using REDIS_INCRBY

```sql
VALUES REDIS_INCRBY('page_views', 10);
```

- Increments the integer value of a key by the specified amount. Returns the new value as BIGINT. Creates the key with value 0 before incrementing if it doesn't exist.

#### Using REDIS_DECRBY

```sql
VALUES REDIS_DECRBY('stock_count', 5);
```

- Decrements the integer value of a key by the specified amount. Returns the new value as BIGINT (can go negative).

#### Using REDIS_PERSIST

```sql
SELECT REDIS_PERSIST('session:abc') FROM SYSIBM.SYSDUMMY1;
```

- Removes the expiration from a key, making it persistent. Returns 1 if the timeout was removed, 0 if the key has no timeout or doesn't exist.

#### Using REDIS_TYPE

```sql
SELECT REDIS_TYPE('mykey') FROM SYSIBM.SYSDUMMY1;
```

- Returns the type of value stored at key: "string", "list", "set", "hash", "zset", or "none" if the key doesn't exist.

#### Using REDIS_STRLEN

```sql
SELECT REDIS_STRLEN('mykey') FROM SYSIBM.SYSDUMMY1;
```

- Returns the length of the string value stored at key. Returns 0 if the key doesn't exist.

#### Using REDIS_KEYS

```sql
-- Find all keys matching a pattern
SELECT REDIS_KEYS('user:*') FROM SYSIBM.SYSDUMMY1;
-- Returns: 'user:1,user:2,user:3' (comma-separated, order may vary)

-- Find all keys starting with 'session:'
SELECT REDIS_KEYS('session:*') FROM SYSIBM.SYSDUMMY1;
```

- Returns all keys matching the glob-style pattern as a comma-separated string. Returns NULL if no keys match. **Warning**: KEYS blocks the Redis server while scanning — use SCAN for production workloads.

#### Using REDIS_SCAN

```sql
-- Scan all keys matching 'user:*' (cursor-based, production-safe)
SELECT REDIS_SCAN('0', 'user:*', 100) FROM SYSIBM.SYSDUMMY1;
-- Returns: '0|user:1,user:2,user:3' (cursor 0 = scan complete)

-- If cursor is not 0, call again with returned cursor:
-- First call returns: '17|user:1,user:2'
-- Second call: SELECT REDIS_SCAN('17', 'user:*', 100) FROM SYSIBM.SYSDUMMY1;
-- Returns: '0|user:3' (cursor 0 = done)
```

- Iterates keys using a cursor. Returns "cursor|key1,key2,..." where the pipe `|` separates the cursor from the key list. When cursor returns "0", the full scan is complete. The third parameter (CNT) is a hint to Redis for how many keys to return per iteration. Safe for production use — does not block Redis.

#### Using REDIS_ZADD

```sql
-- Build a leaderboard sorted set
VALUES REDIS_ZADD('leaderboard', 100.0, 'player1');  -- Returns 1 (added)
VALUES REDIS_ZADD('leaderboard', 250.5, 'player2');  -- Returns 1 (added)
VALUES REDIS_ZADD('leaderboard', 100.0, 'player1');  -- Returns 0 (score updated)
```

- Adds a member with a floating-point score to a sorted set. Returns 1 if the member was newly added, 0 if the score of an existing member was updated.

#### Using REDIS_ZREM

```sql
VALUES REDIS_ZREM('leaderboard', 'player1');  -- Returns 1 (removed)
VALUES REDIS_ZREM('leaderboard', 'unknown');  -- Returns 0 (not found)
```

- Removes a member from a sorted set. Returns 1 if removed, 0 if not a member.

#### Using REDIS_ZSCORE

```sql
SELECT REDIS_ZSCORE('leaderboard', 'player2') FROM SYSIBM.SYSDUMMY1;
-- Returns: '250.5'
```

- Returns the score of a member as a string. Returns NULL if the member or key doesn't exist.

#### Using REDIS_ZRANK

```sql
SELECT REDIS_ZRANK('leaderboard', 'player1') FROM SYSIBM.SYSDUMMY1;
-- Returns: 0 (lowest score = rank 0)
```

- Returns the 0-based rank of a member, ordered by score ascending. Returns NULL if the member doesn't exist.

#### Using REDIS_ZCARD

```sql
SELECT REDIS_ZCARD('leaderboard') FROM SYSIBM.SYSDUMMY1;
```

- Returns the number of members in the sorted set. Returns 0 if the key doesn't exist.

#### Using REDIS_ZRANGE

```sql
-- Get all members ordered by score
SELECT REDIS_ZRANGE('leaderboard', 0, -1) FROM SYSIBM.SYSDUMMY1;
-- Returns: 'player1,player2'

-- Get top 3 members
SELECT REDIS_ZRANGE('leaderboard', 0, 2) FROM SYSIBM.SYSDUMMY1;
```

- Returns members as a comma-separated string, ordered by score. Supports negative indices (-1 = last member).

#### Using REDIS_ZRANGEBYSCORE

```sql
-- Get all members with scores between 100 and 500
SELECT REDIS_ZRANGEBYSCORE('leaderboard', '100', '500') FROM SYSIBM.SYSDUMMY1;

-- Get all members (use -inf and +inf)
SELECT REDIS_ZRANGEBYSCORE('leaderboard', '-inf', '+inf') FROM SYSIBM.SYSDUMMY1;

-- Exclusive range (scores > 100 and < 500)
SELECT REDIS_ZRANGEBYSCORE('leaderboard', '(100', '(500') FROM SYSIBM.SYSDUMMY1;
```

- Returns members within a score range as a comma-separated string. Supports `-inf`, `+inf` for unbounded ranges, and `(` prefix for exclusive bounds.

#### Using REDIS_MGET

```sql
SELECT REDIS_MGET('key1,key2,key3') FROM SYSIBM.SYSDUMMY1;
-- Returns: 'value1,value2,value3'
-- Missing keys return empty: 'value1,,value3'
```

- Gets multiple values in one round-trip. Input and output are comma-separated. Missing keys produce empty strings between commas.

#### Using REDIS_MSET

```sql
VALUES REDIS_MSET('key1=value1,key2=value2,key3=value3');
-- Returns: 'OK'
```

- Sets multiple key-value pairs atomically. Input format is `key1=val1,key2=val2`. Always returns "OK" (MSET never fails).

#### Using REDIS_GETSET

```sql
-- Reset a counter and get the old value
SELECT REDIS_GETSET('counter', '0') FROM SYSIBM.SYSDUMMY1;
-- Returns the previous value of 'counter'
```

- Atomically sets a new value and returns the old value. Returns NULL if the key didn't exist before.

#### Using REDIS_RENAME

```sql
VALUES REDIS_RENAME('old_key', 'new_key');
-- Returns: 'OK'
```

- Renames a key. Returns "OK" on success. Fails with an error if the source key doesn't exist.

#### Using REDIS_HSCAN

```sql
-- Scan all hash fields matching pattern
SELECT REDIS_HSCAN('user:1', '0', '*', 100) FROM SYSIBM.SYSDUMMY1;
-- Returns: '0|name=John,age=30,city=NYC'

-- If cursor is not 0, call again:
-- First call: '17|name=John,age=30'
-- Second: SELECT REDIS_HSCAN('user:1', '17', '*', 100) FROM SYSIBM.SYSDUMMY1;
-- Returns: '0|city=NYC'
```

- Cursor-based hash field iteration. Returns "cursor|field1=value1,field2=value2" where `|` separates the cursor from field=value pairs. Cursor "0" means scan complete.

#### Using REDIS_SSCAN

```sql
-- Scan all set members matching pattern
SELECT REDIS_SSCAN('tags', '0', '*', 100) FROM SYSIBM.SYSDUMMY1;
-- Returns: '0|redis,ibmi,sql'
```

- Cursor-based set member iteration. Returns "cursor|member1,member2" format. Same cursor logic as SCAN and HSCAN.

#### Using REDIS_DBSIZE

```sql
VALUES(REDIS_DBSIZE());
-- Returns: 42 (number of keys in the database)
```

- Returns the total number of keys in the currently selected Redis database. Takes no parameters. Useful for monitoring database size and health checks.

### Notes

Ensure the Redis server is running and accessible at `127.0.0.1:6379` (configurable in `.env`).
Handle errors via SQLSTATE (e.g., `38908` for payload extraction failures, `38904` for timeouts).

## Redis Configuration

This section details how to configure and optimize your Redis server for use with the IBM i UDFs.

### Redis Server Setup

- **Installation**: Install Redis on IBM i PASE or a remote server using yum install redis in PASE, or configure a remote Redis instance.
- **Configuration**:
  - Edit `/QOpenSys/etc/redis.conf` (if on PASE) or the Redis configuration file to bind to `127.0.0.1` and listen on port `6379`.
  - If using `requirepass` in `redis.conf`, use `REDIS_AUTH('password')` before other commands to authenticate.
- **Starting Redis**:

```bash
redis-server /QOpenSys/etc/redis.conf --daemonize yes
```

- **Stopping Redis**:

```bash
redis-cli SHUTDOWN
```

- **Testing Connectivity**:

```bash
redis-cli -h 127.0.0.1 -p 6379 PING
```

Expected output: `PONG`.

### Troubleshooting

- If `REDIS_SET` or `REDIS_GET` fails with timeouts, verify network connectivity and increase the socket timeout in `connect_to_redis` (e.g., `timeout.tv_sec = 5`).
- For `-ERR unknown command`, ensure the RESP format is correct (e.g., `\*3\r\n$3\r\nSET\r\n...`).
- If you encounter `NOAUTH Authentication required`, authenticate first using `VALUES REDIS_AUTH('your_password')` before running other Redis commands.

## EBCDIC/ASCII Conversion

IBM i uses EBCDIC encoding while Redis uses ASCII. This project supports two conversion methods:

### Static Lookup Tables (default)

The default method uses hardcoded 256-byte translation tables for CCSID 37 (US English EBCDIC). This is the fastest option for typical Redis operations.

### iconv via QtqIconvOpen (optional)

For systems running non-English EBCDIC code pages (e.g., Russian CCSID 1025, German CCSID 273, French CCSID 297), the static tables will produce incorrect results for characters outside the 7-bit ASCII range. In this case, enable iconv-based conversion which auto-detects the job's CCSID.

To enable iconv, edit the `.env` file at the project root:

```bash
# In .env, change:
USE_ICONV=0

# To:
USE_ICONV=1
```

Then rebuild with `gmake clean && gmake all`. The makefile reads `.env` and automatically sets the correct compiler flag (`DEFINE(USE_ICONV)`) and linker flag (`BNDSRVPGM(QSYS/QTQICONV)`).

### When to Use Which

| Scenario | Recommended Method |
|----------|-------------------|
| English system (CCSID 37), keys/values are ASCII only | Static tables (default) |
| Non-English system (CCSID 1025, 273, 277, 280, etc.) | iconv (`USE_ICONV`) |
| Keys/values contain accented or international characters | iconv (`USE_ICONV`) |
| Maximum performance on small operations | Static tables (default) |
| Large values (16KB+) | iconv (faster for large buffers) |

### Benchmark

A built-in benchmark compares both methods on your system:

```bash
gmake bench
CALL REDIS400/REDISBENCH
```

Sample results (IBM i POWER7, CCSID 37):

| Data Size | Static Table | iconv | Winner |
|-----------|-------------|-------|--------|
| 32 bytes (key) | 6.7 ns/byte | 71.9 ns/byte | Table 10.7x faster |
| 200 bytes (command) | 6.1 ns/byte | 10.6 ns/byte | Table 1.7x faster |
| 16,370 bytes (value) | 6.0 ns/byte | 1.1 ns/byte | iconv 5.4x faster |

The benchmark also checks correctness: if it reports `FAIL`, it means the static tables produce wrong results for your system's CCSID and you should switch to iconv.

## License

This project is licensed under the MIT License. See the LICENSE file for details.

## Contributing

Contributions are welcome! To contribute to this project:

1. Fork the repository on GitHub.
2. Create a new branch for your feature or bug fix:

   ```bash
   git checkout -b feature/your-feature
   ```

3. Make your changes, commit them, and push to your fork:

   ```bash
   git add .
   git commit -m "Add your feature or fix"
   git push origin feature/your-feature
   ```

4. Submit a pull request to the main repository.
   Please follow the coding standards in the C source files and test your changes on IBM i.

## Authors

- Ernest Rozloznik

### Additional Notes

- Roadmap: Future enhancements may include auto-authentication via `.env` config, pub/sub support (`PUBLISH`, `SUBSCRIBE`), Lua scripting (`EVAL`), transaction support (`MULTI`/`EXEC`), and improved error handling.
- Acknowledgments: Thanks to the IBM i and Redis communities for their tools and support.
