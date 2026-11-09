#ifndef AIRPS3_RESIDENT_DIAGNOSTICS_H
#define AIRPS3_RESIDENT_DIAGNOSTICS_H

#include <stdint.h>
#include <stddef.h>

#define AIRPS3_DIAG_LOG_CAPACITY 128U
#define AIRPS3_DIAG_EVENT_NAME_MAX 32U

typedef struct airps3_resident_log_record {
	volatile uint32_t committed_sequence;
	uint64_t monotonic_us;
	int32_t result;
	uint32_t value;
	uint32_t byte_count;
	char event[AIRPS3_DIAG_EVENT_NAME_MAX];
} airps3_resident_log_record;

#define AIRPS3_RESIDENT_DIAGNOSTICS_MAGIC 0x41505258U

typedef struct airps3_resident_diagnostics {
	uint32_t magic;
	uint32_t stage;
	int32_t error;
	uint32_t events;
	uint32_t flags;
	volatile uint32_t log_write_index;
	volatile uint32_t log_dropped;
	airps3_resident_log_record log[AIRPS3_DIAG_LOG_CAPACITY];
} airps3_resident_diagnostics;

/* Bounded multi-producer diagnostic journal. Text is restricted to internal
 * event names; callers must never pass packet contents or peer identifiers. */
static inline void airps3_diag_log(airps3_resident_diagnostics *diagnostics,
	uint64_t monotonic_us, const char *event, int32_t result, uint32_t value,
	uint32_t byte_count)
{
	uint32_t sequence;
	airps3_resident_log_record *record;
	size_t i;
	if (diagnostics == NULL || event == NULL)
		return;
	sequence = __atomic_fetch_add(&diagnostics->log_write_index, 1U,
		__ATOMIC_RELAXED);
	if (sequence >= AIRPS3_DIAG_LOG_CAPACITY) {
		(void)__atomic_add_fetch(&diagnostics->log_dropped, 1U,
			__ATOMIC_RELAXED);
		return;
	}
	record = &diagnostics->log[sequence];
	record->monotonic_us = monotonic_us;
	record->result = result;
	record->value = value;
	record->byte_count = byte_count;
	for (i = 0; i + 1U < AIRPS3_DIAG_EVENT_NAME_MAX && event[i] != '\0'; ++i)
		record->event[i] = event[i];
	record->event[i] = '\0';
	while (++i < AIRPS3_DIAG_EVENT_NAME_MAX)
		record->event[i] = '\0';
	__atomic_store_n(&record->committed_sequence, sequence + 1U,
		__ATOMIC_RELEASE);
}

#define AIRPS3_DIAG_FLAG_SUPPRESS_TEST_TONE 0x01U

#define AIRPS3_DIAG_EVENT_STARTED        0x01U
#define AIRPS3_DIAG_EVENT_RECEIVER_READY 0x02U
#define AIRPS3_DIAG_EVENT_WORKER_STOPPED 0x04U
#define AIRPS3_DIAG_EVENT_MODULE_STOP    0x08U
#define AIRPS3_DIAG_EVENT_LOG_UNAVAILABLE 0x10U
#define AIRPS3_DIAG_EVENT_AUDIO_READY     0x20U
#define AIRPS3_DIAG_EVENT_AUDIO_FAILED   0x40U
#define AIRPS3_DIAG_EVENT_AUDIO_PCM_SUBMITTED 0x80U
#define AIRPS3_DIAG_EVENT_TEST_TONE_QUEUED 0x100U
#define AIRPS3_DIAG_EVENT_AUDIO_INIT_BORROWED 0x200U
#define AIRPS3_DIAG_EVENT_AUDIO_DRAINED  0x400U

#endif
