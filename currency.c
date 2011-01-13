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

#include "currency_code.h"

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

/* memory/heap structure (not for binary marshalling) */
typedef struct currency
{
        int16 currency_number;
	int16 exponent;		/* precision-minor from currency_rate */
        int64 value;
} currency;

/* creating a currency from a string */
currency* parse_currency(char* str)
{
	double value;
	currency *newval;
	ccc_ent *ccinfo;
	char code[4];
	char fail[2] = "";

	if (sscanf(str, "%lf %3c%1c", &value, &code, &fail) != 2) {
		elog(ERROR, "bad currency value '%s'", str);
	}
	code[3] = '\0';
	elog(WARNING, "value = %g, code = %s", value, &code);

	newval = palloc(sizeof(currency));

	if (!(newval->currency_number = parse_currency_code(&code))) {
		elog(ERROR, "bad currency code '%s'", &code);
		goto error_out;
	}

	ccinfo = lookup_currency_number(newval->currency_number);

	newval->exponent = ccinfo->currency_precision - ccinfo->currency_minor;
	value = value / pow(10, newval->exponent);
	newval->value = rint(value);

	elog(WARNING, "parse_currency: returning '%d'; %d * 10^%d (prec = %d, minor = %d): %lf => %lf", newval->currency_number, newval->value, newval->exponent, ccinfo->currency_precision, ccinfo->currency_minor, value, rint(value));
	return newval;

 error_out:
	pfree(newval);
	return 0;
}

int emit_currency_buf(currency* amount, char* result)
{
	ccc_ent *ccinfo = lookup_currency_number(amount->currency_number);
	int bias;
	double display_val;
	int diff;
	char format[9] = "%.xlf %s";
	if (!ccinfo) {
		return 0;
	}
	bias = ccinfo->currency_precision - ccinfo->currency_minor;
	display_val = pow(10, amount->exponent) * amount->value;
	diff = bias - amount->exponent;
	format[2] = '0' + ccinfo->currency_minor;
	elog(WARNING, "currency_in_cstring: format = '%s', bias = %d, display_val = %lf, diff = %d", &format, bias, display_val, diff);

	if (diff > 0) {
		display_val = display_val / pow(10, diff);
		display_val = rint(display_val);
	}
	else if (diff < 0) {
		display_val = display_val * pow(10, diff);
	}

	sprintf(result, &format, display_val, &ccinfo->currency_code);
	elog(WARNING, "currency_in_cstring: formatted = '%s'", result);
	return 1;
}

char* emit_currency(currency* amount) {
	char buf[32];
	char* res;
	if (emit_currency_buf(amount, &buf)) {
		if (strlen(&buf) > 31)
			return 0;
		res = palloc(strlen(&buf)+1);
		strcpy(res, &buf);
		return res;
	}
	else {
		return 0;
	}
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
	elog(WARNING, "currency_in_cstring: parsing '%s'", str);
	currency *result = parse_currency(str);
	if (!result)
		PG_RETURN_NULL();

	PG_RETURN_POINTER(result);
}/* output function: C string */

PG_FUNCTION_INFO_V1(currency_out_cstring);
Datum
currency_out_cstring(PG_FUNCTION_ARGS)
{
	currency* amount = (void*)PG_GETARG_POINTER(0);
	char *result;
	elog(WARNING, "currency_out_cstring: emitting '%d'; %d * 10^%d", amount->currency_number, amount->value, amount->exponent);

	result = emit_currency(amount);

	PG_RETURN_CSTRING(result);
}

