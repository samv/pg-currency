
#ifndef CURRENCY_CODE_H
#define CURRENCY_CODE_H

#include "postgres.h"

#include <time.h>
#include <unistd.h>

#include "fmgr.h"
#include "utils/builtins.h"

Datum		currency_code_in(PG_FUNCTION_ARGS);
Datum		currency_code_out(PG_FUNCTION_ARGS);
Datum		currency_code_in_text(PG_FUNCTION_ARGS);
Datum		currency_code_out_text(PG_FUNCTION_ARGS);

typedef int16 currency_code;

int16 parse_currency_code(char* str);

int emit_currency_code_buf(int16 currency_code, char* result);

char* emit_currency_code(int16 currency_code);

static TransactionId ccc_txid = -1;
static CommandId ccc_cmdid = -1;

typedef struct ccc_ent
{
	int16 currency_number;
	char currency_code[4];
	int16 currency_minor;
	int16 currency_precision;
	float currency_rate;
	char* currency_symbol;
} ccc_ent;

static ccc_ent* currency_code_cache;
static int currency_code_cache_size;

int _update_currency_code_cache();

inline void update_currency_code_cache();

ccc_ent* lookup_currency_number(int16 currency_number);

#endif
