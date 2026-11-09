#include "../../experimental/resident/resident_diagnostics.h"

#include <assert.h>
#include <pthread.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#define THREADS 8
#define RECORDS_PER_THREAD 16

typedef struct {
	airps3_resident_diagnostics *diagnostics;
	uint32_t thread_id;
} producer_args;

static void *producer(void *opaque)
{
	producer_args *args = (producer_args *)opaque;
	uint32_t i;
	for (i = 0; i < RECORDS_PER_THREAD; ++i)
		airps3_diag_log(args->diagnostics, args->thread_id * 1000U + i,
			"THREAD_EVENT", (int32_t)args->thread_id, i, i * 4U);
	return NULL;
}

int main(void)
{
	airps3_resident_diagnostics diagnostics;
	pthread_t threads[THREADS];
	producer_args args[THREADS];
	uint32_t seen[THREADS][RECORDS_PER_THREAD] = {{0}};
	uint32_t i;
	memset(&diagnostics, 0, sizeof(diagnostics));
	diagnostics.magic = AIRPS3_RESIDENT_DIAGNOSTICS_MAGIC;
	for (i = 0; i < THREADS; ++i) {
		args[i].diagnostics = &diagnostics;
		args[i].thread_id = i;
		assert(pthread_create(&threads[i], NULL, producer, &args[i]) == 0);
	}
	for (i = 0; i < THREADS; ++i)
		assert(pthread_join(threads[i], NULL) == 0);
	assert(__atomic_load_n(&diagnostics.log_write_index, __ATOMIC_ACQUIRE) ==
		AIRPS3_DIAG_LOG_CAPACITY);
	assert(__atomic_load_n(&diagnostics.log_dropped, __ATOMIC_ACQUIRE) == 0);
	for (i = 0; i < AIRPS3_DIAG_LOG_CAPACITY; ++i) {
		const airps3_resident_log_record *record = &diagnostics.log[i];
		uint32_t thread_id = (uint32_t)record->result;
		assert(__atomic_load_n(&record->committed_sequence, __ATOMIC_ACQUIRE) ==
			i + 1U);
		assert(strcmp(record->event, "THREAD_EVENT") == 0);
		assert(thread_id < THREADS);
		assert(record->value < RECORDS_PER_THREAD);
		assert(record->byte_count == record->value * 4U);
		assert(record->monotonic_us == thread_id * 1000U + record->value);
		assert(seen[thread_id][record->value]++ == 0U);
	}
	for (i = 0; i < THREADS; ++i) {
		uint32_t j;
		for (j = 0; j < RECORDS_PER_THREAD; ++j)
			assert(seen[i][j] == 1U);
	}
	airps3_diag_log(&diagnostics, 0, "OVERFLOW", -1, 0, 0);
	assert(__atomic_load_n(&diagnostics.log_write_index, __ATOMIC_ACQUIRE) ==
		AIRPS3_DIAG_LOG_CAPACITY + 1U);
	assert(__atomic_load_n(&diagnostics.log_dropped, __ATOMIC_ACQUIRE) == 1U);
	puts("resident diagnostics queue: 128 concurrent records, ordered commits, overflow accounting PASS");
	return 0;
}
