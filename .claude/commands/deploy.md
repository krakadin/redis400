# /deploy

Deploy the Redis400 project to IBM i (P7) and build.

## Steps

1. Sync files to P7:
```bash
sshpass -p 'ops_api' rsync -avz --delete \
  --exclude='.git' --exclude='.DS_Store' \
  -e "ssh -o StrictHostKeyChecking=no" \
  /Users/erozloznik/Documents/osobne_projekty/v3/ \
  P7-deploy:/home/ernestr/redis400/
```

2. Build on P7:
```bash
sshpass -p 'ops_api' ssh -o StrictHostKeyChecking=no P7-deploy \
  "/QOpenSys/pkgs/bin/bash -c 'export PATH=/QOpenSys/pkgs/bin:\$PATH && cd /home/ernestr/redis400 && gmake clean; gmake all'"
```

3. Report build status:
   - List any compilation errors
   - Confirm service program creation
   - Confirm SQL function registration

## Expected Output

- 9 modules compiled (REDISGET, REDISSET, REDISINCR, REDISDEL, REDISEXP, REDISTTL, REDISPING, REDISAPND, REDISUTILS)
- Service program REDISILE created
- 8 SQL functions registered (REDIS_GET, REDIS_SET, REDIS_INCR, REDIS_DEL, REDIS_EXPIRE, REDIS_TTL, REDIS_PING, REDIS_APPEND)
- IOT/Abort trap messages during `clean` phase are normal (library didn't exist yet)

## Notes

- `gmake` must be invoked with full PATH: `export PATH=/QOpenSys/pkgs/bin:$PATH`
- IBM i module names are limited to 10 characters — source filenames must respect this
