
#include "postgres.h"

#include <time.h>
#include <unistd.h>

#include "fmgr.h"
#include "utils/builtins.h"

Datum		tla_in(PG_FUNCTION_ARGS);
Datum		tla_out(PG_FUNCTION_ARGS);
Datum		tla_in_text(PG_FUNCTION_ARGS);
Datum		tla_out_text(PG_FUNCTION_ARGS);

int32 parse_tla(char* str);

inline void emit_tla_buf(int32 tla, char* result);

char* emit_tla(int32 tla);
