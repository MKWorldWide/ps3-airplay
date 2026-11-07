#include "rtsp_diagnostic.h"

#include <stdio.h>
#include <string.h>

static int copy_token(char *out, size_t capacity, const char *start, size_t length)
{
	size_t i;
	if (length == 0 || length >= capacity)
		return 0;
	for (i = 0; i < length; ++i) {
		unsigned char c = (unsigned char)start[i];
		if (c <= 0x20 || c >= 0x7f || c == '\r' || c == '\n')
			return 0;
		out[i] = (char)c;
	}
	out[length] = '\0';
	return 1;
}

static int parse_request_line(const char *line, size_t length,
	char *method, size_t method_capacity)
{
	const char *space1 = memchr(line, ' ', length);
	const char *space2;
	if (space1 == NULL)
		return 0;
	if (space1 + 1 >= line + length || space1[1] == ' ')
		return 0;
	space2 = memchr(space1 + 1, ' ', length - (size_t)(space1 + 1 - line));
	if (space2 == NULL || (size_t)(line + length - (space2 + 1)) != 8 ||
		memcmp(space2 + 1, "RTSP/1.0", 8) != 0)
		return 0;
	return copy_token(method, method_capacity, line, (size_t)(space1 - line));
}

int rtsp_diag_handle(const char *request, size_t request_length,
	char *response, size_t response_capacity, rtsp_diag_result *result)
{
	const char *end;
	const char *line_end;
	const char *header_end;
	const char *cursor;
	int have_cseq = 0;
	int parse_ok;
	int status;
	int written;

	if (request == NULL || response == NULL || result == NULL ||
		request_length == 0 || request_length > RTSP_DIAG_REQUEST_MAX ||
		response_capacity == 0)
		return 0;

	memset(result, 0, sizeof(*result));
	result->status = 400;
	end = request + request_length;
	line_end = NULL;
	header_end = NULL;
	for (cursor = request; cursor + 1 < end; ++cursor) {
		if (cursor + 3 < end && cursor[0] == '\r' && cursor[1] == '\n' &&
			cursor[2] == '\r' && cursor[3] == '\n') {
			header_end = cursor + 2;
			break;
		}
	}
	if (header_end != NULL) {
		for (cursor = request; cursor + 1 < header_end; ++cursor) {
			if (cursor[0] == '\r' && cursor[1] == '\n') {
				line_end = cursor;
				break;
			}
		}
	}
	parse_ok = header_end != NULL && line_end != NULL && parse_request_line(request,
		(size_t)(line_end - request), result->method, sizeof(result->method));

	if (parse_ok) {
		cursor = line_end + 2;
		while (cursor < header_end) {
			const char *header_end = NULL;
			const char *colon;
			const char *value;
			const char *value_end;
			const char *scan;
			for (scan = cursor; scan + 1 < end; ++scan) {
				if (scan[0] == '\r' && scan[1] == '\n') {
					header_end = scan;
					break;
				}
			}
			if (header_end == NULL)
				break;
			colon = memchr(cursor, ':', (size_t)(header_end - cursor));
			if (colon != NULL && (size_t)(colon - cursor) == 4 &&
				memcmp(cursor, "CSeq", 4) == 0) {
				value = colon + 1;
				while (value < header_end && (*value == ' ' || *value == '\t'))
					++value;
				value_end = header_end;
				while (value_end > value &&
					(value_end[-1] == ' ' || value_end[-1] == '\t'))
					--value_end;
				if (have_cseq || !copy_token(result->cseq,
					sizeof(result->cseq), value, (size_t)(value_end - value)))
					parse_ok = 0;
				have_cseq = 1;
			}
			cursor = header_end + 2;
		}
	}

	if (!have_cseq)
		parse_ok = 0;
	status = !parse_ok ? 400 :
		(strcmp(result->method, "OPTIONS") == 0 ? 200 : 501);
	result->status = status;
	if (status == 200) {
		written = snprintf(response, response_capacity,
			"RTSP/1.0 200 OK\r\nCSeq: %s\r\nPublic: OPTIONS\r\n"
			"Content-Length: 0\r\n\r\n", result->cseq);
	} else if (have_cseq) {
		written = snprintf(response, response_capacity,
			"RTSP/1.0 %d %s\r\nCSeq: %s\r\nContent-Length: 0\r\n\r\n",
			status, status == 400 ? "Bad Request" : "Not Implemented",
			result->cseq);
	} else {
		written = snprintf(response, response_capacity,
			"RTSP/1.0 400 Bad Request\r\nContent-Length: 0\r\n\r\n");
	}
	return written > 0 && (size_t)written < response_capacity;
}
