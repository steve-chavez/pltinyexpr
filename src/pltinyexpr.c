#include <postgres.h>

#include <tcop/utility.h>
#include <utils/builtins.h>
#include <utils/varlena.h>
#include <utils/syscache.h>
#include <funcapi.h>

#include <catalog/pg_proc.h>

#include "tinyexpr.h"

PG_MODULE_MAGIC;

PG_FUNCTION_INFO_V1(pltinyexpr_inline_handler);
Datum pltinyexpr_inline_handler(PG_FUNCTION_ARGS)
{
	InlineCodeBlock *codeblock = (InlineCodeBlock *) DatumGetPointer(PG_GETARG_DATUM(0));
	char* src = codeblock->source_text;

	double res;
	int error_pos;

	res = te_interp(src, &error_pos);

	if (error_pos > 0)
		ereport(ERROR, errmsg("%.*s<- bad syntax here", error_pos, src));
	else
		elog(INFO, "expression result is: %lf", res);

	PG_RETURN_VOID();
}

PG_FUNCTION_INFO_V1(pltinyexpr_handler);
Datum pltinyexpr_handler(PG_FUNCTION_ARGS)
{
	Oid function_oid = fcinfo->flinfo->fn_oid;
	HeapTuple proc_tuple;
	Datum prosrc;
	char* src;
	bool is_null;
	int numargs;
	Oid* argtypes;
	char** argnames;
	char* argmodes;

	double res;
	int error_pos;

	proc_tuple = SearchSysCache(PROCOID, ObjectIdGetDatum(function_oid), 0, 0, 0);
	if (!HeapTupleIsValid(proc_tuple))
		elog(ERROR, "could not find function with oid %u", function_oid);

	prosrc = SysCacheGetAttr(PROCOID, proc_tuple, Anum_pg_proc_prosrc, &is_null);
	if(is_null)
			ereport(ERROR, errmsg("pg_proc.prosrc is NULL"));

	src = text_to_cstring(DatumGetTextPP(prosrc));

	numargs = get_func_arg_info(proc_tuple, &argtypes, &argnames, &argmodes);

	if(numargs > 0){
		te_expr* expr;
		te_variable* vars = palloc0(sizeof(te_variable) * numargs);
		double* argvalues = palloc0(sizeof(double) * numargs);

		for(int i = 0; i < numargs; i++){
			argvalues[i] = DatumGetFloat8(fcinfo->args[i].value);
			vars[i].name = argnames[i];
			vars[i].address = &(argvalues[i]);
		}

		expr = te_compile(src, vars, numargs, &error_pos);

		if (expr) {
			res = te_eval(expr);
			pfree(vars);
			pfree(argvalues);
			te_free(expr);
		} else {
			pfree(vars);
			pfree(argvalues);
			ereport(ERROR, errmsg("%.*s<- bad syntax here", error_pos, src));
		}
	} else {
		te_expr* expr;
		expr = te_compile(src, NULL, 0, &error_pos);
		res = te_eval(expr);

		if (expr)
			te_free(expr);
		else
			ereport(ERROR, errmsg("%.*s<- bad syntax here", error_pos, src));
	}

	ReleaseSysCache(proc_tuple);

	PG_RETURN_FLOAT8(res);
}
