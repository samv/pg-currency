# contrib/currency/Makefile

MODULE_big = currency
OBJS = currency_code.o
DATA_built = currency.sql
DATA = uninstall_currency.sql

ifdef USE_PGXS
PG_CONFIG = pg_config
PGXS := $(shell $(PG_CONFIG) --pgxs)
include $(PGXS)
else
subdir = contrib/currency
top_builddir = ../..
include $(top_builddir)/src/Makefile.global
include $(top_srcdir)/contrib/contrib-global.mk
endif
