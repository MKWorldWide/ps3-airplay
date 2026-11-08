#if !defined(RAOP_USE_PSL1GHT) && !defined(_POSIX_C_SOURCE)
#define _POSIX_C_SOURCE 200809L
#endif

#include "receiver_log.h"

#include <string.h>
#include <time.h>

#if defined(RAOP_USE_PSL1GHT)
#include <sys/thread.h>
#include <lv2/thread.h>
#include <lv2/systime.h>
#include <unistd.h>
#endif

static int safe_field(const char *text, size_t max_length)
{
	size_t i;
	if (text == NULL || text[0] == '\0' || strlen(text) > max_length)
		return 0;
	for (i = 0; text[i] != '\0'; ++i) {
		unsigned char c = (unsigned char)text[i];
		if (!((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') ||
			(c >= '0' && c <= '9') || c == '-' || c == '_' || c == ':' ||
			c == 'T' || c == 'Z' || c == '+' || c == '.'))
			return 0;
	}
	return 1;
}

static uint64_t log_clock_us(void)
{
#if defined(RAOP_USE_PSL1GHT)
	return sysGetSystemTime();
#else
	struct timespec now;
	if (clock_gettime(CLOCK_MONOTONIC, &now) != 0)
		return 0;
	return (uint64_t)now.tv_sec * 1000000U +
		(uint64_t)now.tv_nsec / 1000U;
#endif
}

static void update_max(volatile uint32_t *maximum, uint64_t elapsed)
{
	uint32_t value = elapsed > UINT32_MAX ? UINT32_MAX : (uint32_t)elapsed;
	if (value > *maximum)
		*maximum = value;
}

static int format_line(char line[RECEIVER_LOG_LINE_MAX],
	const char *timestamp, const char *event, int value, size_t byte_count)
{
	int length;
	if (!safe_field(timestamp, 40) ||
		!safe_field(event, RECEIVER_LOG_EVENT_MAX))
		return 0;
	length = snprintf(line, RECEIVER_LOG_LINE_MAX,
		"%s event=%s value=%d bytes=%lu\n", timestamp, event, value,
		(unsigned long)byte_count);
	return length > 0 && (size_t)length < RECEIVER_LOG_LINE_MAX ? length : 0;
}

static int write_direct(receiver_log_file *log, const char *timestamp,
	const char *line, size_t length)
{
	uint64_t started;
	if (log->bytes_written + length > RECEIVER_LOG_MAX_BYTES - 160U) {
		char marker[RECEIVER_LOG_LINE_MAX];
		int marker_length = snprintf(marker, sizeof(marker),
			"%s event=LOG_SESSION_CAPPED value=%lu bytes=0\n", timestamp,
			(unsigned long)log->bytes_written);
		if (marker_length > 0 && (size_t)marker_length < sizeof(marker) &&
			log->bytes_written + (size_t)marker_length <= RECEIVER_LOG_MAX_BYTES) {
			started = log_clock_us();
			if (fwrite(marker, 1, (size_t)marker_length, log->stream) ==
				(size_t)marker_length) {
				update_max(&log->write_max_us, log_clock_us() - started);
				started = log_clock_us();
				if (fflush(log->stream) == 0) {
					update_max(&log->flush_max_us, log_clock_us() - started);
					log->bytes_written += (size_t)marker_length;
				}
			}
		}
		log->capped = 1;
		return 0;
	}
	started = log_clock_us();
	if (fwrite(line, 1, length, log->stream) != length)
		return 0;
	update_max(&log->write_max_us, log_clock_us() - started);
	started = log_clock_us();
	if (fflush(log->stream) != 0)
		return 0;
	update_max(&log->flush_max_us, log_clock_us() - started);
	log->bytes_written += length;
	return 1;
}

#if defined(RAOP_USE_PSL1GHT)
static void log_writer(void *argument)
{
	receiver_log_file *log = (receiver_log_file *)argument;
	for (;;) {
		unsigned int batch = 0;
		while (batch < 32U) {
			uint32_t read_index = log->queue_read_index;
			uint32_t write_index;
			receiver_log_record *record;
			uint64_t started;
			__sync_synchronize();
			write_index = log->queue_write_index;
			if (read_index == write_index)
				break;
			record = &log->queue[read_index % RECEIVER_LOG_QUEUE_CAPACITY];
			started = log_clock_us();
			if (fwrite(record->line, 1, record->length, log->stream) !=
				record->length) {
				log->writer_failed = 1;
				log->queue_read_index = write_index;
				break;
			}
			update_max(&log->write_max_us, log_clock_us() - started);
			log->bytes_written += record->length;
			__sync_synchronize();
			log->queue_read_index = read_index + 1U;
			++batch;
		}
		if (batch != 0 && !log->writer_failed) {
			uint64_t started = log_clock_us();
			if (fflush(log->stream) != 0)
				log->writer_failed = 1;
			else
				update_max(&log->flush_max_us, log_clock_us() - started);
		}
		if (log->worker_stop &&
			log->queue_read_index == log->queue_write_index)
			break;
		if (batch == 0)
			usleep(10000U);
	}
	log->worker_done = 1;
	__sync_synchronize();
	sysThreadExit(0);
}
#endif

int receiver_log_event(FILE *stream, const char *timestamp,
	const char *event, int value, size_t byte_count)
{
	char line[RECEIVER_LOG_LINE_MAX];
	int length;
	if (stream == NULL)
		return 0;
	length = format_line(line, timestamp, event, value, byte_count);
	return length > 0 && fprintf(stream, "%s", line) >= 0;
}

int receiver_log_file_open(receiver_log_file *log, const char *path)
{
	size_t path_length;
	if (log == NULL || path == NULL)
		return 0;
	path_length = strlen(path);
	if (path_length == 0 || path_length >= sizeof(log->path))
		return 0;
	memset(log, 0, sizeof(*log));
	memcpy(log->path, path, path_length + 1);
	log->stream = fopen(log->path, "ab");
	return log->stream != NULL;
}

int receiver_log_file_start_worker(receiver_log_file *log)
{
#if defined(RAOP_USE_PSL1GHT)
	sys_ppu_thread_t thread_id = 0;
	int result;
	if (log == NULL || log->stream == NULL || log->worker_running)
		return 0;
	result = sysThreadCreate(&thread_id, log_writer, log, 3071, 65536,
		THREAD_JOINABLE, "air-ps3-log");
	if (result != 0)
		return 0;
	log->worker_thread = (uint64_t)thread_id;
	log->worker_running = 1;
	return 1;
#else
	(void)log;
	return 0;
#endif
}

void receiver_log_file_get_stats(const receiver_log_file *log,
	receiver_log_stats *stats)
{
	uint32_t write_index, read_index;
	if (log == NULL || stats == NULL) return;
	write_index = log->queue_write_index;
	read_index = log->queue_read_index;
	stats->queued_events = write_index - read_index;
	stats->queue_overruns = log->queue_overruns;
	stats->write_max_us = log->write_max_us;
	stats->flush_max_us = log->flush_max_us;
	stats->writer_failed = log->writer_failed != 0;
}

void receiver_log_file_close(receiver_log_file *log)
{
	if (log == NULL)
		return;
#if defined(RAOP_USE_PSL1GHT)
	if (log->worker_running) {
		uint64_t deadline = log_clock_us() + 500000U;
		log->worker_stop = 1;
		__sync_synchronize();
		while (!log->worker_done && log_clock_us() < deadline)
			usleep(1000U);
		if (!log->worker_done)
			return; /* Avoid racing fclose against a stalled filesystem writer. */
		(void)sysThreadJoin((sys_ppu_thread_t)log->worker_thread, NULL);
		log->worker_running = 0;
	}
#endif
	if (log->stream != NULL)
		fclose(log->stream);
	log->stream = NULL;
	log->bytes_written = 0;
	log->capped = 0;
}

int receiver_log_file_event(receiver_log_file *log, const char *timestamp,
	const char *event, int value, size_t byte_count)
{
	char line[RECEIVER_LOG_LINE_MAX];
	int length;
	if (log == NULL || log->stream == NULL || log->capped ||
		log->writer_failed)
		return 0;
	length = format_line(line, timestamp, event, value, byte_count);
	if (length <= 0)
		return 0;
#if defined(RAOP_USE_PSL1GHT)
	if (log->worker_running) {
		uint32_t write_index = log->queue_write_index;
		uint32_t read_index = log->queue_read_index;
		__sync_synchronize();
		if (log->reserved_bytes + (size_t)length > RECEIVER_LOG_MAX_BYTES - 160U) {
			length = snprintf(line, sizeof(line),
				"%s event=LOG_SESSION_CAPPED value=%lu bytes=0\n", timestamp,
				(unsigned long)log->reserved_bytes);
			if (length <= 0 || (size_t)length >= sizeof(line) ||
				write_index - read_index >= RECEIVER_LOG_QUEUE_CAPACITY) {
				log->capped = 1;
				return 0;
			}
			log->capped = 1;
		} else if (write_index - read_index >= RECEIVER_LOG_QUEUE_CAPACITY) {
			++log->queue_overruns;
			return 0;
		}
		memcpy(log->queue[write_index % RECEIVER_LOG_QUEUE_CAPACITY].line,
			line, (size_t)length);
		log->queue[write_index % RECEIVER_LOG_QUEUE_CAPACITY].length =
			(uint16_t)length;
		log->reserved_bytes += (size_t)length;
		__sync_synchronize();
		log->queue_write_index = write_index + 1U;
		return 1;
	}
#endif
	return write_direct(log, timestamp, line, (size_t)length);
}
