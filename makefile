# Configuration
SRC_DIR = srcfile
BND_DIR = qsrvsrc
TGT_LIB = REDIS400
DBG_VIEW = *NONE
CCSID = 1252 # Windows, Latin 1

# Read .env for USE_ICONV setting (0=translation tables, 1=iconv)
include .env
ifeq ($(USE_ICONV),1)
DEFINE_FLAG = DEFINE(USE_ICONV)
ICONV_BIND = BNDSRVPGM(QSYS/QTQICONV)
else
DEFINE_FLAG =
ICONV_BIND =
endif

# Targets
all: preflight $(TGT_LIB).lib generate_config redisile.srvpgm redis_get.func redis_set.func \
	redis_incr.func redis_del.func redis_expire.func redis_ttl.func redis_ping.func redis_append.func \
	redis_exists.func redis_setnx.func redis_decr.func redis_auth.func \
	redis_hset.func redis_hget.func redis_hdel.func redis_hexists.func redis_hgetall.func \
	redis_lpush.func redis_rpush.func redis_lpop.func redis_rpop.func redis_llen.func redis_lrange.func \
	redis_sadd.func redis_srem.func redis_sismember.func redis_scard.func redis_smembers.func \
	redis_setex.func redis_incrby.func redis_decrby.func redis_persist.func redis_type.func redis_strlen.func \
	redis_keys.func redis_scan.func \
	redis_zadd.func redis_zrem.func redis_zscore.func redis_zrank.func redis_zcard.func redis_zrange.func redis_zrangebyscore.func \
	redis_mget.func redis_mset.func redis_getset.func redis_rename.func \
	redis_hscan.func redis_sscan.func \
	redis_dbsize.func

redisile.srvpgm: redisget.cle redisset.cle redisincr.cle redisdel.cle redisexp.cle redisttl.cle \
	redisping.cle redisapnd.cle redisexst.cle redissnx.cle redisdecr.cle redisauth.cle \
	redishset.cle redishget.cle redishdel.cle redishext.cle redishgta.cle \
	redislpsh.cle redisrpsh.cle redislpop.cle redisrpop.cle redisllen.cle redislrng.cle \
	redissadd.cle redissrem.cle redissism.cle redisscrd.cle redissmem.cle \
	redissetx.cle redisincb.cle redisdecb.cle redispst.cle redistype.cle redisslen.cle \
	rediskeys.cle redisscan.cle \
	rediszadd.cle rediszrem.cle rediszsco.cle rediszrnk.cle rediszcrd.cle rediszrng.cle rediszrbs.cle \
	redismget.cle redismset.cle redisgset.cle redisrnme.cle \
	redishscn.cle redissscn.cle \
	redisdbsz.cle \
	redisutils.cle
redisget.cle: redisget.cmodule redisget.bnd
redisset.cle: redisset.cmodule redisset.bnd
redisincr.cle: redisincr.cmodule redisincr.bnd
redisdel.cle: redisdel.cmodule redisdel.bnd
redisexp.cle: redisexp.cmodule redisexp.bnd
redisttl.cle: redisttl.cmodule redisttl.bnd
redisping.cle: redisping.cmodule redisping.bnd
redisapnd.cle: redisapnd.cmodule redisapnd.bnd
redisexst.cle: redisexst.cmodule redisexst.bnd
redissnx.cle: redissnx.cmodule redissnx.bnd
redisdecr.cle: redisdecr.cmodule redisdecr.bnd
redisauth.cle: redisauth.cmodule redisauth.bnd
redishset.cle: redishset.cmodule redishset.bnd
redishget.cle: redishget.cmodule redishget.bnd
redishdel.cle: redishdel.cmodule redishdel.bnd
redishext.cle: redishext.cmodule redishext.bnd
redishgta.cle: redishgta.cmodule redishgta.bnd
redislpsh.cle: redislpsh.cmodule redislpsh.bnd
redisrpsh.cle: redisrpsh.cmodule redisrpsh.bnd
redislpop.cle: redislpop.cmodule redislpop.bnd
redisrpop.cle: redisrpop.cmodule redisrpop.bnd
redisllen.cle: redisllen.cmodule redisllen.bnd
redislrng.cle: redislrng.cmodule redislrng.bnd
redissadd.cle: redissadd.cmodule redissadd.bnd
redissrem.cle: redissrem.cmodule redissrem.bnd
redissism.cle: redissism.cmodule redissism.bnd
redisscrd.cle: redisscrd.cmodule redisscrd.bnd
redissmem.cle: redissmem.cmodule redissmem.bnd
redissetx.cle: redissetx.cmodule redissetx.bnd
redisincb.cle: redisincb.cmodule redisincb.bnd
redisdecb.cle: redisdecb.cmodule redisdecb.bnd
redispst.cle: redispst.cmodule redispst.bnd
redistype.cle: redistype.cmodule redistype.bnd
redisslen.cle: redisslen.cmodule redisslen.bnd
rediskeys.cle: rediskeys.cmodule rediskeys.bnd
redisscan.cle: redisscan.cmodule redisscan.bnd
rediszadd.cle: rediszadd.cmodule rediszadd.bnd
rediszrem.cle: rediszrem.cmodule rediszrem.bnd
rediszsco.cle: rediszsco.cmodule rediszsco.bnd
rediszrnk.cle: rediszrnk.cmodule rediszrnk.bnd
rediszcrd.cle: rediszcrd.cmodule rediszcrd.bnd
rediszrng.cle: rediszrng.cmodule rediszrng.bnd
rediszrbs.cle: rediszrbs.cmodule rediszrbs.bnd
redismget.cle: redismget.cmodule redismget.bnd
redismset.cle: redismset.cmodule redismset.bnd
redisgset.cle: redisgset.cmodule redisgset.bnd
redisrnme.cle: redisrnme.cmodule redisrnme.bnd
redishscn.cle: redishscn.cmodule redishscn.bnd
redissscn.cle: redissscn.cmodule redissscn.bnd
redisdbsz.cle: redisdbsz.cmodule redisdbsz.bnd
redisutils.cle: redisutils.cmodule redisutils.bnd

# Preflight check to ensure the target library does not exist
preflight:
	@if system -s "CHKOBJ OBJ($(TGT_LIB)) OBJTYPE(*LIB)"; then \
		echo "Error: Library $(TGT_LIB) already exists. Please delete it with 'DLTLIB LIB($(TGT_LIB))' and retry."; \
		exit 1; \
	else \
		echo "No existing $(TGT_LIB) library found - proceeding."; \
		exit 0; \
	fi

# Generate config header from .env before compiling
generate_config:
	-/QOpenSys/pkgs/bin/bash generate_config.sh

# Create the target library
%.lib:
	-system -q "CRTLIB $*"

# Create binding directory and add binding source
%.cmodule: srcfile/%.c
	-system -qi "CRTSRCPF FILE($(TGT_LIB)/QCSRC) RCDLEN(132)"
	system "CRTCMOD MODULE($(TGT_LIB)/$*) SRCSTMF('$<') SYSIFCOPT(*IFSIO *IFS64IO) DBGVIEW($(DBG_VIEW)) $(DEFINE_FLAG)"
	@touch $*.cle

# Create binding directory and add binding source
%.bnd: 
	-system -qi "RTVBNDSRC MODULE($(TGT_LIB)/$*) SRCFILE($(TGT_LIB)/QSRVSRC) MBROPT(*REPLACE)"

# Create service programs
%.srvpgm: $(BND_DIR)/%.bnd
	$(eval modules := $(patsubst %,$(TGT_LIB)/%,$(basename $(filter %.cle,$(notdir $^)))))
	system "CRTSRVPGM SRVPGM($(TGT_LIB)/$*) MODULE($(modules)) SRCSTMF('$(BND_DIR)/$*.bnd') $(ICONV_BIND) OPTION(*DUPPROC *DUPVAR) EXPORT(*SRCFILE) ACTGRP(*CALLER)"

# Create or replace the SQL function for redis_get
redis_get.func:
	-system "RUNSQL SQL('CREATE OR REPLACE FUNCTION $(TGT_LIB).REDIS_GET (KEY VARCHAR(255)) RETURNS VARCHAR(16370) LANGUAGE C SPECIFIC REDIS_GET NOT DETERMINISTIC NO SQL RETURNS NULL ON NULL INPUT DISALLOW PARALLEL NOT FENCED EXTERNAL NAME ''$(TGT_LIB)/REDISILE(getRedisValue)'' PARAMETER STYLE DB2SQL') COMMIT(*NONE)"

# Create or replace the SQL function for redis_set
redis_set.func:
	-system "RUNSQL SQL('CREATE OR REPLACE FUNCTION $(TGT_LIB).REDIS_SET (KEY VARCHAR(255), VALUE VARCHAR(16370)) RETURNS VARCHAR(128) LANGUAGE C SPECIFIC REDIS_SET NOT DETERMINISTIC NO SQL RETURNS NULL ON NULL INPUT DISALLOW PARALLEL NOT FENCED EXTERNAL NAME ''$(TGT_LIB)/REDISILE(setRedisValue)'' PARAMETER STYLE DB2SQL') COMMIT(*NONE)"

# Create or replace the SQL function for redis_incr
redis_incr.func:
	-system "RUNSQL SQL('CREATE OR REPLACE FUNCTION $(TGT_LIB).REDIS_INCR (KEY VARCHAR(255)) RETURNS BIGINT LANGUAGE C SPECIFIC REDIS_INCR NOT DETERMINISTIC NO SQL RETURNS NULL ON NULL INPUT DISALLOW PARALLEL NOT FENCED EXTERNAL NAME ''$(TGT_LIB)/REDISILE(incrRedisValue)'' PARAMETER STYLE DB2SQL') COMMIT(*NONE)"

# Create or replace the SQL function for redis_del
redis_del.func:
	-system "RUNSQL SQL('CREATE OR REPLACE FUNCTION $(TGT_LIB).REDIS_DEL (KEY VARCHAR(255)) RETURNS SMALLINT LANGUAGE C SPECIFIC REDIS_DEL NOT DETERMINISTIC NO SQL RETURNS NULL ON NULL INPUT DISALLOW PARALLEL NOT FENCED EXTERNAL NAME ''$(TGT_LIB)/REDISILE(delRedisKey)'' PARAMETER STYLE DB2SQL') COMMIT(*NONE)"

# Create or replace the SQL function for redis_exp
redis_expire.func:
	-system "RUNSQL SQL('CREATE OR REPLACE FUNCTION $(TGT_LIB).REDIS_EXPIRE (KEY VARCHAR(255), TTL INTEGER) RETURNS SMALLINT LANGUAGE C SPECIFIC REDIS_EXPIRE NOT DETERMINISTIC NO SQL RETURNS NULL ON NULL INPUT DISALLOW PARALLEL NOT FENCED EXTERNAL NAME ''$(TGT_LIB)/REDISILE(expireRedisKey)'' PARAMETER STYLE DB2SQL') COMMIT(*NONE)"

# Create or replace the SQL function for REDIS_TTL
redis_ttl.func: redisile.srvpgm
	-system "RUNSQL SQL('CREATE OR REPLACE FUNCTION $(TGT_LIB).REDIS_TTL (KEY VARCHAR(255)) RETURNS INTEGER LANGUAGE C SPECIFIC REDIS_TTL NOT DETERMINISTIC NO SQL RETURNS NULL ON NULL INPUT DISALLOW PARALLEL NOT FENCED EXTERNAL NAME ''$(TGT_LIB)/REDISILE(ttlRedisKey)'' PARAMETER STYLE DB2SQL') COMMIT(*NONE)"

# Create or replace the SQL function for redis_append
redis_append.func:
	-system "RUNSQL SQL('CREATE OR REPLACE FUNCTION $(TGT_LIB).REDIS_APPEND (KEY VARCHAR(255), VALUE VARCHAR(16370)) RETURNS BIGINT LANGUAGE C SPECIFIC REDIS_APPEND NOT DETERMINISTIC NO SQL RETURNS NULL ON NULL INPUT DISALLOW PARALLEL NOT FENCED EXTERNAL NAME ''$(TGT_LIB)/REDISILE(appendRedisValue)'' PARAMETER STYLE DB2SQL') COMMIT(*NONE)"

# Create or replace the SQL function for redis_exists
redis_exists.func:
	-system "RUNSQL SQL('CREATE OR REPLACE FUNCTION $(TGT_LIB).REDIS_EXISTS (KEY VARCHAR(255)) RETURNS BIGINT LANGUAGE C SPECIFIC REDIS_EXISTS NOT DETERMINISTIC NO SQL RETURNS NULL ON NULL INPUT DISALLOW PARALLEL NOT FENCED EXTERNAL NAME ''$(TGT_LIB)/REDISILE(existsRedisKey)'' PARAMETER STYLE DB2SQL') COMMIT(*NONE)"

# Create or replace the SQL function for redis_setnx
redis_setnx.func:
	-system "RUNSQL SQL('CREATE OR REPLACE FUNCTION $(TGT_LIB).REDIS_SETNX (KEY VARCHAR(255), VALUE VARCHAR(16370)) RETURNS BIGINT LANGUAGE C SPECIFIC REDIS_SETNX NOT DETERMINISTIC NO SQL RETURNS NULL ON NULL INPUT DISALLOW PARALLEL NOT FENCED EXTERNAL NAME ''$(TGT_LIB)/REDISILE(setnxRedisValue)'' PARAMETER STYLE DB2SQL') COMMIT(*NONE)"

# Create or replace the SQL function for redis_decr
redis_decr.func:
	-system "RUNSQL SQL('CREATE OR REPLACE FUNCTION $(TGT_LIB).REDIS_DECR (KEY VARCHAR(255)) RETURNS BIGINT LANGUAGE C SPECIFIC REDIS_DECR NOT DETERMINISTIC NO SQL RETURNS NULL ON NULL INPUT DISALLOW PARALLEL NOT FENCED EXTERNAL NAME ''$(TGT_LIB)/REDISILE(decrRedisValue)'' PARAMETER STYLE DB2SQL') COMMIT(*NONE)"

# Create or replace the SQL function for redis_auth
redis_auth.func:
	-system "RUNSQL SQL('CREATE OR REPLACE FUNCTION $(TGT_LIB).REDIS_AUTH (PASSWORD VARCHAR(255)) RETURNS VARCHAR(128) LANGUAGE C SPECIFIC REDIS_AUTH NOT DETERMINISTIC NO SQL RETURNS NULL ON NULL INPUT DISALLOW PARALLEL NOT FENCED EXTERNAL NAME ''$(TGT_LIB)/REDISILE(authRedis)'' PARAMETER STYLE DB2SQL') COMMIT(*NONE)"

# Create or replace the SQL function for redis_hset
redis_hset.func:
	-system "RUNSQL SQL('CREATE OR REPLACE FUNCTION $(TGT_LIB).REDIS_HSET (KEY VARCHAR(255), FIELD VARCHAR(255), VALUE VARCHAR(16370)) RETURNS BIGINT LANGUAGE C SPECIFIC REDIS_HSET NOT DETERMINISTIC NO SQL RETURNS NULL ON NULL INPUT DISALLOW PARALLEL NOT FENCED EXTERNAL NAME ''$(TGT_LIB)/REDISILE(hsetRedisValue)'' PARAMETER STYLE DB2SQL') COMMIT(*NONE)"

# Create or replace the SQL function for redis_hget
redis_hget.func:
	-system "RUNSQL SQL('CREATE OR REPLACE FUNCTION $(TGT_LIB).REDIS_HGET (KEY VARCHAR(255), FIELD VARCHAR(255)) RETURNS VARCHAR(16370) LANGUAGE C SPECIFIC REDIS_HGET NOT DETERMINISTIC NO SQL RETURNS NULL ON NULL INPUT DISALLOW PARALLEL NOT FENCED EXTERNAL NAME ''$(TGT_LIB)/REDISILE(hgetRedisValue)'' PARAMETER STYLE DB2SQL') COMMIT(*NONE)"

# Create or replace the SQL function for redis_hdel
redis_hdel.func:
	-system "RUNSQL SQL('CREATE OR REPLACE FUNCTION $(TGT_LIB).REDIS_HDEL (KEY VARCHAR(255), FIELD VARCHAR(255)) RETURNS BIGINT LANGUAGE C SPECIFIC REDIS_HDEL NOT DETERMINISTIC NO SQL RETURNS NULL ON NULL INPUT DISALLOW PARALLEL NOT FENCED EXTERNAL NAME ''$(TGT_LIB)/REDISILE(hdelRedisField)'' PARAMETER STYLE DB2SQL') COMMIT(*NONE)"

# Create or replace the SQL function for redis_hexists
redis_hexists.func:
	-system "RUNSQL SQL('CREATE OR REPLACE FUNCTION $(TGT_LIB).REDIS_HEXISTS (KEY VARCHAR(255), FIELD VARCHAR(255)) RETURNS BIGINT LANGUAGE C SPECIFIC REDIS_HEXISTS NOT DETERMINISTIC NO SQL RETURNS NULL ON NULL INPUT DISALLOW PARALLEL NOT FENCED EXTERNAL NAME ''$(TGT_LIB)/REDISILE(hexistsRedisField)'' PARAMETER STYLE DB2SQL') COMMIT(*NONE)"

# Create or replace the SQL function for redis_hgetall
redis_hgetall.func:
	-system "RUNSQL SQL('CREATE OR REPLACE FUNCTION $(TGT_LIB).REDIS_HGETALL (KEY VARCHAR(255)) RETURNS VARCHAR(16370) LANGUAGE C SPECIFIC REDIS_HGETALL NOT DETERMINISTIC NO SQL RETURNS NULL ON NULL INPUT DISALLOW PARALLEL NOT FENCED EXTERNAL NAME ''$(TGT_LIB)/REDISILE(hgetallRedis)'' PARAMETER STYLE DB2SQL') COMMIT(*NONE)"

# Create or replace the SQL function for redis_lpush
redis_lpush.func:
	-system "RUNSQL SQL('CREATE OR REPLACE FUNCTION $(TGT_LIB).REDIS_LPUSH (KEY VARCHAR(255), VALUE VARCHAR(16370)) RETURNS BIGINT LANGUAGE C SPECIFIC REDIS_LPUSH NOT DETERMINISTIC NO SQL RETURNS NULL ON NULL INPUT DISALLOW PARALLEL NOT FENCED EXTERNAL NAME ''$(TGT_LIB)/REDISILE(lpushRedisList)'' PARAMETER STYLE DB2SQL') COMMIT(*NONE)"

# Create or replace the SQL function for redis_rpush
redis_rpush.func:
	-system "RUNSQL SQL('CREATE OR REPLACE FUNCTION $(TGT_LIB).REDIS_RPUSH (KEY VARCHAR(255), VALUE VARCHAR(16370)) RETURNS BIGINT LANGUAGE C SPECIFIC REDIS_RPUSH NOT DETERMINISTIC NO SQL RETURNS NULL ON NULL INPUT DISALLOW PARALLEL NOT FENCED EXTERNAL NAME ''$(TGT_LIB)/REDISILE(rpushRedisList)'' PARAMETER STYLE DB2SQL') COMMIT(*NONE)"

# Create or replace the SQL function for redis_lpop
redis_lpop.func:
	-system "RUNSQL SQL('CREATE OR REPLACE FUNCTION $(TGT_LIB).REDIS_LPOP (KEY VARCHAR(255)) RETURNS VARCHAR(16370) LANGUAGE C SPECIFIC REDIS_LPOP NOT DETERMINISTIC NO SQL RETURNS NULL ON NULL INPUT DISALLOW PARALLEL NOT FENCED EXTERNAL NAME ''$(TGT_LIB)/REDISILE(lpopRedisList)'' PARAMETER STYLE DB2SQL') COMMIT(*NONE)"

# Create or replace the SQL function for redis_rpop
redis_rpop.func:
	-system "RUNSQL SQL('CREATE OR REPLACE FUNCTION $(TGT_LIB).REDIS_RPOP (KEY VARCHAR(255)) RETURNS VARCHAR(16370) LANGUAGE C SPECIFIC REDIS_RPOP NOT DETERMINISTIC NO SQL RETURNS NULL ON NULL INPUT DISALLOW PARALLEL NOT FENCED EXTERNAL NAME ''$(TGT_LIB)/REDISILE(rpopRedisList)'' PARAMETER STYLE DB2SQL') COMMIT(*NONE)"

# Create or replace the SQL function for redis_llen
redis_llen.func:
	-system "RUNSQL SQL('CREATE OR REPLACE FUNCTION $(TGT_LIB).REDIS_LLEN (KEY VARCHAR(255)) RETURNS BIGINT LANGUAGE C SPECIFIC REDIS_LLEN NOT DETERMINISTIC NO SQL RETURNS NULL ON NULL INPUT DISALLOW PARALLEL NOT FENCED EXTERNAL NAME ''$(TGT_LIB)/REDISILE(llenRedisList)'' PARAMETER STYLE DB2SQL') COMMIT(*NONE)"

# Create or replace the SQL function for redis_lrange
redis_lrange.func:
	-system "RUNSQL SQL('CREATE OR REPLACE FUNCTION $(TGT_LIB).REDIS_LRANGE (KEY VARCHAR(255), START_IDX INTEGER, STOP_IDX INTEGER) RETURNS VARCHAR(16370) LANGUAGE C SPECIFIC REDIS_LRANGE NOT DETERMINISTIC NO SQL RETURNS NULL ON NULL INPUT DISALLOW PARALLEL NOT FENCED EXTERNAL NAME ''$(TGT_LIB)/REDISILE(lrangeRedisList)'' PARAMETER STYLE DB2SQL') COMMIT(*NONE)"

# Create or replace the SQL function for redis_sadd
redis_sadd.func:
	-system "RUNSQL SQL('CREATE OR REPLACE FUNCTION $(TGT_LIB).REDIS_SADD (KEY VARCHAR(255), MEMBER VARCHAR(16370)) RETURNS BIGINT LANGUAGE C SPECIFIC REDIS_SADD NOT DETERMINISTIC NO SQL RETURNS NULL ON NULL INPUT DISALLOW PARALLEL NOT FENCED EXTERNAL NAME ''$(TGT_LIB)/REDISILE(saddRedisSet)'' PARAMETER STYLE DB2SQL') COMMIT(*NONE)"

# Create or replace the SQL function for redis_srem
redis_srem.func:
	-system "RUNSQL SQL('CREATE OR REPLACE FUNCTION $(TGT_LIB).REDIS_SREM (KEY VARCHAR(255), MEMBER VARCHAR(16370)) RETURNS BIGINT LANGUAGE C SPECIFIC REDIS_SREM NOT DETERMINISTIC NO SQL RETURNS NULL ON NULL INPUT DISALLOW PARALLEL NOT FENCED EXTERNAL NAME ''$(TGT_LIB)/REDISILE(sremRedisSet)'' PARAMETER STYLE DB2SQL') COMMIT(*NONE)"

# Create or replace the SQL function for redis_sismember
redis_sismember.func:
	-system "RUNSQL SQL('CREATE OR REPLACE FUNCTION $(TGT_LIB).REDIS_SISMEMBER (KEY VARCHAR(255), MEMBER VARCHAR(16370)) RETURNS BIGINT LANGUAGE C SPECIFIC REDIS_SISMEMBER NOT DETERMINISTIC NO SQL RETURNS NULL ON NULL INPUT DISALLOW PARALLEL NOT FENCED EXTERNAL NAME ''$(TGT_LIB)/REDISILE(sismemberRedisSet)'' PARAMETER STYLE DB2SQL') COMMIT(*NONE)"

# Create or replace the SQL function for redis_scard
redis_scard.func:
	-system "RUNSQL SQL('CREATE OR REPLACE FUNCTION $(TGT_LIB).REDIS_SCARD (KEY VARCHAR(255)) RETURNS BIGINT LANGUAGE C SPECIFIC REDIS_SCARD NOT DETERMINISTIC NO SQL RETURNS NULL ON NULL INPUT DISALLOW PARALLEL NOT FENCED EXTERNAL NAME ''$(TGT_LIB)/REDISILE(scardRedisSet)'' PARAMETER STYLE DB2SQL') COMMIT(*NONE)"

# Create or replace the SQL function for redis_smembers
redis_smembers.func:
	-system "RUNSQL SQL('CREATE OR REPLACE FUNCTION $(TGT_LIB).REDIS_SMEMBERS (KEY VARCHAR(255)) RETURNS VARCHAR(16370) LANGUAGE C SPECIFIC REDIS_SMEMBERS NOT DETERMINISTIC NO SQL RETURNS NULL ON NULL INPUT DISALLOW PARALLEL NOT FENCED EXTERNAL NAME ''$(TGT_LIB)/REDISILE(smembersRedisSet)'' PARAMETER STYLE DB2SQL') COMMIT(*NONE)"

# Create or replace the SQL function for redis_setex
redis_setex.func:
	-system "RUNSQL SQL('CREATE OR REPLACE FUNCTION $(TGT_LIB).REDIS_SETEX (KEY VARCHAR(255), TTL INTEGER, VALUE VARCHAR(16370)) RETURNS VARCHAR(128) LANGUAGE C SPECIFIC REDIS_SETEX NOT DETERMINISTIC NO SQL RETURNS NULL ON NULL INPUT DISALLOW PARALLEL NOT FENCED EXTERNAL NAME ''$(TGT_LIB)/REDISILE(setexRedisKey)'' PARAMETER STYLE DB2SQL') COMMIT(*NONE)"

# Create or replace the SQL function for redis_incrby
redis_incrby.func:
	-system "RUNSQL SQL('CREATE OR REPLACE FUNCTION $(TGT_LIB).REDIS_INCRBY (KEY VARCHAR(255), INCREMENT BIGINT) RETURNS BIGINT LANGUAGE C SPECIFIC REDIS_INCRBY NOT DETERMINISTIC NO SQL RETURNS NULL ON NULL INPUT DISALLOW PARALLEL NOT FENCED EXTERNAL NAME ''$(TGT_LIB)/REDISILE(incrbyRedisValue)'' PARAMETER STYLE DB2SQL') COMMIT(*NONE)"

# Create or replace the SQL function for redis_decrby
redis_decrby.func:
	-system "RUNSQL SQL('CREATE OR REPLACE FUNCTION $(TGT_LIB).REDIS_DECRBY (KEY VARCHAR(255), DECREMENT BIGINT) RETURNS BIGINT LANGUAGE C SPECIFIC REDIS_DECRBY NOT DETERMINISTIC NO SQL RETURNS NULL ON NULL INPUT DISALLOW PARALLEL NOT FENCED EXTERNAL NAME ''$(TGT_LIB)/REDISILE(decrbyRedisValue)'' PARAMETER STYLE DB2SQL') COMMIT(*NONE)"

# Create or replace the SQL function for redis_persist
redis_persist.func:
	-system "RUNSQL SQL('CREATE OR REPLACE FUNCTION $(TGT_LIB).REDIS_PERSIST (KEY VARCHAR(255)) RETURNS BIGINT LANGUAGE C SPECIFIC REDIS_PERSIST NOT DETERMINISTIC NO SQL RETURNS NULL ON NULL INPUT DISALLOW PARALLEL NOT FENCED EXTERNAL NAME ''$(TGT_LIB)/REDISILE(persistRedisKey)'' PARAMETER STYLE DB2SQL') COMMIT(*NONE)"

# Create or replace the SQL function for redis_type
redis_type.func:
	-system "RUNSQL SQL('CREATE OR REPLACE FUNCTION $(TGT_LIB).REDIS_TYPE (KEY VARCHAR(255)) RETURNS VARCHAR(20) LANGUAGE C SPECIFIC REDIS_TYPE NOT DETERMINISTIC NO SQL RETURNS NULL ON NULL INPUT DISALLOW PARALLEL NOT FENCED EXTERNAL NAME ''$(TGT_LIB)/REDISILE(typeRedisKey)'' PARAMETER STYLE DB2SQL') COMMIT(*NONE)"

# Create or replace the SQL function for redis_strlen
redis_strlen.func:
	-system "RUNSQL SQL('CREATE OR REPLACE FUNCTION $(TGT_LIB).REDIS_STRLEN (KEY VARCHAR(255)) RETURNS BIGINT LANGUAGE C SPECIFIC REDIS_STRLEN NOT DETERMINISTIC NO SQL RETURNS NULL ON NULL INPUT DISALLOW PARALLEL NOT FENCED EXTERNAL NAME ''$(TGT_LIB)/REDISILE(strlenRedisKey)'' PARAMETER STYLE DB2SQL') COMMIT(*NONE)"

# Create or replace the SQL function for redis_keys
redis_keys.func:
	-system "RUNSQL SQL('CREATE OR REPLACE FUNCTION $(TGT_LIB).REDIS_KEYS (PATTERN VARCHAR(255)) RETURNS VARCHAR(16370) LANGUAGE C SPECIFIC REDIS_KEYS NOT DETERMINISTIC NO SQL RETURNS NULL ON NULL INPUT DISALLOW PARALLEL NOT FENCED EXTERNAL NAME ''$(TGT_LIB)/REDISILE(keysRedisPattern)'' PARAMETER STYLE DB2SQL') COMMIT(*NONE)"

# Create or replace the SQL function for redis_scan
redis_scan.func:
	-system "RUNSQL SQL('CREATE OR REPLACE FUNCTION $(TGT_LIB).REDIS_SCAN (CURSOR VARCHAR(20), PATTERN VARCHAR(255), CNT INTEGER) RETURNS VARCHAR(16370) LANGUAGE C SPECIFIC REDIS_SCAN NOT DETERMINISTIC NO SQL RETURNS NULL ON NULL INPUT DISALLOW PARALLEL NOT FENCED EXTERNAL NAME ''$(TGT_LIB)/REDISILE(scanRedisKeys)'' PARAMETER STYLE DB2SQL') COMMIT(*NONE)"

# --- Phase 7: Sorted Set Operations ---

# Create or replace the SQL function for redis_zadd
redis_zadd.func:
	-system "RUNSQL SQL('CREATE OR REPLACE FUNCTION $(TGT_LIB).REDIS_ZADD (KEY VARCHAR(255), SCORE DOUBLE, MEMBER VARCHAR(16370)) RETURNS BIGINT LANGUAGE C SPECIFIC REDIS_ZADD NOT DETERMINISTIC NO SQL RETURNS NULL ON NULL INPUT DISALLOW PARALLEL NOT FENCED EXTERNAL NAME ''$(TGT_LIB)/REDISILE(zaddRedisSortedSet)'' PARAMETER STYLE DB2SQL') COMMIT(*NONE)"

# Create or replace the SQL function for redis_zrem
redis_zrem.func:
	-system "RUNSQL SQL('CREATE OR REPLACE FUNCTION $(TGT_LIB).REDIS_ZREM (KEY VARCHAR(255), MEMBER VARCHAR(16370)) RETURNS BIGINT LANGUAGE C SPECIFIC REDIS_ZREM NOT DETERMINISTIC NO SQL RETURNS NULL ON NULL INPUT DISALLOW PARALLEL NOT FENCED EXTERNAL NAME ''$(TGT_LIB)/REDISILE(zremRedisSortedSet)'' PARAMETER STYLE DB2SQL') COMMIT(*NONE)"

# Create or replace the SQL function for redis_zscore
redis_zscore.func:
	-system "RUNSQL SQL('CREATE OR REPLACE FUNCTION $(TGT_LIB).REDIS_ZSCORE (KEY VARCHAR(255), MEMBER VARCHAR(16370)) RETURNS VARCHAR(50) LANGUAGE C SPECIFIC REDIS_ZSCORE NOT DETERMINISTIC NO SQL RETURNS NULL ON NULL INPUT DISALLOW PARALLEL NOT FENCED EXTERNAL NAME ''$(TGT_LIB)/REDISILE(zscoreRedisSSet)'' PARAMETER STYLE DB2SQL') COMMIT(*NONE)"

# Create or replace the SQL function for redis_zrank
redis_zrank.func:
	-system "RUNSQL SQL('CREATE OR REPLACE FUNCTION $(TGT_LIB).REDIS_ZRANK (KEY VARCHAR(255), MEMBER VARCHAR(16370)) RETURNS BIGINT LANGUAGE C SPECIFIC REDIS_ZRANK NOT DETERMINISTIC NO SQL RETURNS NULL ON NULL INPUT DISALLOW PARALLEL NOT FENCED EXTERNAL NAME ''$(TGT_LIB)/REDISILE(zrankRedisSSet)'' PARAMETER STYLE DB2SQL') COMMIT(*NONE)"

# Create or replace the SQL function for redis_zcard
redis_zcard.func:
	-system "RUNSQL SQL('CREATE OR REPLACE FUNCTION $(TGT_LIB).REDIS_ZCARD (KEY VARCHAR(255)) RETURNS BIGINT LANGUAGE C SPECIFIC REDIS_ZCARD NOT DETERMINISTIC NO SQL RETURNS NULL ON NULL INPUT DISALLOW PARALLEL NOT FENCED EXTERNAL NAME ''$(TGT_LIB)/REDISILE(zcardRedisSSet)'' PARAMETER STYLE DB2SQL') COMMIT(*NONE)"

# Create or replace the SQL function for redis_zrange
redis_zrange.func:
	-system "RUNSQL SQL('CREATE OR REPLACE FUNCTION $(TGT_LIB).REDIS_ZRANGE (KEY VARCHAR(255), START_IDX INTEGER, STOP_IDX INTEGER) RETURNS VARCHAR(16370) LANGUAGE C SPECIFIC REDIS_ZRANGE NOT DETERMINISTIC NO SQL RETURNS NULL ON NULL INPUT DISALLOW PARALLEL NOT FENCED EXTERNAL NAME ''$(TGT_LIB)/REDISILE(zrangeRedisSSet)'' PARAMETER STYLE DB2SQL') COMMIT(*NONE)"

# Create or replace the SQL function for redis_zrangebyscore
redis_zrangebyscore.func:
	-system "RUNSQL SQL('CREATE OR REPLACE FUNCTION $(TGT_LIB).REDIS_ZRANGEBYSCORE (KEY VARCHAR(255), MINVAL VARCHAR(50), MAXVAL VARCHAR(50)) RETURNS VARCHAR(16370) LANGUAGE C SPECIFIC REDIS_ZRNGBYSCR NOT DETERMINISTIC NO SQL RETURNS NULL ON NULL INPUT DISALLOW PARALLEL NOT FENCED EXTERNAL NAME ''$(TGT_LIB)/REDISILE(zrangebyscoreRedisSSet)'' PARAMETER STYLE DB2SQL') COMMIT(*NONE)"

# --- Phase 8: Multi-key Operations ---

# Create or replace the SQL function for redis_mget
redis_mget.func:
	-system "RUNSQL SQL('CREATE OR REPLACE FUNCTION $(TGT_LIB).REDIS_MGET (KEYS VARCHAR(16370)) RETURNS VARCHAR(16370) LANGUAGE C SPECIFIC REDIS_MGET NOT DETERMINISTIC NO SQL RETURNS NULL ON NULL INPUT DISALLOW PARALLEL NOT FENCED EXTERNAL NAME ''$(TGT_LIB)/REDISILE(mgetRedisValues)'' PARAMETER STYLE DB2SQL') COMMIT(*NONE)"

# Create or replace the SQL function for redis_mset
redis_mset.func:
	-system "RUNSQL SQL('CREATE OR REPLACE FUNCTION $(TGT_LIB).REDIS_MSET (KVPAIRS VARCHAR(16370)) RETURNS VARCHAR(128) LANGUAGE C SPECIFIC REDIS_MSET NOT DETERMINISTIC NO SQL RETURNS NULL ON NULL INPUT DISALLOW PARALLEL NOT FENCED EXTERNAL NAME ''$(TGT_LIB)/REDISILE(msetRedisValues)'' PARAMETER STYLE DB2SQL') COMMIT(*NONE)"

# Create or replace the SQL function for redis_getset
redis_getset.func:
	-system "RUNSQL SQL('CREATE OR REPLACE FUNCTION $(TGT_LIB).REDIS_GETSET (KEY VARCHAR(255), VALUE VARCHAR(16370)) RETURNS VARCHAR(16370) LANGUAGE C SPECIFIC REDIS_GETSET NOT DETERMINISTIC NO SQL RETURNS NULL ON NULL INPUT DISALLOW PARALLEL NOT FENCED EXTERNAL NAME ''$(TGT_LIB)/REDISILE(getsetRedisValue)'' PARAMETER STYLE DB2SQL') COMMIT(*NONE)"

# Create or replace the SQL function for redis_rename
redis_rename.func:
	-system "RUNSQL SQL('CREATE OR REPLACE FUNCTION $(TGT_LIB).REDIS_RENAME (OLDKEY VARCHAR(255), NEWKEY VARCHAR(255)) RETURNS VARCHAR(128) LANGUAGE C SPECIFIC REDIS_RENAME NOT DETERMINISTIC NO SQL RETURNS NULL ON NULL INPUT DISALLOW PARALLEL NOT FENCED EXTERNAL NAME ''$(TGT_LIB)/REDISILE(renameRedisKey)'' PARAMETER STYLE DB2SQL') COMMIT(*NONE)"

# --- Phase 9: Hash/Set Scanning ---

# Create or replace the SQL function for redis_hscan
redis_hscan.func:
	-system "RUNSQL SQL('CREATE OR REPLACE FUNCTION $(TGT_LIB).REDIS_HSCAN (KEY VARCHAR(255), CURSOR VARCHAR(20), PATTERN VARCHAR(255), CNT INTEGER) RETURNS VARCHAR(16370) LANGUAGE C SPECIFIC REDIS_HSCAN NOT DETERMINISTIC NO SQL RETURNS NULL ON NULL INPUT DISALLOW PARALLEL NOT FENCED EXTERNAL NAME ''$(TGT_LIB)/REDISILE(hscanRedisHash)'' PARAMETER STYLE DB2SQL') COMMIT(*NONE)"

# Create or replace the SQL function for redis_sscan
redis_sscan.func:
	-system "RUNSQL SQL('CREATE OR REPLACE FUNCTION $(TGT_LIB).REDIS_SSCAN (KEY VARCHAR(255), CURSOR VARCHAR(20), PATTERN VARCHAR(255), CNT INTEGER) RETURNS VARCHAR(16370) LANGUAGE C SPECIFIC REDIS_SSCAN NOT DETERMINISTIC NO SQL RETURNS NULL ON NULL INPUT DISALLOW PARALLEL NOT FENCED EXTERNAL NAME ''$(TGT_LIB)/REDISILE(sscanRedisSet)'' PARAMETER STYLE DB2SQL') COMMIT(*NONE)"

# --- Phase 10: Server Operations ---

# Create or replace the SQL function for redis_dbsize
redis_dbsize.func:
	-system "RUNSQL SQL('CREATE OR REPLACE FUNCTION $(TGT_LIB).REDIS_DBSIZE () RETURNS BIGINT LANGUAGE C SPECIFIC REDIS_DBSIZE NOT DETERMINISTIC NO SQL CALLED ON NULL INPUT DISALLOW PARALLEL NOT FENCED EXTERNAL NAME ''$(TGT_LIB)/REDISILE(dbsizeRedis)'' PARAMETER STYLE DB2SQL') COMMIT(*NONE)"

# Create or replace the SQL function for redis_ping
redis_ping.func: redisile.srvpgm
	-system "RUNSQL SQL('CREATE OR REPLACE FUNCTION $(TGT_LIB).REDIS_PING () RETURNS VARCHAR(10) CCSID 37 LANGUAGE C SPECIFIC REDIS_PING NOT DETERMINISTIC NO SQL RETURNS NULL ON NULL INPUT DISALLOW PARALLEL NOT FENCED EXTERNAL NAME ''$(TGT_LIB)/REDISILE(pingRedis)'' PARAMETER STYLE DB2SQL') COMMIT(*NONE)"

# Benchmark: compile and create standalone program to compare table vs iconv
# Run: gmake bench   (after gmake all, since it needs the library)
# Execute: CALL REDIS400/REDISBENCH
bench:
	system "CRTCMOD MODULE($(TGT_LIB)/REDISBENCH) SRCSTMF('$(CURDIR)/srcfile/redisbench.c') SYSIFCOPT(*IFSIO *IFS64IO) INCDIR('$(CURDIR)/include/') DBGVIEW(*SOURCE)"
	system "CRTPGM PGM($(TGT_LIB)/REDISBENCH) MODULE($(TGT_LIB)/REDISBENCH) BNDSRVPGM(QSYS/QTQICONV) ACTGRP(*NEW)"

# Clean up
clean:
	-system "DLTLIB LIB($(TGT_LIB))"

# Phony targets
.PHONY: all preflight clean bench