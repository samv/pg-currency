/*
 * PostgreSQL type definitions for currency_code type
 * Written by Sam Vilain
 * sam.vilain@adioso.com
 *
 * contrib/adioso/currency_code.c
 */

#include "postgres.h"

#include <time.h>
#include <unistd.h>

#include "fmgr.h"
#include "utils/builtins.h"
#include "utils/lsyscache.h"
#include "utils/memutils.h"
#include "executor/spi.h"
#include "access/xact.h"

#include "currency_code.h"

PG_MODULE_MAGIC;

/*
 * This type uses a three-letter currency code, which is checked
 * against the currency_rate table and saved as that code's
 * corresponding 3-digit code.
 */

int _update_currency_code_cache() {
	int res, i;
	HeapTuple tuple;
	TupleDesc tupdesc;
	Datum attr;
	Oid typoutput;
	bool typisvarlena, isnull;
	char* outputstr;

	if (SPI_connect() == SPI_ERROR_CONNECT) {
		elog(ERROR, "failed to connect to SPI");
		return 0;
	}
	res = SPI_execute(
		"select"
		" currency_number, currency_code"
		" from currency_rate"
		" order by currency_number"
		, true
		, 1000
		);
	if (res != SPI_OK_SELECT) {
		elog(ERROR, "failed to fetch currency codes");
		return 0;
	}

	if (currency_code_cache && currency_code_cache_size != SPI_processed) {
		pfree(currency_code_cache);
		currency_code_cache = 0;
	}
	if (!currency_code_cache) {
		currency_code_cache = MemoryContextAlloc(
			CurTransactionContext,
			sizeof(ccc_ent) * SPI_processed
			);
		currency_code_cache_size = SPI_processed;
	}

	/* results are in SPI_tuptable */
	tupdesc = SPI_tuptable->tupdesc;
	getTypeOutputInfo(
		tupdesc->attrs[1]->atttypid,
		&typoutput, &typisvarlena
		);

	/* elog(WARNING, "currency table has %d currencies", SPI_processed); */
	
	for (i = 0; i < SPI_processed; i++) {
		tuple = SPI_tuptable->vals[i];
		attr = heap_getattr(tuple, 1, tupdesc, &isnull);
		currency_code_cache[i].currency_number = attr;
		attr = heap_getattr(tuple, 2, tupdesc, &isnull);
		outputstr = OidOutputFunctionCall(typoutput, attr);
		strncpy(currency_code_cache[i].currency_code, outputstr, 4);
		/* elog(WARNING, "added code[%d]: %d = %s", i, currency_code_cache[i].currency_number, outputstr); */
		pfree(outputstr);
	}
	/* elog(WARNING, "finished iterating"); */

	SPI_finish();

	/* elog(WARNING, "SPI finished"); */
	ccc_cmdid = GetCurrentCommandId(false);
	return currency_code_cache_size;
}

inline int emit_currency_code_buf(int16 currency_number, char* result) {
	int max, min, i;
	int16 f;

	update_currency_code_cache();

	min = 0;
	max = currency_code_cache_size - 1;

	while (min <= max) {
		i = (min + max) >> 1;
		/* elog(WARNING, "emit_currency_code_buf, i = %d", i); */
		f = currency_code_cache[i].currency_number;
		if ( f == currency_number ) {
			/* elog(WARNING, "emit_currency_code_buf, found"); */
			strncpy( result,
				 currency_code_cache[i].currency_code,
				 4
				);
			return 1;
		}
		else if ( f < currency_number ) {
			min = i + 1;
		}
		else {
			max = i - 1;
		}
	}
	/* elog(WARNING, "emit_currency_code_buf returning 0"); */
	result[0] = '\0';
	return 0;
}

int16 parse_currency_code(char* str)
{
	int i;
	char uc_str[4];
	update_currency_code_cache();
	/* uppercase str */
	for (i=0; i<3; i++) {
		uc_str[i] = str[i] & 0x5f;
		if (uc_str[i] < 'A' || uc_str[i] > 'Z') {
			elog(ERROR, "invalid currency code '%s'", str);
		}
	}
	uc_str[3] = '\0';

	for (i=0; i<currency_code_cache_size; i++) {
		/* elog(WARNING, "parse_currency_code: code[%d] = %s (%d)", i, currency_code_cache[i].currency_code, currency_code_cache[i].currency_number); */
		if (strcmp(uc_str, currency_code_cache[i].currency_code)
		    ==0) {
			/* elog(WARNING, "parse_currency_code, found"); */
			return currency_code_cache[i].currency_number;
		}
	}
	elog(ERROR, "unknown currency code '%s'", (char*)&uc_str);
	return -1;
}

char* emit_currency_code(int16 currency_code)
{
	char *result;

	/* elog(WARNING, "emit_currency_code[%d]", currency_code); */
	result = (char *) palloc(4);
	emit_currency_code_buf(currency_code, result);

	return result;
}

/*
 * CURRENCY_CODE reader.
 */
PG_FUNCTION_INFO_V1(currency_code_in);
Datum
currency_code_in(PG_FUNCTION_ARGS)
{
	char *str = PG_GETARG_CSTRING(0);
	PG_RETURN_INT32(parse_currency_code(str));
}

/*
 * CURRENCY_CODE output function.
 */

PG_FUNCTION_INFO_V1(currency_code_out);
Datum
currency_code_out(PG_FUNCTION_ARGS)
{
	int32 currency_code = PG_GETARG_INT32(0);
	char *result;

	result = emit_currency_code(currency_code);

	PG_RETURN_CSTRING(result);
}

/*
 * CURRENCY_CODE casting : in from TEXT
 */
PG_FUNCTION_INFO_V1(currency_code_in_text);
Datum
currency_code_in_text(PG_FUNCTION_ARGS)
{
	text* currency_code = PG_GETARG_TEXT_PP(0);
	int32 result = parse_currency_code(VARDATA(currency_code));
	PG_FREE_IF_COPY(currency_code, 0);
	PG_RETURN_INT32(result);
}

/*
 * CURRENCY_CODE casting : out to TEXT
 */
PG_FUNCTION_INFO_V1(currency_code_out_text);
Datum
currency_code_out_text(PG_FUNCTION_ARGS)
{
	int32 currency_code = PG_GETARG_INT32(0);
	char* xxx = emit_currency_code(currency_code);
	text* res = cstring_to_text(xxx);
	pfree(xxx);
	PG_RETURN_TEXT_P(res);
}

