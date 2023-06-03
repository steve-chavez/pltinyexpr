EXTENSION = pltinyexpr

DATA = $(wildcard sql/*--*.sql)

MODULE_big = $(EXTENSION)
OBJS = src/tinyexpr.o src/pltinyexpr.o

TESTS = $(wildcard test/sql/*.sql)
REGRESS = $(patsubst test/sql/%.sql,%,$(TESTS))
REGRESS_OPTS = --inputdir=test

PG_CONFIG = pg_config

PG_CFLAGS = -std=c99 -Wall -Werror -Wno-declaration-after-statement

PGXS := $(shell $(PG_CONFIG) --pgxs)
include $(PGXS)
