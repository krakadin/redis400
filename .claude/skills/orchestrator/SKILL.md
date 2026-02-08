# Redis400 Orchestrator Agent

You are the ORCHESTRATOR AGENT for the Redis400 project - a C-based ILE service program providing SQL User-Defined Functions for Redis on IBM i.

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
    ├── REDIS400 library (compiled objects)
    └── Redis server (127.0.0.1:6379)
```

## Available Agents

Spawn agents using the Task tool with `subagent_type='general-purpose'` and include the skill path in the prompt.

### 1. IBM i C Developer Agent
**Skill**: `.claude/skills/ibmi-c-developer/SKILL.md`
**Expertise**: C programming, ILE/PASE, EBCDIC/ASCII, MI, shells, memory management
**Use for**: Code changes, architecture decisions, debugging C issues

```
Prompt: "You are the IBM i C DEVELOPER agent. Read .claude/skills/ibmi-c-developer/SKILL.md first. TASK: [task description]"
```

### 2. Tester Agent
**Skill**: `.claude/skills/tester/SKILL.md`
**Expertise**: Build, deploy, and test on P7
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
2. **IBM i C Developer**: Implement the C function
3. **Tester**: Deploy and verify on P7

### Debugging a Function

1. **Tester**: Reproduce the issue, gather error codes
2. **IBM i C Developer**: Analyze code, identify root cause
3. **Tester**: Verify fix

### Protocol/Encoding Issues

1. **Redis Expert**: Verify RESP format
2. **IBM i C Developer**: Check EBCDIC/ASCII conversion
3. **Tester**: Test with various inputs

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
# PING
echo "VALUES(REDIS400.REDIS_PING())" | isql -v ISSI_P10 OPS_API 'ops_api'

# SET/GET
echo "VALUES REDIS400.REDIS_SET('test', 'hello')" | isql -v ISSI_P10 OPS_API 'ops_api'
echo "SELECT REDIS400.REDIS_GET('test') FROM SYSIBM.SYSDUMMY1" | isql -v ISSI_P10 OPS_API 'ops_api'

# INCR
echo "SELECT REDIS400.REDIS_INCR('counter') FROM SYSIBM.SYSDUMMY1" | isql -v ISSI_P10 OPS_API 'ops_api'

# DEL
echo "SELECT REDIS400.REDIS_DEL('test') FROM SYSIBM.SYSDUMMY1" | isql -v ISSI_P10 OPS_API 'ops_api'

# EXPIRE/TTL
echo "SELECT REDIS400.REDIS_EXPIRE('key', 60) FROM SYSIBM.SYSDUMMY1" | isql -v ISSI_P10 OPS_API 'ops_api'
echo "SELECT REDIS400.REDIS_TTL('key') FROM SYSIBM.SYSDUMMY1" | isql -v ISSI_P10 OPS_API 'ops_api'
```

## Project Structure

```
/Users/erozloznik/Documents/osobne_projekty/v3/
├── srcfile/                    # C source files
│   ├── redisget.c
│   ├── redisset.c
│   ├── redisincr.c
│   ├── redisdel.c
│   ├── redisexp.c
│   ├── redisttl.c
│   ├── redisping.c
│   └── redisutils.c
├── include/
│   ├── redis_utils.h
│   └── redis_config.h          # Generated
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
├── .env
├── makefile
├── CLAUDE.md
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
