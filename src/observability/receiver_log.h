#ifndef PS3_AIRPLAY_RECEIVER_LOG_H
#define PS3_AIRPLAY_RECEIVER_LOG_H

#include <stddef.h>
#include <stdio.h>

#define RECEIVER_LOG_MAX_BYTES 16384
#define RECEIVER_LOG_PATH_MAX 256
#define RECEIVER_LOG_EVENT_MAX 48

typedef struct {
	FILE *stream;
	char path[RECEIVER_LOG_PATH_MAX];
	size_t bytes_written;
	int capped;
} receiver_log_file;

/* Events deliberately exclude IPs, headers, bodies, track names, and keys. */
int receiver_log_event(FILE *stream, const char *timestamp,
	const char *event, int value, size_t byte_count);

/* Append-only file; the fixed byte cap applies independently to each launch. */
int receiver_log_file_open(receiver_log_file *log, const char *path);
void receiver_log_file_close(receiver_log_file *log);
int receiver_log_file_event(receiver_log_file *log, const char *timestamp,
	const char *event, int value, size_t byte_count);

#endif
