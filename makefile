# Configuration
SRC_DIR = srcfile
BND_DIR = qsrvsrc
TGT_LIB = REDIS400
DBG_VIEW = *NONE
CCSID = 1252 # Windows, Latin 1

# Targets
all: preflight $(TGT_LIB).lib generate_config redisile.srvpgm redis_get.func redis_set.func \
	redis_incr.func redis_del.func redis_expire.func redis_ttl.func 

redisile.srvpgm: redisget.cle redisset.cle redisincr.cle redisdel.cle redisexp.cle redisttl.cle \
	redisutils.cle
redisget.cle: redisget.cmodule redisget.bnd
redisset.cle: redisset.cmodule redisset.bnd
redisincr.cle: redisincr.cmodule redisincr.bnd
redisdel.cle: redisdel.cmodule redisdel.bnd
redisexp.cle: redisexp.cmodule redisexp.bnd
redisttl.cle: redisttl.cmodule redisttl.bnd
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
	system "CRTCMOD MODULE($(TGT_LIB)/$*) SRCSTMF('$<') SYSIFCOPT(*IFSIO *IFS64IO) INCDIR('$(CURDIR)/include/') DBGVIEW($(DBG_VIEW))"
	@touch $*.cle

# Create binding directory and add binding source
%.bnd: 
	-system -qi "RTVBNDSRC MODULE($(TGT_LIB)/$*) SRCFILE($(TGT_LIB)/QSRVSRC) MBROPT(*REPLACE)"

# Create service programs
%.srvpgm: $(BND_DIR)/%.bnd
	$(eval modules := $(patsubst %,$(TGT_LIB)/%,$(basename $(filter %.cle,$(notdir $^)))))
	system "CRTSRVPGM SRVPGM($(TGT_LIB)/$*) MODULE($(modules)) SRCSTMF('$(BND_DIR)/$*.bnd') OPTION(*DUPPROC *DUPVAR) EXPORT(*SRCFILE) ACTGRP(*CALLER)"

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
	
# Clean up
clean:
	-system "DLTLIB LIB($(TGT_LIB))"

# Phony targets
.PHONY: all preflight clean