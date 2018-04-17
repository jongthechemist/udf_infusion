#include "common.h"

DLLEXPORT my_bool row_number_distinct_init(UDF_INIT *initid, UDF_ARGS *args, char *message) {
	longlong *data;

	if (0 != args->arg_count) {
		strcpy(message, "rownumber doesn't require an argument");
		return 1;
	}

	if (NULL == (data = malloc(sizeof(*data)))) {
		strcpy(message, "Couldn't allocate memory");
		return 1;
	}
	*data = 0;

	initid->ptr = (char *)data;
	initid->const_item = 0;
	initid->maybe_null = 1;

	return 0;
}

DLLEXPORT longlong row_number_distinct(UDF_INIT *initid, UDF_ARGS *args, char *is_null, char *error) {
	longlong *data;

	if (initid->ptr) {
		data = (longlong *)initid->ptr;
		return ++(*data);
	}

	*error = 1;
	return 0;
}

DLLEXPORT void row_number_distinct_deinit(UDF_INIT *initid) {
	if (initid->ptr) {
		free(initid->ptr);
	}
}
