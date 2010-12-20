
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

inline int emit_currency_code_buf(int16 currency_code, char* result);

char* emit_currency_code(int16 currency_code);

