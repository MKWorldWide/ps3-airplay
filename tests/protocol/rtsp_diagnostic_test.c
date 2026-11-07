#include "../../src/protocol/rtsp_diagnostic.h"
#include "../../src/observability/receiver_log.h"

#include <assert.h>
#include <stdio.h>
#include <string.h>

int main(void)
{
	char response[512];
	rtsp_diag_result result;
	static const char options[] =
		"OPTIONS * RTSP/1.0\r\nCSeq: 7\r\nUser-Agent: test\r\n\r\n";
	static const char unsupported[] =
		"SETUP rtsp://receiver/stream RTSP/1.0\r\nCSeq: 8\r\n\r\n";
	static const char duplicate[] =
		"OPTIONS * RTSP/1.0\r\nCSeq: 1\r\nCSeq: 2\r\n\r\n";
	static const char incomplete[] = "OPTIONS * RTSP/1.0\r\nCSeq: 9\r\n";
	static const char empty_uri[] = "OPTIONS  RTSP/1.0\r\nCSeq: 9\r\n\r\n";
	FILE *log = tmpfile();
	char log_line[160];

	assert(rtsp_diag_handle(options, sizeof(options) - 1, response,
		sizeof(response), &result));
	assert(result.status == 200 && strcmp(result.method, "OPTIONS") == 0);
	assert(strstr(response, "CSeq: 7\r\n") != NULL);
	assert(strstr(response, "Public: OPTIONS\r\n") != NULL);
	assert(rtsp_diag_handle(unsupported, sizeof(unsupported) - 1, response,
		sizeof(response), &result));
	assert(result.status == 501 && strcmp(result.method, "SETUP") == 0);
	assert(strstr(response, "501 Not Implemented") != NULL);
	assert(rtsp_diag_handle(duplicate, sizeof(duplicate) - 1, response,
		sizeof(response), &result));
	assert(result.status == 400);
	assert(rtsp_diag_handle(incomplete, sizeof(incomplete) - 1, response,
		sizeof(response), &result));
	assert(result.status == 400);
	assert(rtsp_diag_handle(empty_uri, sizeof(empty_uri) - 1, response,
		sizeof(response), &result));
	assert(result.status == 400);
	assert(!rtsp_diag_handle(options, RTSP_DIAG_REQUEST_MAX + 1,
		response, sizeof(response), &result));
	assert(log != NULL);
	assert(receiver_log_event(log, "2026-11-06T23:00:00Z", "rtsp_request",
		result.status, sizeof(options) - 1));
	assert(!receiver_log_event(log, "bad\nfield", "rtsp_request", 200, 4));
	rewind(log);
	assert(fgets(log_line, sizeof(log_line), log) != NULL);
	assert(strstr(log_line, "event=rtsp_request value=400") != NULL);
	assert(strchr(log_line, '\n') == strrchr(log_line, '\n'));
	fclose(log);
	puts("RTSP diagnostic parser and privacy-safe event log validated");
	return 0;
}
