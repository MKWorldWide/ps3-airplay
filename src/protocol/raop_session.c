#include "raop_session.h"

#include <math.h>
#include <stdio.h>
#include <string.h>

#include <mbedtls/platform_util.h>

typedef struct {
	char method[20];
	char cseq[12];
	char content_type[48];
	char transport[256];
	char apple_challenge[32];
	size_t apple_challenge_length;
	size_t body_offset;
	size_t body_length;
	int have_cseq;
	int have_content_length;
	int have_apple_challenge;
} request_fields;

static int ascii_equal(const char *left, size_t left_length,
	const char *right)
{
	size_t i;
	if (strlen(right) != left_length)
		return 0;
	for (i = 0; i < left_length; ++i) {
		char a = left[i];
		char b = right[i];
		if (a >= 'A' && a <= 'Z') a = (char)(a + ('a' - 'A'));
		if (b >= 'A' && b <= 'Z') b = (char)(b + ('a' - 'A'));
		if (a != b)
			return 0;
	}
	return 1;
}

static int copy_value(char *out, size_t capacity,
	const char *start, size_t length)
{
	size_t i;
	while (length > 0 && (*start == ' ' || *start == '\t')) {
		++start;
		--length;
	}
	while (length > 0 && (start[length - 1] == ' ' || start[length - 1] == '\t'))
		--length;
	if (length == 0 || length >= capacity)
		return 0;
	for (i = 0; i < length; ++i) {
		unsigned char c = (unsigned char)start[i];
		if (c < 0x20 || c >= 0x7f)
			return 0;
		out[i] = (char)c;
	}
	out[length] = '\0';
	return 1;
}

static int parse_decimal(const char *value, size_t length, size_t maximum,
	size_t *number)
{
	size_t result = 0;
	size_t i;
	while (length > 0 && (*value == ' ' || *value == '\t')) {
		++value;
		--length;
	}
	while (length > 0 && (value[length - 1] == ' ' || value[length - 1] == '\t'))
		--length;
	if (length == 0)
		return 0;
	for (i = 0; i < length; ++i) {
		unsigned char c = (unsigned char)value[i];
		if (c < '0' || c > '9' || result > (maximum - (size_t)(c - '0')) / 10)
			return 0;
		result = result * 10 + (size_t)(c - '0');
	}
	*number = result;
	return 1;
}

static int parse_volume_millidb(const char *value, size_t length,
	int32_t *volume_millidb)
{
	uint32_t whole = 0;
	uint32_t fraction = 0;
	uint32_t fraction_digits = 0;
	uint32_t magnitude;
	size_t i = 0;
	int negative = 0;
	int have_whole = 0;
	int have_fraction = 0;
	while (i < length && (value[i] == ' ' || value[i] == '\t'))
		++i;
	if (i < length && (value[i] == '-' || value[i] == '+')) {
		negative = value[i] == '-';
		++i;
	}
	while (i < length && value[i] >= '0' && value[i] <= '9') {
		have_whole = 1;
		if (whole < 1000000U)
			whole = whole * 10U + (uint32_t)(value[i] - '0');
		++i;
	}
	if (i < length && value[i] == '.') {
		++i;
		while (i < length && value[i] >= '0' && value[i] <= '9') {
			have_fraction = 1;
			if (fraction_digits < 3U) {
				fraction = fraction * 10U +
					(uint32_t)(value[i] - '0');
				++fraction_digits;
			}
			++i;
		}
		if (!have_fraction)
			return 0;
	}
	if (!have_whole && !have_fraction)
		return 0;
	while (i < length && (value[i] == ' ' || value[i] == '\t'))
		++i;
	if (i != length)
		return 0;
	while (fraction_digits < 3U) {
		fraction *= 10U;
		++fraction_digits;
	}
	if (whole > 1000000U)
		whole = 1000000U;
	magnitude = whole * 1000U + fraction;
	if (magnitude > 1000000000U)
		magnitude = 1000000000U;
	*volume_millidb = negative ? -(int32_t)magnitude : (int32_t)magnitude;
	return 1;
}

static int parse_parameter_volume(const request_fields *fields,
	const char *request, int32_t *volume_millidb, int *has_volume)
{
	size_t cursor;
	size_t end;
	*has_volume = 0;
	if (!ascii_equal(fields->content_type, strlen(fields->content_type),
		"text/parameters"))
		return 1;
	cursor = fields->body_offset;
	end = cursor + fields->body_length;
	while (cursor < end) {
		size_t line_start = cursor;
		size_t line_end;
		size_t colon;
		while (cursor < end && request[cursor] != '\r' &&
			request[cursor] != '\n')
			++cursor;
		line_end = cursor;
		while (cursor < end && (request[cursor] == '\r' ||
			request[cursor] == '\n'))
			++cursor;
		while (line_start < line_end && (request[line_start] == ' ' ||
			request[line_start] == '\t'))
			++line_start;
		while (line_end > line_start && (request[line_end - 1] == ' ' ||
			request[line_end - 1] == '\t'))
			--line_end;
		for (colon = line_start; colon < line_end && request[colon] != ':';
			++colon)
			;
		if (colon < line_end && ascii_equal(request + line_start,
			colon - line_start, "volume")) {
			if (*has_volume || !parse_volume_millidb(request + colon + 1,
				line_end - colon - 1, volume_millidb))
				return 0;
			*has_volume = 1;
		}
	}
	return 1;
}

static void set_volume(raop_session *session, int32_t requested_millidb)
{
	int32_t applied_millidb = requested_millidb;
	if (requested_millidb <= -144000) {
		session->volume_millidb = -144000;
		session->volume_gain = 0.0f;
		session->volume_muted = 1;
	} else {
		if (applied_millidb < -30000)
			applied_millidb = -30000;
		if (applied_millidb > 0)
			applied_millidb = 0;
		session->volume_millidb = applied_millidb;
		session->volume_gain = powf(10.0f,
			(float)applied_millidb / 20000.0f);
		session->volume_muted = 0;
	}
	++session->volume_updates;
}

static int parse_transport_port(const char *transport, const char *name,
	uint16_t *port)
{
	const char *cursor = transport;
	size_t name_length = strlen(name);
	int found = 0;
	while (*cursor != '\0') {
		const char *item;
		const char *end;
		const char *equals;
		while (*cursor == ';' || *cursor == ' ' || *cursor == '\t')
			++cursor;
		item = cursor;
		while (*cursor != '\0' && *cursor != ';')
			++cursor;
		end = cursor;
		equals = memchr(item, '=', (size_t)(end - item));
		if (equals != NULL && (size_t)(equals - item) == name_length &&
			ascii_equal(item, name_length, name)) {
			size_t value;
			if (found || !parse_decimal(equals + 1,
				(size_t)(end - (equals + 1)), 65535, &value) || value == 0)
				return 0;
			*port = (uint16_t)value;
			found = 1;
		}
	}
	return found;
}

static const char *find_crlf(const char *start, const char *end)
{
	const char *cursor;
	for (cursor = start; cursor + 1 < end; ++cursor)
		if (cursor[0] == '\r' && cursor[1] == '\n')
			return cursor;
	return NULL;
}

static int parse_request(const char *request, size_t length,
	request_fields *fields)
{
	const char *end = request + length;
	const char *header_end = NULL;
	const char *line_end;
	const char *cursor;
	const char *space1;
	const char *space2;
	size_t value;

	memset(fields, 0, sizeof(*fields));
	for (cursor = request; cursor + 3 < end; ++cursor) {
		if (memcmp(cursor, "\r\n\r\n", 4) == 0) {
			header_end = cursor;
			break;
		}
	}
	if (header_end == NULL)
		return 0;
	line_end = find_crlf(request, header_end + 2);
	if (line_end == NULL)
		return 0;
	space1 = memchr(request, ' ', (size_t)(line_end - request));
	if (space1 == NULL)
		return 0;
	space2 = memchr(space1 + 1, ' ', (size_t)(line_end - (space1 + 1)));
	if (space2 == NULL || (size_t)(line_end - (space2 + 1)) != 8 ||
		memcmp(space2 + 1, "RTSP/1.0", 8) != 0 ||
		!copy_value(fields->method, sizeof(fields->method), request,
			(size_t)(space1 - request)))
		return 0;

	cursor = line_end + 2;
	while (cursor < header_end) {
		const char *next = find_crlf(cursor, header_end + 2);
		const char *colon;
		const char *value_start;
		size_t value_length;
		if (next == NULL)
			return 0;
		if (next == cursor)
			break;
		colon = memchr(cursor, ':', (size_t)(next - cursor));
		if (colon == NULL)
			return 0;
		value_start = colon + 1;
		value_length = (size_t)(next - value_start);
		if (ascii_equal(cursor, (size_t)(colon - cursor), "CSeq")) {
			if (fields->have_cseq || !copy_value(fields->cseq,
				sizeof(fields->cseq), value_start, value_length) ||
				!parse_decimal(fields->cseq, strlen(fields->cseq), 2147483647U,
					&value))
				return 0;
			fields->have_cseq = 1;
		} else if (ascii_equal(cursor, (size_t)(colon - cursor),
			"Content-Length")) {
			if (fields->have_content_length ||
				!parse_decimal(value_start, value_length, RAOP_SESSION_SDP_MAX,
					&fields->body_length))
				return 0;
			fields->have_content_length = 1;
		} else if (ascii_equal(cursor, (size_t)(colon - cursor),
			"Content-Type")) {
			if (!copy_value(fields->content_type, sizeof(fields->content_type),
				value_start, value_length))
				return 0;
		} else if (ascii_equal(cursor, (size_t)(colon - cursor), "Transport")) {
			if (!copy_value(fields->transport, sizeof(fields->transport),
				value_start, value_length))
				return 0;
		} else if (ascii_equal(cursor, (size_t)(colon - cursor),
			"Apple-Challenge")) {
			if (fields->have_apple_challenge || !copy_value(
				fields->apple_challenge, sizeof(fields->apple_challenge),
				value_start, value_length))
				return 0;
			fields->have_apple_challenge = 1;
			fields->apple_challenge_length = strlen(fields->apple_challenge);
		}
		cursor = next + 2;
	}
	if (!fields->have_cseq)
		return 0;
	fields->body_offset = (size_t)(header_end + 4 - request);
	if (fields->have_content_length) {
		if (fields->body_length != length - fields->body_offset)
			return 0;
	} else if (fields->body_offset != length) {
		return 0;
	}
	return 1;
}

void raop_session_init(raop_session *session, uint16_t audio_port,
	uint16_t control_port, uint16_t timing_port)
{
	if (session == NULL)
		return;
	memset(session, 0, sizeof(*session));
	session->audio_port = audio_port;
	session->control_port = control_port;
	session->timing_port = timing_port;
	session->volume_gain = 1.0f;
}

void raop_session_set_diagnostic(raop_session *session,
	raop_session_diagnostic_fn diagnostic, void *context)
{
	if (session == NULL)
		return;
	session->diagnostic = diagnostic;
	session->diagnostic_context = context;
}

static void session_diagnostic(raop_session *session, const char *event,
	int value, size_t bytes)
{
	if (session->diagnostic != NULL)
		session->diagnostic(session->diagnostic_context, event, value, bytes);
}

static int unwrap_announce_key(raop_session *session,
	const raop_sdp_offer *offer, uint8_t audio_iv[RAOP_AES_IV_SIZE],
	uint8_t audio_key[RAOP_AES_KEY_SIZE])
{
	int valid;
	if (!session->crypto_configured || !offer->has_rsa_aes_key ||
		!offer->has_aes_iv || !raop_crypto_base64_decode(offer->aes_iv,
			offer->aes_iv_length, audio_iv, RAOP_AES_IV_SIZE,
			RAOP_AES_IV_SIZE)) {
		session_diagnostic(session, "ANNOUNCE_CRYPTO_INPUT_INVALID", 0,
			offer->rsa_aes_key_length);
		return 0;
	}
	session_diagnostic(session, "ANNOUNCE_KEY_UNWRAP_START", 0,
		offer->rsa_aes_key_length);
	valid = raop_crypto_decrypt_session_key(session->crypto,
		offer->rsa_aes_key, offer->rsa_aes_key_length, session->rng,
		session->rng_context, audio_key);
	session_diagnostic(session, "ANNOUNCE_KEY_UNWRAP_DONE", valid,
		valid ? RAOP_AES_KEY_SIZE : 0);
	return valid;
}

int raop_session_configure_crypto(raop_session *session,
	raop_crypto_context *crypto, mbedtls_f_rng_t *rng, void *rng_context,
	const uint8_t local_ipv4[4], const uint8_t mac[6])
{
	if (session == NULL || crypto == NULL || !crypto->initialized || rng == NULL ||
		local_ipv4 == NULL || mac == NULL)
		return 0;
	session->crypto = crypto;
	session->rng = rng;
	session->rng_context = rng_context;
	memcpy(session->local_ipv4, local_ipv4, sizeof(session->local_ipv4));
	memcpy(session->mac, mac, sizeof(session->mac));
	session->crypto_configured = 1;
	return 1;
}

int raop_session_decrypt_audio(const raop_session *session,
	uint8_t *payload, size_t payload_length)
{
	if (session == NULL || payload == NULL || payload_length == 0)
		return 0;
	if (!session->has_audio_key)
		return 1;
	return raop_crypto_decrypt_audio(session->audio_key, session->audio_iv,
		payload, payload_length, payload, payload_length);
}

static int write_response(char *response, size_t capacity, int status,
	const char *reason, const request_fields *fields, const char *extra)
{
	int count = snprintf(response, capacity,
		"RTSP/1.0 %d %s\r\nCSeq: %s\r\nServer: AirTunes/105.1\r\n"
		"Audio-Jack-Status: connected; type=analog\r\n%s"
		"Content-Length: 0\r\n\r\n",
		status, reason, fields->cseq, extra == NULL ? "" : extra);
	return count > 0 && (size_t)count < capacity;
}

int raop_session_handle(raop_session *session, const char *request,
	size_t request_length, char *response, size_t response_capacity)
{
	request_fields fields;
	raop_session_state next_state;
	const char *extra = "";
	const char *reason = "OK";
	int status = 200;
	int count;
	int clear_offer = 0;
	int announce_encrypted = 0;
	int has_volume = 0;
	int32_t requested_volume_millidb = 0;
	raop_sdp_offer offer;
	uint8_t new_audio_key[RAOP_AES_KEY_SIZE] = { 0 };
	uint8_t new_audio_iv[RAOP_AES_IV_SIZE] = { 0 };
	uint8_t challenge[16];
	char apple_response[RAOP_APPLE_RESPONSE_MAX];
	char apple_header[RAOP_APPLE_RESPONSE_MAX + 24];
	char combined_extra[RAOP_SESSION_RESPONSE_MAX];
	const char *response_extra;

	if (session == NULL || request == NULL || response == NULL ||
		request_length == 0 || request_length > RAOP_SESSION_REQUEST_MAX ||
		response_capacity == 0 || !parse_request(request, request_length, &fields))
		return 0;
	next_state = session->state;
	apple_header[0] = '\0';
	if (fields.have_apple_challenge) {
		if (!session->crypto_configured) {
			status = 501;
			reason = "Not Implemented";
		} else if (!raop_crypto_decode_challenge(fields.apple_challenge,
			fields.apple_challenge_length, challenge)) {
			status = 400;
			reason = "Bad Request";
		} else if (!raop_crypto_apple_response(session->crypto, challenge,
			session->local_ipv4, session->mac, session->rng,
			session->rng_context, apple_response, sizeof(apple_response))) {
			status = 500;
			reason = "Internal Server Error";
		} else {
			count = snprintf(apple_header, sizeof(apple_header),
				"Apple-Response: %s\r\n", apple_response);
			if (count <= 0 || (size_t)count >= sizeof(apple_header)) {
				status = 500;
				reason = "Internal Server Error";
			}
		}
		mbedtls_platform_zeroize(challenge, sizeof(challenge));
		mbedtls_platform_zeroize(apple_response, sizeof(apple_response));
	}

	if (status != 200) {
		/* Challenge errors are answered below without advancing session state. */
	} else if (strcmp(fields.method, "OPTIONS") == 0) {
		extra = "Public: OPTIONS, ANNOUNCE, SETUP, RECORD, PAUSE, FLUSH, TEARDOWN, "
			"GET_PARAMETER, SET_PARAMETER\r\n";
	} else if (strcmp(fields.method, "ANNOUNCE") == 0) {
		const char *sdp = request + fields.body_offset;
		int offer_valid;
		session_diagnostic(session, "ANNOUNCE_SDP_PARSE_START", 0,
			fields.body_length);
		offer_valid = raop_sdp_parse(sdp, fields.body_length, &offer);
		session_diagnostic(session, "ANNOUNCE_SDP_PARSE_DONE",
			offer_valid, fields.body_length);
		session_diagnostic(session, "ANNOUNCE_VALIDATE_START",
			(int)session->state, offer_valid ? offer.rsa_aes_key_length : 0);
		session_diagnostic(session, "ANNOUNCE_OFFER_FIELDS",
			offer_valid ? (int)((offer.has_encryption_type ? 1U : 0U) |
				((unsigned int)offer.encryption_type << 1) |
				(offer.has_rsa_aes_key ? 1U << 9 : 0U) |
				(offer.has_aes_iv ? 1U << 10 : 0U)) : 0,
			offer_valid ? offer.rsa_aes_key_length : 0);
		if (session->state != RAOP_SESSION_IDLE &&
			session->state != RAOP_SESSION_ANNOUNCED) {
			status = 455;
			reason = "Method Not Valid in This State";
		} else if (fields.body_length == 0 ||
			fields.body_length > RAOP_SESSION_SDP_MAX ||
			!ascii_equal(fields.content_type, strlen(fields.content_type),
				"application/sdp")) {
			status = 415;
			reason = "Unsupported Media Type";
		} else if (!offer_valid) {
			status = 400;
			reason = "Bad Request";
		} else if (offer.has_encryption_type && offer.encryption_type == 1 &&
			!unwrap_announce_key(session, &offer, new_audio_iv,
				new_audio_key)) {
			status = 400;
			reason = "Bad Request";
		} else if (offer.has_encryption_type && offer.encryption_type != 0 &&
			offer.encryption_type != 1) {
			status = 415;
			reason = "Unsupported Media Type";
		} else {
			session->offer = offer;
			announce_encrypted = offer.has_encryption_type &&
				offer.encryption_type == 1;
			mbedtls_platform_zeroize(session->audio_key,
				sizeof(session->audio_key));
			mbedtls_platform_zeroize(session->audio_iv,
				sizeof(session->audio_iv));
			if (announce_encrypted) {
				memcpy(session->audio_key, new_audio_key,
					sizeof(session->audio_key));
				memcpy(session->audio_iv, new_audio_iv,
					sizeof(session->audio_iv));
				session->has_audio_key = 1;
			} else {
				session->has_audio_key = 0;
			}
			next_state = RAOP_SESSION_ANNOUNCED;
		}
		session_diagnostic(session, "ANNOUNCE_VALIDATE_DONE", status,
			(unsigned int)next_state);
	} else if (strcmp(fields.method, "SETUP") == 0) {
		uint16_t peer_control_port = 0;
		uint16_t peer_timing_port = 0;
		if (session->state != RAOP_SESSION_ANNOUNCED &&
			session->state != RAOP_SESSION_SETUP) {
			status = 455;
			reason = "Method Not Valid in This State";
		} else if (session->audio_port == 0 || session->control_port == 0 ||
			session->timing_port == 0 ||
			strstr(fields.transport, "RTP/AVP/UDP") == NULL ||
			strstr(fields.transport, "interleaved=") != NULL ||
			strstr(fields.transport, "unicast") == NULL ||
			!parse_transport_port(fields.transport, "control_port",
				&peer_control_port) ||
			!parse_transport_port(fields.transport, "timing_port",
				&peer_timing_port)) {
			status = 461;
			reason = "Unsupported Transport";
		} else {
			count = snprintf(response, response_capacity,
				"RTSP/1.0 200 OK\r\nCSeq: %s\r\n"
				"Server: AirTunes/105.1\r\n"
				"Audio-Jack-Status: connected; type=analog\r\n"
				"%sSession: PS3AIRPLAY000001\r\n"
				"Transport: RTP/AVP/UDP;unicast;mode=record;server_port=%u;"
				"control_port=%u;timing_port=%u\r\nContent-Length: 0\r\n\r\n",
				fields.cseq, apple_header, session->audio_port, session->control_port,
				session->timing_port);
			if (count <= 0 || (size_t)count >= response_capacity)
				return 0;
			session->peer_control_port = peer_control_port;
			session->peer_timing_port = peer_timing_port;
			next_state = RAOP_SESSION_SETUP;
		}
	} else if (strcmp(fields.method, "RECORD") == 0) {
		if (session->state != RAOP_SESSION_SETUP &&
			session->state != RAOP_SESSION_PAUSED) {
			status = 455;
			reason = "Method Not Valid in This State";
		} else {
			next_state = RAOP_SESSION_RECORDING;
			extra = "Session: PS3AIRPLAY000001\r\nAudio-Latency: 11025\r\n";
		}
	} else if (strcmp(fields.method, "PAUSE") == 0) {
		if (session->state != RAOP_SESSION_RECORDING) {
			status = 455;
			reason = "Method Not Valid in This State";
		} else {
			next_state = RAOP_SESSION_PAUSED;
			extra = "Session: PS3AIRPLAY000001\r\n";
		}
	} else if (strcmp(fields.method, "FLUSH") == 0) {
		if (session->state != RAOP_SESSION_RECORDING &&
			session->state != RAOP_SESSION_PAUSED) {
			status = 455;
			reason = "Method Not Valid in This State";
		} else {
			extra = "Session: PS3AIRPLAY000001\r\n";
		}
	} else if (strcmp(fields.method, "TEARDOWN") == 0) {
		if (session->state == RAOP_SESSION_IDLE) {
			status = 455;
			reason = "Method Not Valid in This State";
		} else {
			clear_offer = 1;
			next_state = RAOP_SESSION_IDLE;
			extra = "Session: PS3AIRPLAY000001\r\n";
		}
	} else if (strcmp(fields.method, "GET_PARAMETER") == 0 ||
		strcmp(fields.method, "SET_PARAMETER") == 0) {
		if (session->state == RAOP_SESSION_IDLE) {
			status = 455;
			reason = "Method Not Valid in This State";
		} else if (strcmp(fields.method, "SET_PARAMETER") == 0 &&
			!parse_parameter_volume(&fields, request,
				&requested_volume_millidb, &has_volume)) {
			status = 400;
			reason = "Bad Request";
		} else {
			extra = "Session: PS3AIRPLAY000001\r\n";
		}
	} else {
		status = 501;
		reason = "Not Implemented";
	}
	mbedtls_platform_zeroize(new_audio_key, sizeof(new_audio_key));
	mbedtls_platform_zeroize(new_audio_iv, sizeof(new_audio_iv));

	if (status == 200 && strcmp(fields.method, "SETUP") == 0 &&
		next_state == RAOP_SESSION_SETUP) {
		session->state = next_state;
		++session->requests;
		return 1;
	}
	response_extra = extra;
	if (apple_header[0] != '\0' &&
		status == 200) {
		count = snprintf(combined_extra, sizeof(combined_extra), "%s%s",
			apple_header, extra);
		if (count <= 0 || (size_t)count >= sizeof(combined_extra))
			return 0;
		response_extra = combined_extra;
	}
	if (strcmp(fields.method, "ANNOUNCE") == 0)
		session_diagnostic(session, "ANNOUNCE_RESPONSE_BUILD_START", status,
			session->requests);
	if (!write_response(response, response_capacity, status, reason, &fields,
		response_extra)) {
		if (strcmp(fields.method, "ANNOUNCE") == 0)
			session_diagnostic(session, "ANNOUNCE_RESPONSE_BUILD_FAIL", status,
				response_capacity);
		return 0;
	}
	if (strcmp(fields.method, "ANNOUNCE") == 0)
		session_diagnostic(session, "ANNOUNCE_RESPONSE_BUILD_DONE", status,
			strlen(response));
	if (status == 200) {
		session->state = next_state;
		if (clear_offer) {
			mbedtls_platform_zeroize(session->audio_key,
				sizeof(session->audio_key));
			mbedtls_platform_zeroize(session->audio_iv,
				sizeof(session->audio_iv));
			session->has_audio_key = 0;
			memset(&session->offer, 0, sizeof(session->offer));
			session->volume_millidb = 0;
			session->volume_gain = 1.0f;
			session->volume_muted = 0;
		}
		if (has_volume)
			set_volume(session, requested_volume_millidb);
	}
	++session->requests;
	return 1;
}
