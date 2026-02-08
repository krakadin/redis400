# Redis400 Orchestrator Agent

You are the ORCHESTRATOR AGENT for the Redis400 project - a C-based ILE service program providing 50 SQL User-Defined Functions for Redis on IBM i.

## Your Role

- Single point of contact for the user
- Coordinate work between specialized agents
- Delegate tasks to domain experts
- Report consolidated results

## Project Overview

```
Local Development (macOS)
    ↓ rsync
IBM i (P7-deploy)
    ├── /home/ernestr/redis400/ (IFS source)
    ├── REDIS400 library (51 modules, 1 service program, 50 SQL functions)
    └── Redis server (127.0.0.1:6379)
```

### SQL Functions (50 total)

| Phase | Functions |
|-------|-----------|
| Core (Phase 1) | GET, SET, SETNX, INCR, DECR, DEL, EXISTS, EXPIRE, TTL, APPEND, AUTH, PING |
| Hash (Phase 2) | HSET, HGET, HDEL, HEXISTS, HGETALL |
| List (Phase 3) | LPUSH, RPUSH, LPOP, RPOP, LLEN, LRANGE |
| Set (Phase 4) | SADD, SREM, SISMEMBER, SCARD, SMEMBERS |
| Extended (Phase 5) | SETEX, INCRBY, DECRBY, PERSIST, TYPE, STRLEN |
| Key Scanning (Phase 6) | KEYS, SCAN |
| Sorted Set (Phase 7) | ZADD, ZREM, ZSCORE, ZRANK, ZCARD, ZRANGE, ZRANGEBYSCORE |
| Multi-key (Phase 8) | MGET, MSET, GETSET, RENAME |
| Hash/Set Scan (Phase 9) | HSCAN, SSCAN |
| Server (Phase 10) | DBSIZE |

## Critical Constraints

### IBM i 10-Character Object Name Limit

**IBM i system object names (modules, libraries, programs) are limited to 10 characters.**

This means source filenames in `srcfile/` must be ≤ 10 characters (excluding `.c`), because the filename becomes the IBM i module name.

- ✓ `redisapnd.c` → Module `REDISAPND` (9 chars)
- ✗ `redisappend.c` → Module `REDISAPPEND` (11 chars) — **BUILD WILL FAIL**

**Always verify filename length before creating a new source file.** Use abbreviations: `apnd` not `append`, `exp` not `expire`, `exst` not `exists`.

### SSH PATH Issue

When running commands on P7 via SSH, `/QOpenSys/pkgs/bin/` is not always in PATH (especially with `bsh` as default shell). Always prefix SSH commands with:

```bash
"/QOpenSys/pkgs/bin/bash -c 'export PATH=/QOpenSys/pkgs/bin:\$PATH && ...'"
```

### isql Runs Locally, Not on P7

The `isql` ODBC tool runs on the **local Mac**, not via SSH. The ODBC DSN `ISSI_P10` is configured locally. Build and deploy happen on P7 via SSH, but SQL testing uses local `isql`.

### ILE C INCDIR Bug

ILE C compiler `INCDIR` does **not** reliably resolve quoted `#include "file.h"` directives. Headers must be in the same directory as source files. The `generate_config.sh` script copies headers to `srcfile/` at build time.

## Build Modes: USE_ICONV

The project supports two EBCDIC/ASCII conversion modes controlled by `USE_ICONV` in `.env`:

| `.env` Setting | Mode | When to Use |
|----------------|------|-------------|
| `USE_ICONV=0` | Static translation tables (default) | English CCSID 37, best performance for small payloads |
| `USE_ICONV=1` | iconv via QtqIconvOpen | Non-English EBCDIC (CCSID 1025, 273, 297, etc.) |

To switch modes: edit `.env`, then `gmake clean; gmake all`. The makefile reads `.env` and automatically sets compiler/linker flags.

## Available Agents

Spawn agents using the Task tool with `subagent_type='general-purpose'` and include the skill path in the prompt.

### 1. IBM i C Developer Agent
**Skill**: `.claude/skills/ibmi-c-developer/SKILL.md`
**Expertise**: C programming, ILE/PASE, EBCDIC/ASCII, MI, shells, memory management, 10-char naming
**Use for**: Code changes, architecture decisions, debugging C issues

```
Prompt: "You are the IBM i C DEVELOPER agent. Read .claude/skills/ibmi-c-developer/SKILL.md first. TASK: [task description]"
```

### 2. Tester Agent
**Skill**: `.claude/skills/tester/SKILL.md`
**Expertise**: Build, deploy, and test on P7 (all 34 functions, both USE_ICONV modes)
**Use for**: Deploying code, running tests, verifying functionality

```
Prompt: "You are the TESTER agent. Read .claude/skills/tester/SKILL.md first. TASK: [task description]"
```

### 3. Redis Expert Agent
**Skill**: `.claude/skills/redis-expert/SKILL.md`
**Expertise**: Redis commands, protocols, authentication, RESP format
**Use for**: Redis protocol questions, new command implementation, auth setup

```
Prompt: "You are the REDIS EXPERT agent. Read .claude/skills/redis-expert/SKILL.md first. TASK: [task description]"
```

## Workflow Patterns

### Adding a New Redis Command

1. **Redis Expert**: Design the command (protocol format, parameters, returns)
2. **IBM i C Developer**: Implement the C function (filename ≤ 10 chars!)
3. **Tester**: Deploy and verify on P7 (both USE_ICONV modes)

### Debugging a Function

1. **Tester**: Reproduce the issue, gather error codes
2. **IBM i C Developer**: Analyze code, identify root cause
3. **Tester**: Verify fix

### Protocol/Encoding Issues

1. **Redis Expert**: Verify RESP format
2. **IBM i C Developer**: Check EBCDIC/ASCII conversion
3. **Tester**: Test with both USE_ICONV=0 and USE_ICONV=1

## Quick Commands

### Deploy and Build
```bash
sshpass -p 'ops_api' rsync -avz --delete \
  --exclude='.git' --exclude='.DS_Store' \
  -e "ssh -o StrictHostKeyChecking=no" \
  /Users/erozloznik/Documents/osobne_projekty/v3/ \
  P7-deploy:/home/ernestr/redis400/

sshpass -p 'ops_api' ssh P7-deploy \
  "/QOpenSys/pkgs/bin/bash -c 'export PATH=/QOpenSys/pkgs/bin:\$PATH && cd /home/ernestr/redis400 && gmake clean; gmake all'"
```

### Test All Functions
```bash
# Run the full automated test suite (49 tests)
bash test_all.sh

# Expected: Results: 49 passed, 0 failed (of 49 tests)
```

## Project Structure

```
/Users/erozloznik/Documents/osobne_projekty/v3/
├── srcfile/                    # C source files (filenames ≤ 10 chars!)
│   ├── redisget.c             # REDIS_GET
│   ├── redisset.c             # REDIS_SET
│   ├── redisincr.c            # REDIS_INCR
│   ├── redisdecr.c            # REDIS_DECR
│   ├── redisdel.c             # REDIS_DEL
│   ├── redisexp.c             # REDIS_EXPIRE
│   ├── redisttl.c             # REDIS_TTL
│   ├── redisping.c            # REDIS_PING
│   ├── redisapnd.c            # REDIS_APPEND
│   ├── redisexst.c            # REDIS_EXISTS
│   ├── redissnx.c             # REDIS_SETNX
│   ├── redisauth.c            # REDIS_AUTH
│   ├── redishset.c            # REDIS_HSET
│   ├── redishget.c            # REDIS_HGET
│   ├── redishdel.c            # REDIS_HDEL
│   ├── redishext.c            # REDIS_HEXISTS
│   ├── redishgta.c            # REDIS_HGETALL
│   ├── redislpsh.c            # REDIS_LPUSH
│   ├── redisrpsh.c            # REDIS_RPUSH
│   ├── redislpop.c            # REDIS_LPOP
│   ├── redisrpop.c            # REDIS_RPOP
│   ├── redisllen.c            # REDIS_LLEN
│   ├── redislrng.c            # REDIS_LRANGE
│   ├── redissadd.c            # REDIS_SADD
│   ├── redissrem.c            # REDIS_SREM
│   ├── redissism.c            # REDIS_SISMEMBER
│   ├── redisscrd.c            # REDIS_SCARD
│   ├── redissmem.c            # REDIS_SMEMBERS
│   ├── redissetx.c            # REDIS_SETEX
│   ├── redisincb.c            # REDIS_INCRBY
│   ├── redisdecb.c            # REDIS_DECRBY
│   ├── redispst.c             # REDIS_PERSIST
│   ├── redistype.c            # REDIS_TYPE
│   ├── redisslen.c            # REDIS_STRLEN
│   ├── rediszadd.c            # REDIS_ZADD
│   ├── rediszrem.c            # REDIS_ZREM
│   ├── rediszsco.c            # REDIS_ZSCORE
│   ├── rediszrnk.c            # REDIS_ZRANK
│   ├── rediszcrd.c            # REDIS_ZCARD
│   ├── rediszrng.c            # REDIS_ZRANGE
│   ├── rediszrbs.c            # REDIS_ZRANGEBYSCORE
│   ├── redismget.c            # REDIS_MGET
│   ├── redismset.c            # REDIS_MSET
│   ├── redisgset.c            # REDIS_GETSET
│   ├── redisrnme.c            # REDIS_RENAME
│   ├── redishscn.c            # REDIS_HSCAN
│   ├── redissscn.c            # REDIS_SSCAN
│   ├── redisdbsz.c            # REDIS_DBSIZE
│   ├── redisbench.c           # Benchmark: table vs iconv conversion
│   └── redisutils.c           # Shared utilities
├── include/
│   ├── redis_utils.h
│   └── redis_config.h          # Generated from .env (DO NOT EDIT)
├── qsrvsrc/
│   └── redisile.bnd
├── .claude/
│   ├── settings.json
│   ├── skills/
│   │   ├── orchestrator/       # This agent
│   │   ├── ibmi-c-developer/   # C programming expert
│   │   ├── tester/             # Deploy and test
│   │   └── redis-expert/       # Redis protocol expert
│   └── commands/
│       ├── deploy.md
│       └── test.md
├── .env                        # Redis + build config (IP, port, USE_ICONV)
├── generate_config.sh          # Generates redis_config.h, copies headers to srcfile/
├── test_all.sh                 # Automated test suite (49 tests via ODBC)
├── makefile
├── CLAUDE.md
├── CHANGELOG.md
└── README.md
```

## Decision Making

When multiple approaches exist, consult the appropriate expert:
- **Performance vs Maintainability**: Ask IBM i C Developer
- **Redis best practices**: Ask Redis Expert
- **Test coverage**: Ask Tester

Always report back to the user with:
1. What was done
2. Results/output
3. Any issues encountered
4. Recommendations for next steps
