/*
 * PostgreSQL type definitions for tla type
 * Written by Sam Vilain
 * samv@adioso.com
 *
 * contrib/currency/tla.c
 */

#include "postgres.h"

#include <time.h>
#include <unistd.h>

#include "fmgr.h"
#include "utils/builtins.h"

#include "tla.h"

PG_MODULE_MAGIC;

static char tla_alphabet[] = "0ABCDEFGHIJKLMNOPQRSTUVWXYZ12345";

/*
 * This type packs a three-letter code, such as a currency or an
 * airport code, into an int2.
 *
 * Each letter is coded into 5 bits of the output int2, with 6 dummy
 * letters for the extra 6 letters of the 32-letter alphabet, which
 * are not currently accepted on input but output as [0-5]
 */

inline void emit_tla_buf(int32 tla, char* result) {
	result[3] = '\0';
	result[2] = tla_alphabet[tla & 0x1f];
	result[1] = tla_alphabet[(tla & (0x1f << 5)) >> 5];
	result[0] = tla_alphabet[(tla & (0x1f << 10)) >> 10];
}

int32 parse_tla(char* str)
{
	int32 result = 0;
	int i;
	char* str_x = str;

	for (i=0; *str_x; str_x++,i++) {
		result <<= 5;
		if ( *str_x >= 'A' && *str_x <= 'Z' ) {
			result += (unsigned char)(*str_x) - 64;
		}
		else if ( *str_x >= 'a' && *str_x <= 'z' ) {
			result += (unsigned char)(*str_x) - 64 - 32;
		}
		else {
			ereport(ERROR,
				(errcode(ERRCODE_DATA_EXCEPTION),
				 errmsg("invalid char \"%c\" in tla", *str_x)
					)); 
			goto error_out;
		}
	}
	if ( i != 3 ) {
		ereport(ERROR,
			(errcode(ERRCODE_DATA_EXCEPTION),
			 errmsg("invalid length tla \"%s\"", str)
				)); 
		goto error_out;
	}
	
	return result;
 error_out:
	return 0;
}

char* emit_tla(int32 tla)
{
	char *result;

	result = (char *) palloc(4);
	emit_tla_buf(tla, result);

	return result;
}

/*
 * TLA reader.
 */
PG_FUNCTION_INFO_V1(tla_in);
Datum
tla_in(PG_FUNCTION_ARGS)
{
	char *str = PG_GETARG_CSTRING(0);
	PG_RETURN_INT32(parse_tla(str));
}

/*
 * TLA output function.
 */

PG_FUNCTION_INFO_V1(tla_out);
Datum
tla_out(PG_FUNCTION_ARGS)
{
	int32 tla = PG_GETARG_INT32(0);
	char *result;

	result = emit_tla(tla);

	PG_RETURN_CSTRING(result);
}

/*
 * TLA casting : in from TEXT
 */
PG_FUNCTION_INFO_V1(tla_in_text);
Datum
tla_in_text(PG_FUNCTION_ARGS)
{
	text* tla = PG_GETARG_TEXT_PP(0);
	int32 result = parse_tla(VARDATA(tla));
	PG_FREE_IF_COPY(tla, 0);
	PG_RETURN_INT32(result);
}

/*
 * TLA casting : out to TEXT
 */
PG_FUNCTION_INFO_V1(tla_out_text);
Datum
tla_out_text(PG_FUNCTION_ARGS)
{
	int32 tla = PG_GETARG_INT32(0);
	char* xxx = emit_tla(tla);
	text* res = cstring_to_text(xxx);
	pfree(xxx);
	PG_RETURN_TEXT_P(res);
}

