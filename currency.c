/*
 * PostgreSQL type definitions for currency type
 * Written by Sam Vilain
 * sam.vilain@adioso.com
 *
 * contrib/currency/currency.c
 */

#include "postgres.h"

#include <time.h>
#include <stdio.h>
#include <unistd.h>
#include <math.h>

#include "fmgr.h"

/*
 * in principle, we could select oids from the various catalog tables
 * and use the FCI to call the appropriate functions by Oid, but this
 * saves that SPI call and other questions about invalidating/caching
 * that retrieved Oid.
 */
#include "tla.h"

/*
 * This type combines int8 fixed-point numbers with a currency code,
 * to provide automatically converted values via an auxilliary lookup
 * table.
 */

/* IO methods */
Datum		currency_in_cstring(PG_FUNCTION_ARGS);
Datum		currency_out_cstring(PG_FUNCTION_ARGS);
//Datum		currency_in_text(PG_FUNCTION_ARGS);
//Datum		currency_out_text(PG_FUNCTION_ARGS);

/* comparisons */
Datum		currency_eq(PG_FUNCTION_ARGS);
Datum		currency_ne(PG_FUNCTION_ARGS);
Datum		currency_lt(PG_FUNCTION_ARGS);
Datum		currency_le(PG_FUNCTION_ARGS);
Datum		currency_ge(PG_FUNCTION_ARGS);
Datum		currency_gt(PG_FUNCTION_ARGS);

/* necessary typecasting methods */
//Datum		currency_in_text(PG_FUNCTION_ARGS);
//Datum		currency_out_text(PG_FUNCTION_ARGS);
//Datum		currency_in_money(PG_FUNCTION_ARGS);
//Datum		currency_out_money(PG_FUNCTION_ARGS);

#define numeric_oid 1700
#define numeric_in 1701
#define numeric_out 1702
#define numeric_round_scale 1707
#define numeric_eq 1718
#define numeric_ne 1719
#define numeric_gt 1720
#define numeric_ge 1721
#define numeric_lt 1722
#define numeric_le 1723
#define numeric_add 1724
#define numeric_sub 1725
#define numeric_mul 1726
#define numeric_div 1727

/* memory/heap structure (not for binary marshalling) */
typedef struct currency
{
	int32 vl_len_;	/* varlena header */
        int16 currency_code;
	char numeric[]; /* numeric data, EXCLUDING the varlena header */
} currency;

#define alloc_varlena( var, size ) \
	var = palloc( size ); \
	SET_VARSIZE( var, size );

/* handy little function for dumping chunks of memory as hex :) */
char* dump_hex(void* ptr, int len)
{
	char* hexa;
	int i;
	char* x;
	unsigned char* nx;

	hexa = palloc( len * 2 + 1 );
	x = hexa;
	nx = ptr;
	for ( i = 0; i < len; i++ ) {
		sprintf(x, "%.2x", *nx);
		x += 2;
		nx++;
	}
	return hexa;
}

/* creating a currency from a string */
currency* parse_currency(char* str)
{
	char* number = palloc(strlen(str)+1);
	Datum numeric_val;
	char code[4];
	char fail[2] = "";
	currency *newval;

	// use sscanf, so that whitespace between number and currency
	// code can be optional.
	if (sscanf(str, " %s %3c %1c", number, (char*)&code, (char*)&fail) != 2) {
		elog(ERROR, "bad currency value '%s'", str);
	}
	code[3] = '\0';
	//elog(WARNING, "number = %s, code = %s", number, (char*)&code);

	// use numeric_in to parse the numeric
	numeric_val = OidFunctionCall3(
		numeric_in,
		CStringGetDatum( number ),
		numeric,
		-1  /* typmod */
		);
	//elog(WARNING, "numeric_in called ok, length %d, numeric_val = %s", VARSIZE(numeric_val), dump_hex(DatumGetPointer(numeric_val), VARSIZE(numeric_val)) );
	
	alloc_varlena(newval, VARSIZE( numeric_val ) + offsetof(currency, numeric) - VARHDRSZ );
	memcpy( &newval->numeric,
		DatumGetPointer( numeric_val ) + VARHDRSZ,
		VARSIZE( numeric_val ) - VARHDRSZ );

	if (!(newval->currency_code = parse_tla(&code))) {
		elog(ERROR, "bad currency code '%s'", &code);
		goto error_out;
	}

	//elog(WARNING, "parse_tla called ok, currency_code = %.4x", newval->currency_code);

	//elog(WARNING, "return struct = %s", dump_hex(newval, VARSIZE(newval)));

	return newval;

 error_out:
	pfree(newval);
	return 0;
}

char* emit_currency(currency* amount) {
	char* res;
	char* outstr;
	char* c;

	/* construct a varlena structure so we can call the
	 * numeric_out function and have a happy life */
	struct varlena* tv;

	//elog(WARNING, "emit_currency: alloc_varlena(tv, %d)", VARSIZE( amount ) - offsetof(currency, numeric) + VARHDRSZ );
	alloc_varlena(
		tv,
		VARSIZE( amount ) - offsetof(currency, numeric) + VARHDRSZ
		);
	memcpy( (char*)tv + VARHDRSZ,
		&amount->numeric,
		VARSIZE( amount ) - offsetof(currency, numeric)
		);

	//elog(WARNING, "emit_currency: calling numeric_out on %s", dump_hex(tv, VARSIZE(tv)));
	outstr = OidFunctionCall1( numeric_out, PointerGetDatum( tv ) );
	//elog(WARNING, "emit_currency: outstr = %s", outstr);

	res = palloc( strlen(outstr) + 5 );
	memcpy( res, outstr, strlen(outstr)+1 );
	c = res + strlen(res);
	*c++ = ' ';
	emit_tla_buf( amount->currency_code, c );
	c += 3;
	c = '\0';
	//elog(WARNING, "emit_currency: res = %s", res);

	return res;
}


/*
 * Pg bindings
 */

/* input function: C string */
PG_FUNCTION_INFO_V1(currency_in_cstring);
Datum
currency_in_cstring(PG_FUNCTION_ARGS)
{
	char *str = PG_GETARG_CSTRING(0);
	//elog(WARNING, "currency_in_cstring: parsing '%s'", str);
	currency *result = parse_currency(str);
	if (!result)
		PG_RETURN_NULL();
	//elog(WARNING, "currency_in_cstring: ok, returning '%x'", result);

	PG_RETURN_POINTER(result);
}/* output function: C string */

PG_FUNCTION_INFO_V1(currency_out_cstring);
Datum
currency_out_cstring(PG_FUNCTION_ARGS)
{
	currency* amount = (void*)PG_GETARG_POINTER(0);
	char *result;
	//elog(WARNING, "currency_out_cstring: emitting (%x) '%s'", amount, dump_hex(amount, VARSIZE(amount)));
	result = emit_currency(amount);

	PG_RETURN_CSTRING(result);
}
