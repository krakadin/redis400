# Redis SQL Functions for IBM i

## Description

This project provides Redis utility functions for IBM i, enabling seamless interaction with a Redis server through SQL User-Defined Functions (UDFs). It includes four main functions:

1. **`REDIS_GET`**: Retrieves a value from Redis using a specified key.
2. **`REDIS_SET`**: Sets a value in Redis using a key-value pair.
3. **`REDIS_INCR`**: Increments the integer value of a key by 1. If the key does not exist, it is set to 0 before performing the operation.
4. **`REDIS_DEL`**: Deletes a key from Redis. Returns 1 if the key was deleted, or 0 if the key did not exist.
5. **`REDIS_EXPIRE`** (New as of March 12, 2025): Sets an expiration time (TTL) in seconds for a Redis key.
6. **`REDIS_TTL`** (New as of March 12, 2025): Retrieves the remaining time-to-live (TTL) of a Redis key in seconds.
7. **`REDIS_PING`** (New as of March 30, 2025): Sends a `PING` command to the Redis server and returns the response (`PONG`). Useful for testing connectivity to the Redis server.

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
    - `redisexp.c`        # Source for REDIS_EXPIRE function (added March 12, 2025)
    - `redisttl.c`        # Source for REDIS_TTL function (added March 12, 2025)
    - `redisping.c`       # Source for REDIS_PING function (added March 30, 2025)
    - `redisutils.c`      # Shared utility functions
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
| `redis_ping.func` | Creates or replaces the `REDIS_PING` SQL function (added March 30, 2025).   |
| `clean`           | Deletes the target library and all associated objects.                     |

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

   - The C source files (redisget.c, redisset.c, redisincr.c, redisdel.c, redisexp.c, redisttl.c, and redisutils.c) are compiled into ILE modules:
     - `REDISGET`
     - `REDISSET`
     - `REDISINCR`
     - `REDISDEL`
     - `REDISEXP`
     - `REDISTTL`
     - `REDISPING`
     - `REDISUTILS`

3. **Create the Service Program (`redisile.srvpgm`)**:

   - The compiled modules are bound together to create the service program `REDISILE`.

4. **Create the SQL Functions**:
   - The SQL functions `REDIS_GET`, `REDIS_SET`, `REDIS_INCR`, `REDIS_DEL`, `REDIS_EXPIRE`, `REDIS_TTL` and `REDIS_PING` are created or replaced in the target library.

**Note**: If the execution failes due errors in `generate_config.sh`, you may need to modify git settings `core.autocrlf`

```bash
git config --global core.autocrlf input
```

---

## Verify the Build

After the build process completes, verify the following objects in the target library (`REDIS400`):

### Modules

- `REDISGET`
- `REDISSET`
- `REDISINCR`
- `REDISDEL`
- `REDISEXP`
- `REDISTTL`
- `REDISPING`
- `REDISUTILS`

### Service Program

- `REDISILE`

### SQL Functions

- `REDIS_GET`
- `REDIS_SET`
- `REDIS_INCR`
- `REDIS_DEL`
- `REDIS_EXPIRE`
- `REDIS_TTL`
- `REDIS_PING`

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
### Notes

Ensure the Redis server is running and accessible at `127.0.0.1:6379` (configurable in `.env`).
Handle errors via SQLSTATE (e.g., `38908` for payload extraction failures, `38904` for timeouts).

## Redis Configuration

This section details how to configure and optimize your Redis server for use with the IBM i UDFs.

### Redis Server Setup

- **Installation**: Install Redis on IBM i PASE or a remote server using yum install redis in PASE, or configure a remote Redis instance.
- **Configuration**:
  - Edit `/QOpenSys/etc/redis.conf` (if on PASE) or the Redis configuration file to bind to `127.0.0.1` and listen on port `6379`.
  - **Note**: This project currently assumes an unauthenticated Redis server (i.e., `requirepass` is not set). Support for Redis authentication (`AUTH` command) may be added in a future release.
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
- If you encounter `NOAUTH Authentication required`, the Redis server requires a password (`requirepass` is set). This project does not yet support authentication; either disable `requirepass` in `redis.conf` or use an unauthenticated Redis instance.

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

- Roadmap: Future enhancements may include additional Redis commands (e.g., `APPEND`, `AUTH`, `SCAN`), Python integration, and improved error handling.
- Acknowledgments: Thanks to the IBM i and Redis communities for their tools and support.
