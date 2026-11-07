#include "../../src/protocol/raop_session.h"

#include <assert.h>
#include <stdio.h>
#include <string.h>

static void request(raop_session *session, const char *message,
	int expected_status, const char *expected_text)
{
	char response[RAOP_SESSION_RESPONSE_MAX];
	if (!raop_session_handle(session, message, strlen(message), response,
		sizeof(response))) {
		fprintf(stderr, "no RTSP response for request: %.32s\n", message);
		assert(0);
	}
	assert(strstr(response, expected_text) != NULL);
	if (expected_status != 0) {
		char status[32];
		(void)snprintf(status, sizeof(status), "RTSP/1.0 %d ", expected_status);
		assert(strstr(response, status) != NULL);
	}
}

int main(void)
{
	raop_session session;
	char response[RAOP_SESSION_RESPONSE_MAX];
	const char *announce =
		"ANNOUNCE rtsp://ps3/stream RTSP/1.0\r\n"
		"CSeq: 2\r\nContent-Type: application/sdp\r\n"
		"Content-Length: 25\r\n\r\n"
		"v=0\r\nm=audio 0 RTP/AVP 96";
	raop_session_init(&session, 6000, 6001, 6002);

	request(&session, "OPTIONS * RTSP/1.0\r\nCSeq: 1\r\n\r\n",
		200, "Public: OPTIONS, ANNOUNCE");
	assert(session.state == RAOP_SESSION_IDLE);
	request(&session, announce, 200, "CSeq: 2");
	assert(session.state == RAOP_SESSION_ANNOUNCED);
	request(&session,
		"SETUP rtsp://ps3/stream RTSP/1.0\r\nCSeq: 3\r\n"
		"Transport: RTP/AVP/UDP;unicast;mode=record;control_port=5001;"
		"timing_port=5002\r\n\r\n",
		200, "server_port=6000;control_port=6001;timing_port=6002");
	assert(session.state == RAOP_SESSION_SETUP);
	assert(session.peer_control_port == 5001);
	assert(session.peer_timing_port == 5002);
	request(&session,
		"RECORD rtsp://ps3/stream RTSP/1.0\r\nCSeq: 4\r\n\r\n",
		200, "Audio-Latency: 11025");
	assert(session.state == RAOP_SESSION_RECORDING);
	request(&session,
		"GET_PARAMETER rtsp://ps3/stream RTSP/1.0\r\nCSeq: 5\r\n\r\n",
		200, "Session: PS3AIRPLAY000001");
	request(&session,
		"FLUSH rtsp://ps3/stream RTSP/1.0\r\nCSeq: 6\r\n\r\n",
		200, "CSeq: 6");
	request(&session,
		"TEARDOWN rtsp://ps3/stream RTSP/1.0\r\nCSeq: 7\r\n\r\n",
		200, "Session: PS3AIRPLAY000001");
	assert(session.state == RAOP_SESSION_IDLE);
	assert(session.requests == 7);

	raop_session_init(&session, 6000, 6001, 6002);
	request(&session, announce, 200, "CSeq: 2");
	request(&session,
		"SETUP rtsp://ps3/stream RTSP/1.0\r\nCSeq: 3\r\n"
		"Transport: RTP/AVP/UDP;unicast;mode=record\r\n\r\n",
		461, "Unsupported Transport");
	assert(session.state == RAOP_SESSION_ANNOUNCED);

	raop_session_init(&session, 6000, 6001, 6002);
	request(&session,
		"RECORD rtsp://ps3/stream RTSP/1.0\r\nCSeq: 1\r\n\r\n",
		455, "Method Not Valid in This State");
	assert(session.state == RAOP_SESSION_IDLE);
	assert(raop_session_handle(&session,
		"OPTIONS * RTSP/1.0\r\n\r\n",
		sizeof("OPTIONS * RTSP/1.0\r\n\r\n") - 1, response,
		sizeof(response)) == 0);
	puts("RAOP RTSP session transitions: PASS");
	return 0;
}
