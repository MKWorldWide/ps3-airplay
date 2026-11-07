#include "receiver_log.h"

#include <string.h>

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

int receiver_log_event(FILE *stream, const char *timestamp,
	const char *event, int value, size_t byte_count)
{
	if (stream == NULL || !safe_field(timestamp, 40) ||
		!safe_field(event, RECEIVER_LOG_EVENT_MAX))
		return 0;
	return fprintf(stream, "%s event=%s value=%d bytes=%lu\n", timestamp,
		event, value, (unsigned long)byte_count) >= 0;
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
	log->stream = fopen(log->path, "wb");
	return log->stream != NULL;
}

void receiver_log_file_close(receiver_log_file *log)
{
	if (log == NULL)
		return;
	if (log->stream != NULL)
		fclose(log->stream);
	log->stream = NULL;
	log->bytes_written = 0;
}

int receiver_log_file_event(receiver_log_file *log, const char *timestamp,
	const char *event, int value, size_t byte_count)
{
	char line[160];
	int length;
	if (log == NULL || log->stream == NULL ||
		!safe_field(timestamp, 40) ||
		!safe_field(event, RECEIVER_LOG_EVENT_MAX))
		return 0;
	length = snprintf(line, sizeof(line), "%s event=%s value=%d bytes=%lu\n",
		timestamp, event, value, (unsigned long)byte_count);
	if (length <= 0 || (size_t)length >= sizeof(line) ||
		(size_t)length > RECEIVER_LOG_MAX_BYTES)
		return 0;
	if (log->bytes_written + (size_t)length > RECEIVER_LOG_MAX_BYTES) {
		FILE *old_stream = log->stream;
		size_t old_bytes = log->bytes_written;
		log->stream = NULL;
		if (fclose(old_stream) != 0)
			return 0;
		log->stream = fopen(log->path, "wb");
		log->bytes_written = 0;
		if (log->stream == NULL)
			return 0;
		length = snprintf(line, sizeof(line),
			"%s event=LOG_WRAPPED value=%lu bytes=0\n", timestamp,
			(unsigned long)old_bytes);
		if (length <= 0 || (size_t)length >= sizeof(line) ||
			fwrite(line, 1, (size_t)length, log->stream) != (size_t)length ||
			fflush(log->stream) != 0)
			return 0;
		log->bytes_written = (size_t)length;
		length = snprintf(line, sizeof(line),
			"%s event=%s value=%d bytes=%lu\n", timestamp, event, value,
			(unsigned long)byte_count);
		if (length <= 0 || (size_t)length >= sizeof(line) ||
			log->bytes_written + (size_t)length > RECEIVER_LOG_MAX_BYTES)
			return 0;
	}
	if (fwrite(line, 1, (size_t)length, log->stream) != (size_t)length ||
		fflush(log->stream) != 0)
		return 0;
	log->bytes_written += (size_t)length;
	return 1;
}
