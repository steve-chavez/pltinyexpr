#include <postgres.h>

#include <tcop/utility.h>
#include <utils/builtins.h>
#include <utils/varlena.h>
#include <utils/syscache.h>

#include <catalog/pg_proc.h>

#include "tinyexpr.h"

PG_MODULE_MAGIC;

PG_FUNCTION_INFO_V1(pltinyexpr_handler);
Datum pltinyexpr_handler(PG_FUNCTION_ARGS)
{
	Oid function_oid = fcinfo->flinfo->fn_oid;
	HeapTuple proc_tuple;
	Datum prosrc;
	char* src;
	bool is_null;

	double res;
	int error_pos;

	proc_tuple = SearchSysCache(PROCOID, ObjectIdGetDatum(function_oid), 0, 0, 0);
	if (!HeapTupleIsValid(proc_tuple))
		elog(ERROR, "could not find function with oid %u", function_oid);

	prosrc = SysCacheGetAttr(PROCOID, proc_tuple, Anum_pg_proc_prosrc, &is_null);
	if(is_null)
			ereport(ERROR, errmsg("pg_proc.prosrc is NULL"));

	src = text_to_cstring(DatumGetTextPP(prosrc));

	res = te_interp(src, &error_pos);

	if (error_pos > 0)
		elog(ERROR, "%.*s<- bad syntax here", error_pos, src);

	ReleaseSysCache(proc_tuple);

	PG_RETURN_FLOAT8(res);
}
