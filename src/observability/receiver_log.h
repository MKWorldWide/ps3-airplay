#ifndef PS3_AIRPLAY_RECEIVER_LOG_H
#define PS3_AIRPLAY_RECEIVER_LOG_H

#include <stddef.h>
#include <stdint.h>
#include <stdio.h>

#define RECEIVER_LOG_MAX_BYTES (5U * 1024U * 1024U)
#define RECEIVER_LOG_PATH_MAX 256
#define RECEIVER_LOG_EVENT_MAX 48
#define RECEIVER_LOG_QUEUE_CAPACITY 128U
#define RECEIVER_LOG_LINE_MAX 160U

typedef struct {
	char line[RECEIVER_LOG_LINE_MAX];
	uint16_t length;
} receiver_log_record;

typedef struct {
	FILE *stream;
	char path[RECEIVER_LOG_PATH_MAX];
	size_t bytes_written;
	size_t reserved_bytes;
	int capped;
	volatile uint32_t queue_write_index;
	volatile uint32_t queue_read_index;
	volatile uint32_t queue_overruns;
	volatile uint32_t write_max_us;
	volatile uint32_t flush_max_us;
	volatile int worker_stop;
	volatile int worker_done;
	volatile int writer_failed;
	int worker_running;
	uint64_t worker_thread;
	receiver_log_record queue[RECEIVER_LOG_QUEUE_CAPACITY];
} receiver_log_file;

typedef struct {
	uint32_t queued_events;
	uint32_t queue_overruns;
	uint32_t write_max_us;
	uint32_t flush_max_us;
	uint32_t writer_failed;
} receiver_log_stats;

/* Events deliberately exclude IPs, headers, bodies, track names, and keys. */
int receiver_log_event(FILE *stream, const char *timestamp,
	const char *event, int value, size_t byte_count);

/* Append-only file; the fixed byte cap applies independently to each launch. */
int receiver_log_file_open(receiver_log_file *log, const char *path);
/* Uses a low-priority PS3 PPU writer and a fixed SPSC queue. Host builds use
 * synchronous output and return 0 because no platform worker is available. */
int receiver_log_file_start_worker(receiver_log_file *log);
void receiver_log_file_get_stats(const receiver_log_file *log,
	receiver_log_stats *stats);
void receiver_log_file_close(receiver_log_file *log);
int receiver_log_file_event(receiver_log_file *log, const char *timestamp,
	const char *event, int value, size_t byte_count);

#endif
