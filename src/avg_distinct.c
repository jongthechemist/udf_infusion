#include "common.h"
#include "array.h"

struct Buffer {
	struct array values;
	unsigned count;
};

typedef struct Tuple {
	double key;
	double value;
} Tuple;

DLLEXPORT my_bool avg_distinct_init(UDF_INIT *initid, UDF_ARGS *args, char *message) {
	struct Buffer *data;

	if (2 != args->arg_count) {
		strcpy(message, "avg_distinct must have exactly two arguments");
		return 1;
	}

	args->arg_type[0] = REAL_RESULT;
	args->arg_type[1] = REAL_RESULT;

	data = calloc(1, sizeof(*data));
	if (NULL == data) {
		snprintf(message, MYSQL_ERRMSG_SIZE, "Memory allocation failed");
		return 1;
	}

	if (NULL == array_init(&data->values, sizeof(Tuple), 32)) {
		snprintf(message, MYSQL_ERRMSG_SIZE, "Memory allocation failed");
		return 1;
	}

	initid->decimals = NOT_FIXED_DEC;
	initid->maybe_null = 1;
	initid->ptr = (char*)data;

	return 0;
}

DLLEXPORT void avg_distinct_clear(UDF_INIT* initid, char* is_null, char *error) {
	struct Buffer *data = (struct Buffer *) initid->ptr;
	array_truncate(&data->values);
	data->count = 0;
}

DLLEXPORT void avg_distinct_add(UDF_INIT* initid, UDF_ARGS* args, char* is_null, char *error) {
	struct Buffer *data = (struct Buffer *) initid->ptr;

	if (NULL == args->args[0])
		return;

	Tuple *newTuple;
	newTuple = malloc(sizeof(*newTuple));
	newTuple->key = *((double *)args->args[0]);
	newTuple->value = *((double *)args->args[1]);
	if (NULL == array_append(&data->values, newTuple)) {
		*error = 1;
		return;
	}
	free(newTuple);
	data->count++;
}

DLLEXPORT void avg_distinct_deinit(UDF_INIT *initid) {
	struct Buffer *data = (struct Buffer *) initid->ptr;

	if (NULL != data) {
		array_free(&data->values);
		free(data);
		data = NULL;
	}
}


static int compar(const void *pa, const void *pb) {
	return ((Tuple *)pa)->key - ((Tuple *)pb)->key;
}

DLLEXPORT double avg_distinct(UDF_INIT *initid __attribute__((unused)), UDF_ARGS *args,
	char *is_null,
	char *error __attribute__((unused))) {
	struct Buffer *data = (struct Buffer *) initid->ptr;


	if (data->values.n == 0) {
		*is_null = 1;
		return 0;
	}

	qsort(data->values.p, data->values.n, sizeof(Tuple), compar);

	double sum = 0;
	unsigned count = 0;
	double key;
	size_t i = 0;
	Tuple *currentTuple = NULL;


	for (i = 0; i < data->values.n; i++) {

		currentTuple = (Tuple *)((char *)data->values.p + data->values.size*i);
		if (currentTuple != NULL && (i == 0 || key - currentTuple->key != 0)) {
			key = currentTuple->key;
			sum += currentTuple->value;
			count++;
		}
	}
	return sum/count;
}
