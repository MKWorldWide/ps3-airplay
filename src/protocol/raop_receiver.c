#define _POSIX_C_SOURCE 200809L

#if defined(RAOP_USE_PSL1GHT)
#include <sys/types.h>
#include <sys/time.h>
#endif

#include "raop_receiver.h"

#include <errno.h>
#include <limits.h>
#include <stdio.h>
#include <string.h>
#include <sys/time.h>
#include <time.h>

#include <mbedtls/platform_util.h>

#if defined(RAOP_USE_PSL1GHT)
#include <lv2/systime.h>
#endif

#if defined(__powerpc64__) && !defined(RAOP_USE_PSL1GHT)
#include <cell/libnet.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <netinet/in.h>
#elif defined(RAOP_USE_PSL1GHT)
#include <sys/time.h>
#include <net/select.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#else
#include <sys/select.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#endif

#define RAOP_UDP_PACKET_MAX 2048
#define RAOP_MAX_MESSAGES_PER_POLL 4
#define RAOP_REQUEST_DEADLINE_MS 2000U
#define RAOP_RESPONSE_DEADLINE_MS 2000U
#define RAOP_CLIENT_IDLE_MS 60000U
#define RAOP_RESEND_MAX_PACKETS 16U
#define RAOP_RESEND_MIN_INTERVAL_MS 50U

static int bounded_event_count(uint64_t count);

static uint64_t monotonic_milliseconds(void)
{
#if defined(RAOP_USE_PSL1GHT)
	return (uint64_t)(sysGetSystemTime() / 1000U);
#else
	struct timespec now;
	if (clock_gettime(CLOCK_MONOTONIC, &now) != 0)
		return 0;
	return (uint64_t)now.tv_sec * 1000U + (uint64_t)now.tv_nsec / 1000000U;
#endif
}

static void close_socket(int fd)
{
#if defined(__powerpc64__) && !defined(RAOP_USE_PSL1GHT)
	(void)socketclose(fd);
#else
	(void)close(fd);
#endif
}

static int ascii_equal(const char *left, size_t length, const char *right)
{
	size_t i;
	if (strlen(right) != length)
		return 0;
	for (i = 0; i < length; ++i) {
		char a = left[i], b = right[i];
		if (a >= 'A' && a <= 'Z') a = (char)(a + ('a' - 'A'));
		if (b >= 'A' && b <= 'Z') b = (char)(b + ('a' - 'A'));
		if (a != b)
			return 0;
	}
	return 1;
}

static int decimal_value(const char *text, size_t length, size_t *value)
{
	size_t out = 0, i;
	while (length > 0 && (*text == ' ' || *text == '\t')) {
		++text;
		--length;
	}
	while (length > 0 && (text[length - 1] == ' ' || text[length - 1] == '\t'))
		--length;
	if (length == 0)
		return 0;
	for (i = 0; i < length; ++i) {
		if (text[i] < '0' || text[i] > '9' ||
			out > (RAOP_SESSION_SDP_MAX - (size_t)(text[i] - '0')) / 10)
			return 0;
		out = out * 10 + (size_t)(text[i] - '0');
	}
	*value = out;
	return 1;
}

/* Returns 0 while incomplete, -1 if invalid/oversized, otherwise message size. */
static int message_length(const char *data, size_t length, size_t *message_size)
{
	const char *end = data + length;
	const char *header_end = NULL;
	const char *line;
	size_t body_length = 0;
	int have_length = 0;
	size_t header_length;
	for (line = data; line + 3 < end; ++line) {
		if (memcmp(line, "\r\n\r\n", 4) == 0) {
			header_end = line;
			break;
		}
	}
	if (header_end == NULL)
		return length >= RAOP_SESSION_REQUEST_MAX ? -1 : 0;
	header_length = (size_t)(header_end - data);
	line = data;
	while (line < header_end) {
		const char *next = NULL;
		const char *colon;
		const char *scan;
		for (scan = line; scan + 1 <= header_end + 1; ++scan) {
			if (scan[0] == '\r' && scan[1] == '\n') {
				next = scan;
				break;
			}
		}
		if (next == NULL)
			return -1;
		colon = memchr(line, ':', (size_t)(next - line));
		if (colon != NULL && ascii_equal(line, (size_t)(colon - line),
			"Content-Length")) {
			if (have_length || !decimal_value(colon + 1,
				(size_t)(next - (colon + 1)), &body_length))
				return -1;
			have_length = 1;
		}
		line = next + 2;
	}
	if (body_length > RAOP_SESSION_SDP_MAX)
		return -1;
	if (header_length + 4 > RAOP_SESSION_REQUEST_MAX - body_length)
		return -1;
	*message_size = header_length + 4 + body_length;
	if (length < *message_size)
		return 0;
	return (int)*message_size;
}

static int bind_socket(int type, uint16_t port, uint16_t *actual_port)
{
	struct sockaddr_in address;
	socklen_t length = (socklen_t)sizeof(address);
	int reuse = 1;
	int fd = socket(AF_INET, type, 0);
	if (fd < 0)
		return -1;
	(void)setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse));
	memset(&address, 0, sizeof(address));
	address.sin_family = AF_INET;
	address.sin_addr.s_addr = htonl(INADDR_ANY);
	address.sin_port = htons(port);
	if (bind(fd, (struct sockaddr *)&address, sizeof(address)) < 0 ||
		getsockname(fd, (struct sockaddr *)&address, &length) < 0) {
		close_socket(fd);
		return -1;
	}
	*actual_port = ntohs(address.sin_port);
	return fd;
}

static void timestamp_utc(char timestamp[21])
{
	time_t now = time(NULL);
	struct tm *utc = gmtime(&now);
	if (utc == NULL || strftime(timestamp, 21, "%Y-%m-%dT%H:%M:%SZ", utc) == 0)
		memcpy(timestamp, "1970-01-01T00:00:00Z", 21);
}

static void log_event(raop_receiver *receiver, const char *event,
	int value, size_t bytes)
{
	char timestamp[21];
	if (receiver->log == NULL)
		return;
	timestamp_utc(timestamp);
	(void)receiver_log_file_event(receiver->log, timestamp, event, value, bytes);
}

static void session_log_event(void *context, const char *event, int value,
	size_t bytes)
{
	log_event((raop_receiver *)context, event, value, bytes);
}

static void configure_session_diagnostics(raop_receiver *receiver)
{
	raop_session_set_diagnostic(&receiver->session, session_log_event,
		receiver);
}

static size_t flush_pending_audio_datagrams(raop_receiver *receiver)
{
	uint8_t packet[RAOP_UDP_PACKET_MAX];
	size_t discarded = 0;
	unsigned int attempt;
	for (attempt = 0; attempt < RAOP_RTP_QUEUE_CAPACITY; ++attempt) {
		fd_set read_set;
		struct timeval timeout = {0, 0};
		int ready;
		FD_ZERO(&read_set);
		FD_SET(receiver->audio_fd, &read_set);
		ready = select(receiver->audio_fd + 1, &read_set, NULL, NULL,
			&timeout);
		if (ready <= 0 || !FD_ISSET(receiver->audio_fd, &read_set))
			break;
		if (recvfrom(receiver->audio_fd, packet, sizeof(packet), 0,
			NULL, NULL) < 0)
			break;
		++discarded;
	}
	log_event(receiver, "RTP_SOCKET_FLUSH", (int)discarded, 0);
	if (attempt == RAOP_RTP_QUEUE_CAPACITY) {
		fd_set read_set;
		struct timeval timeout = {0, 0};
		FD_ZERO(&read_set);
		FD_SET(receiver->audio_fd, &read_set);
		if (select(receiver->audio_fd + 1, &read_set, NULL, NULL,
			&timeout) > 0 && FD_ISSET(receiver->audio_fd, &read_set))
			log_event(receiver, "RTP_FLUSH_DRAIN_LIMIT",
				RAOP_RTP_QUEUE_CAPACITY, 0);
	}
	return discarded;
}

#ifdef RAOP_VERBOSE_LOGGING
static const char *rtsp_method_event(const char *request, size_t length)
{
	static const struct {
		const char *method;
		const char *event;
	} methods[] = {
		{"OPTIONS", "RTSP_OPTIONS"}, {"ANNOUNCE", "RTSP_ANNOUNCE"},
		{"SETUP", "RTSP_SETUP"}, {"RECORD", "RTSP_RECORD"},
		{"PAUSE", "RTSP_PAUSE"}, {"FLUSH", "RTSP_FLUSH"},
		{"TEARDOWN", "RTSP_TEARDOWN"},
		{"GET_PARAMETER", "RTSP_GET_PARAMETER"},
		{"SET_PARAMETER", "RTSP_SET_PARAMETER"}
	};
	const char *space = memchr(request, ' ', length);
	size_t method_length, i;
	if (space == NULL)
		return "RTSP_UNKNOWN";
	method_length = (size_t)(space - request);
	for (i = 0; i < sizeof(methods) / sizeof(methods[0]); ++i)
		if (strlen(methods[i].method) == method_length &&
			memcmp(request, methods[i].method, method_length) == 0)
			return methods[i].event;
	return "RTSP_OTHER";
}
#endif

static void disconnect_client(raop_receiver *receiver)
{
	if (receiver->client_fd >= 0)
		close_socket(receiver->client_fd);
	receiver->client_fd = -1;
	mbedtls_platform_zeroize(receiver->session.audio_key,
		sizeof(receiver->session.audio_key));
	mbedtls_platform_zeroize(receiver->session.audio_iv,
		sizeof(receiver->session.audio_iv));
	receiver->request_length = 0;
	receiver->request_started_ms = 0;
	receiver->client_last_activity_ms = 0;
	receiver->timing_requests = 0;
	receiver->timing_responses = 0;
	receiver->timing_bad_packets = 0;
	receiver->timing_next_send_ms = 0;
	receiver->timing_outstanding_ms = 0;
	receiver->timing_sequence = 0;
	receiver->timing_outstanding_sequence = 0;
	receiver->timing_last_rtt_ms = 0;
	receiver->timing_max_rtt_ms = 0;
	receiver->timing_outstanding = 0;
	receiver->resend_sequence = 0;
	receiver->resend_first_sequence = 0;
	receiver->resend_packet_count = 0;
	receiver->resend_received_mask = 0;
	receiver->resend_range_valid = 0;
	receiver->resend_requests = 0;
	receiver->resend_failures = 0;
	receiver->resend_recovered = 0;
	receiver->resend_rejected = 0;
	receiver->resend_last_send_ms = 0;
	raop_session_init(&receiver->session, receiver->audio_port,
		receiver->control_port, receiver->timing_port);
	configure_session_diagnostics(receiver);
	memset(&receiver->audio_queue, 0, sizeof(receiver->audio_queue));
	receiver->last_audio_log_packets = 0;
}

static int initialize_client_session(raop_receiver *receiver, int client_fd)
{
	struct sockaddr_in local;
	socklen_t local_length = (socklen_t)sizeof(local);
	raop_session_init(&receiver->session, receiver->audio_port,
		receiver->control_port, receiver->timing_port);
	configure_session_diagnostics(receiver);
	if (!receiver->crypto_configured)
		return 1;
	memset(&local, 0, sizeof(local));
	if (getsockname(client_fd, (struct sockaddr *)&local, &local_length) != 0 ||
		local.sin_family != AF_INET || local.sin_addr.s_addr == htonl(INADDR_ANY))
		return 0;
	return raop_session_configure_crypto(&receiver->session, receiver->crypto,
		receiver->rng, receiver->rng_context,
		(const uint8_t *)&local.sin_addr.s_addr, receiver->mac);
}

static int send_all(int fd, const char *data, size_t length)
{
	size_t sent = 0;
	uint64_t started_ms = monotonic_milliseconds();
	while (sent < length) {
		uint64_t now_ms = monotonic_milliseconds();
		if (started_ms != 0 && now_ms >= started_ms &&
			now_ms - started_ms >= RAOP_RESPONSE_DEADLINE_MS) {
			errno = ETIMEDOUT;
			return 0;
		}
		int result = (int)send(fd, data + sent, length - sent, 0);
		if (result < 0 && errno == EINTR)
			continue;
		if (result <= 0)
			return 0;
		sent += (size_t)result;
	}
	return 1;
}

static int configure_client_send_timeout(int fd)
{
	struct timeval timeout;
	timeout.tv_sec = (long)(RAOP_RESPONSE_DEADLINE_MS / 1000U);
	timeout.tv_usec = (long)(RAOP_RESPONSE_DEADLINE_MS % 1000U) * 1000L;
	return setsockopt(fd, SOL_SOCKET, SO_SNDTIMEO, &timeout,
		sizeof(timeout)) == 0;
}

static int serve_messages(raop_receiver *receiver)
{
	unsigned int handled;
	for (handled = 0; handled < RAOP_MAX_MESSAGES_PER_POLL; ++handled) {
		size_t size = 0;
		int result = message_length(receiver->request,
			receiver->request_length, &size);
		int status = 0;
		uint64_t handle_started_ms = monotonic_milliseconds();
		char response[RAOP_SESSION_RESPONSE_MAX];
#ifdef RAOP_VERBOSE_LOGGING
		const char *method_event;
		raop_session_state previous_state;
#endif
		if (result == 0)
			return 1;
#ifdef RAOP_VERBOSE_LOGGING
		method_event = rtsp_method_event(receiver->request,
			receiver->request_length);
		previous_state = receiver->session.state;
		log_event(receiver, method_event, (int)previous_state,
			size > 0 ? size : receiver->request_length);
#endif
		if (result < 0 || !raop_session_handle(&receiver->session,
			receiver->request, size, response, sizeof(response))) {
			log_event(receiver, "RTSP_BAD_FRAME", result, receiver->request_length);
			return 0;
		}
		log_event(receiver, "RTSP_HANDLE_MS",
			bounded_event_count(monotonic_milliseconds() - handle_started_ms), 0);
		(void)sscanf(response, "RTSP/1.0 %d", &status);
		if (status == 200 && size >= 6 &&
			memcmp(receiver->request, "FLUSH ", 6) == 0) {
			size_t discarded = receiver->audio_queue.count;
			raop_rtp_queue_flush(&receiver->audio_queue);
			(void)flush_pending_audio_datagrams(receiver);
			receiver->resend_range_valid = 0;
			receiver->resend_received_mask = 0;
			receiver->resend_packet_count = 0;
			receiver->resend_sequence = 0;
			receiver->resend_first_sequence = 0;
			receiver->resend_last_send_ms = 0;
			receiver->last_audio_log_packets =
				receiver->audio_queue.stats.packets;
			receiver->last_audio_log_payload_bytes =
				receiver->audio_queue.stats.payload_bytes;
			receiver->last_audio_log_missing_packets =
				receiver->audio_queue.stats.missing_packets;
			receiver->last_audio_log_out_of_order_packets =
				receiver->audio_queue.stats.out_of_order_packets;
			receiver->last_audio_log_duplicate_packets =
				receiver->audio_queue.stats.duplicate_packets;
			receiver->last_audio_log_late_packets =
				receiver->audio_queue.stats.late_packets;
			receiver->last_audio_log_queue_drops =
				receiver->audio_queue.stats.queue_drops;
			++receiver->flush_generation;
			log_event(receiver, "RTP_QUEUE_FLUSH", (int)discarded, 0);
		}
		if (!send_all(receiver->client_fd, response, strlen(response))) {
#ifdef RAOP_VERBOSE_LOGGING
			const char *event = errno == ETIMEDOUT || errno == EAGAIN ||
				errno == EWOULDBLOCK ? "RTSP_SEND_TIMEOUT" : "RTSP_SEND_FAIL";
			log_event(receiver, event, errno, strlen(response));
#endif
			return 0;
		}
		if (receiver->session.state != RAOP_SESSION_RECORDING ||
			(size >= 6 && memcmp(receiver->request, "FLUSH ", 6) == 0) ||
			(size >= 9 && memcmp(receiver->request, "TEARDOWN ", 9) == 0))
			receiver->resend_range_valid = 0;
		{
#ifdef RAOP_VERBOSE_LOGGING
			log_event(receiver, "RTSP_STATUS", status, size);
			if (receiver->session.state != previous_state)
				log_event(receiver, "RAOP_STATE",
					(int)receiver->session.state, 0);
			if (status == 200 && strcmp(method_event, "RTSP_ANNOUNCE") == 0) {
				log_event(receiver, "SDP_RATE",
					(int)receiver->session.offer.fmtp[10], 0);
				log_event(receiver, "SDP_CHANNELS",
					(int)receiver->session.offer.fmtp[6], 0);
				log_event(receiver, "SDP_BITS",
					(int)receiver->session.offer.fmtp[2], 0);
				log_event(receiver, "SDP_ENCRYPTED",
					receiver->session.has_audio_key, 0);
			}
#else
			log_event(receiver, "RTSP_MESSAGE", status, size);
#endif
		}
		receiver->request_length -= size;
		if (receiver->request_length > 0)
			memmove(receiver->request, receiver->request + size,
				receiver->request_length);
		receiver->request_started_ms = receiver->request_length == 0 ? 0 :
			monotonic_milliseconds();
	}
	return 1;
}

static int receive_client(raop_receiver *receiver)
{
	size_t available = sizeof(receiver->request) - receiver->request_length;
	size_t previous_length = receiver->request_length;
	int amount;
	if (available == 0)
		return 0;
	amount = (int)recv(receiver->client_fd,
		receiver->request + receiver->request_length, available, 0);
	if (amount < 0 && (errno == EINTR || errno == EAGAIN || errno == EWOULDBLOCK))
		return 1;
	if (amount == 0) {
		log_event(receiver, "RTSP_PEER_CLOSED", 0, receiver->request_length);
		return 0;
	}
	if (amount < 0) {
		log_event(receiver, "RTSP_RECV_FAIL", errno, receiver->request_length);
		return 0;
	}
	if (previous_length == 0)
		receiver->request_started_ms = monotonic_milliseconds();
	receiver->client_last_activity_ms = monotonic_milliseconds();
	receiver->request_length += (size_t)amount;
	return serve_messages(receiver);
}

static int same_sender(const raop_receiver *receiver,
	const struct sockaddr_in *sender)
{
	return receiver->client_fd >= 0 &&
		sender->sin_addr.s_addr == receiver->peer_address.sin_addr.s_addr;
}

static int bounded_event_count(uint64_t count)
{
	return count > INT_MAX ? INT_MAX : (int)count;
}

static uint16_t read_u16(const uint8_t *data)
{
	return (uint16_t)(((uint16_t)data[0] << 8) | data[1]);
}

static void write_u16(uint8_t *data, uint16_t value)
{
	data[0] = (uint8_t)(value >> 8);
	data[1] = (uint8_t)value;
}

static void request_missing_audio(raop_receiver *receiver, uint64_t now_ms)
{
	uint8_t request[8] = {0x80, 0xd5, 0, 0, 0, 0, 0, 0};
	struct sockaddr_in destination;
	uint16_t first = receiver->audio_queue.stats.last_missing_sequence;
	uint16_t count = receiver->audio_queue.stats.last_missing_count;
	ssize_t sent;
	if (count == 0 || receiver->session.peer_control_port == 0)
		return;
	if (count > RAOP_RESEND_MAX_PACKETS) {
		log_event(receiver, "RTP_RESEND_RANGE_LIMIT", first, count);
		return;
	}
	if (receiver->resend_last_send_ms != 0 &&
		now_ms >= receiver->resend_last_send_ms &&
		now_ms - receiver->resend_last_send_ms < RAOP_RESEND_MIN_INTERVAL_MS)
		return;
	++receiver->resend_sequence;
	write_u16(request + 2, receiver->resend_sequence);
	write_u16(request + 4, first);
	write_u16(request + 6, count);
	destination = receiver->peer_address;
	destination.sin_port = htons(receiver->session.peer_control_port);
	sent = sendto(receiver->control_fd, request, sizeof(request), 0,
		(struct sockaddr *)&destination, sizeof(destination));
	receiver->resend_last_send_ms = now_ms;
	if (sent != (ssize_t)sizeof(request)) {
		++receiver->resend_failures;
		log_event(receiver, "RTP_RESEND_SEND_FAILED", first,
			sent < 0 ? 0 : (size_t)sent);
		return;
	}
	receiver->resend_first_sequence = first;
	receiver->resend_packet_count = count;
	receiver->resend_received_mask = (uint16_t)((1U << count) - 1U);
	receiver->resend_range_valid = 1;
	++receiver->resend_requests;
	if (receiver->resend_requests == 1 ||
		(receiver->resend_requests % 16) == 0)
		log_event(receiver, "RTP_RESEND_REQUEST", first, count);
}

static void receive_resend_audio(raop_receiver *receiver,
	const struct sockaddr_in *sender, const uint8_t *packet, size_t length)
{
	raop_rtp_packet audio;
	uint16_t offset, bit;
	if (receiver->session.state != RAOP_SESSION_RECORDING ||
		ntohs(sender->sin_port) != receiver->session.peer_control_port ||
		length <= 4 || packet[0] != 0x80 || packet[1] != 0xd6 ||
		!raop_rtp_parse(packet + 4, length - 4, &audio) ||
		audio.payload_type != 96) {
		++receiver->resend_rejected;
		if (receiver->resend_rejected == 1 ||
			(receiver->resend_rejected % 16) == 0)
			log_event(receiver, "RTP_RESEND_REJECTED", -1, length);
		return;
	}
	if (!receiver->resend_range_valid || receiver->resend_packet_count == 0) {
		++receiver->resend_rejected;
		if (receiver->resend_rejected == 1 ||
			(receiver->resend_rejected % 16) == 0)
			log_event(receiver, "RTP_RESEND_REJECTED", audio.sequence, length);
		return;
	}
	offset = (uint16_t)(audio.sequence - receiver->resend_first_sequence);
	if (offset >= receiver->resend_packet_count || offset >= 16) {
		++receiver->resend_rejected;
		if (receiver->resend_rejected == 1 ||
			(receiver->resend_rejected % 16) == 0)
			log_event(receiver, "RTP_RESEND_REJECTED", audio.sequence, length);
		return;
	}
	bit = (uint16_t)(1U << offset);
	if ((receiver->resend_received_mask & bit) == 0 ||
		!raop_rtp_queue_recover(&receiver->audio_queue, packet + 4,
			length - 4)) {
		++receiver->resend_rejected;
		if (receiver->resend_rejected == 1 ||
			(receiver->resend_rejected % 16) == 0)
			log_event(receiver, "RTP_RESEND_REJECTED", audio.sequence, length);
		return;
	}
	receiver->resend_received_mask &= (uint16_t)~bit;
	++receiver->resend_recovered;
	if (receiver->resend_recovered == 1 ||
		(receiver->resend_recovered % 16) == 0)
		log_event(receiver, "RTP_RESEND_RECOVERED", audio.sequence,
			audio.payload_length);
}

static void send_timing_request_if_due(raop_receiver *receiver, uint64_t now_ms)
{
	uint8_t packet[32] = {0};
	struct sockaddr_in destination;
	uint64_t interval_ms;
	ssize_t sent;
	if (receiver->client_fd < 0 ||
		receiver->session.state != RAOP_SESSION_RECORDING ||
		receiver->session.peer_timing_port == 0 ||
		(now_ms < receiver->timing_next_send_ms))
		return;
	/* Classic RAOP sends 32-byte 0xD2 timing probes: three at 300 ms, then 3 s. */
	packet[0] = 0x80;
	packet[1] = 0xd2;
	++receiver->timing_sequence;
	write_u16(packet + 2, receiver->timing_sequence);
	destination = receiver->peer_address;
	destination.sin_port = htons(receiver->session.peer_timing_port);
	sent = sendto(receiver->timing_fd, packet, sizeof(packet), 0,
		(struct sockaddr *)&destination, sizeof(destination));
	if (sent != (ssize_t)sizeof(packet)) {
		log_event(receiver, "RTP_TIMING_SEND_FAILED", -1, sent < 0 ? 0 : (size_t)sent);
		interval_ms = 3000;
	} else {
		++receiver->timing_requests;
		receiver->timing_outstanding = 1;
		receiver->timing_outstanding_sequence = receiver->timing_sequence;
		receiver->timing_outstanding_ms = now_ms;
		interval_ms = receiver->timing_requests < 3 ? 300 : 3000;
		if (receiver->timing_requests == 1 ||
			(receiver->timing_requests % 16) == 0)
			log_event(receiver, "RTP_TIMING_REQUEST", (int)receiver->timing_requests,
				sizeof(packet));
	}
	receiver->timing_next_send_ms = now_ms + interval_ms;
}

static void receive_udp(raop_receiver *receiver, int fd, int channel)
{
	uint8_t packet[RAOP_UDP_PACKET_MAX];
	struct sockaddr_in sender;
	socklen_t sender_length = (socklen_t)sizeof(sender);
	int amount = (int)recvfrom(fd, packet, sizeof(packet), 0,
		(struct sockaddr *)&sender, &sender_length);
	if (amount <= 0 || !same_sender(receiver, &sender))
		return;
	if (channel == 0) {
		uint64_t now_ms;
		raop_rtp_stats *stats = &receiver->audio_queue.stats;
		if (receiver->session.state != RAOP_SESSION_RECORDING)
			return;
		(void)raop_rtp_queue_push(&receiver->audio_queue, packet,
			(size_t)amount);
		if (stats->missing_packets != 0) {
			now_ms = monotonic_milliseconds();
			request_missing_audio(receiver, now_ms);
		}
		if (stats->packets >= receiver->last_audio_log_packets &&
			stats->packets - receiver->last_audio_log_packets >= 128U) {
			uint64_t delta = stats->packets - receiver->last_audio_log_packets;
			uint64_t payload_delta = stats->payload_bytes -
				receiver->last_audio_log_payload_bytes;
			log_event(receiver, "RTP_RX_BATCH",
				bounded_event_count(delta), (size_t)payload_delta);
			log_event(receiver, "RTP_QUEUE_DEPTH",
				(int)receiver->audio_queue.count, 0);
			if (stats->missing_packets >
				receiver->last_audio_log_missing_packets)
				log_event(receiver, "RTP_MISSING_BATCH",
					bounded_event_count(stats->missing_packets -
					receiver->last_audio_log_missing_packets), 0);
			if (stats->out_of_order_packets >
				receiver->last_audio_log_out_of_order_packets)
				log_event(receiver, "RTP_REORDER_BATCH",
					bounded_event_count(stats->out_of_order_packets -
					receiver->last_audio_log_out_of_order_packets), 0);
			if (stats->duplicate_packets >
				receiver->last_audio_log_duplicate_packets)
				log_event(receiver, "RTP_DUPLICATE_BATCH",
					bounded_event_count(stats->duplicate_packets -
					receiver->last_audio_log_duplicate_packets), 0);
			if (stats->late_packets > receiver->last_audio_log_late_packets)
				log_event(receiver, "RTP_LATE_BATCH",
					bounded_event_count(stats->late_packets -
					receiver->last_audio_log_late_packets), 0);
			if (stats->queue_drops >
				receiver->last_audio_log_queue_drops)
				log_event(receiver, "RTP_QUEUE_DROP_BATCH",
					bounded_event_count(stats->queue_drops -
					receiver->last_audio_log_queue_drops), 0);
			receiver->last_audio_log_packets = stats->packets;
			receiver->last_audio_log_payload_bytes = stats->payload_bytes;
			receiver->last_audio_log_missing_packets = stats->missing_packets;
			receiver->last_audio_log_out_of_order_packets =
				stats->out_of_order_packets;
			receiver->last_audio_log_duplicate_packets =
				stats->duplicate_packets;
			receiver->last_audio_log_late_packets = stats->late_packets;
			receiver->last_audio_log_queue_drops = stats->queue_drops;
		}
	} else if (channel == 1) {
		++receiver->control_packets;
		if (amount >= 2 && packet[1] == 0xd6)
			receive_resend_audio(receiver, &sender, packet, (size_t)amount);
	} else {
		++receiver->timing_packets;
		if (ntohs(sender.sin_port) != receiver->session.peer_timing_port ||
			amount != 32 || packet[0] != 0x80 || packet[1] != 0xd3 ||
			!receiver->timing_outstanding ||
			read_u16(packet + 2) != receiver->timing_outstanding_sequence) {
			++receiver->timing_bad_packets;
			if (receiver->timing_bad_packets == 1 ||
				(receiver->timing_bad_packets % 16) == 0)
				log_event(receiver, "RTP_TIMING_BAD",
					amount > 0 ? amount : -1,
					amount > 0 ? (size_t)amount : 0);
			return;
		}
		{
			uint64_t now_ms = monotonic_milliseconds();
			uint64_t rtt = now_ms >= receiver->timing_outstanding_ms ?
				now_ms - receiver->timing_outstanding_ms : 0;
			receiver->timing_last_rtt_ms = rtt > 65535 ? 65535 :
				(uint16_t)rtt;
			if (receiver->timing_last_rtt_ms > receiver->timing_max_rtt_ms)
				receiver->timing_max_rtt_ms = receiver->timing_last_rtt_ms;
			receiver->timing_outstanding = 0;
			++receiver->timing_responses;
			if (receiver->timing_responses == 1 ||
				(receiver->timing_responses % 16) == 0)
				log_event(receiver, "RTP_TIMING_REPLY",
					receiver->timing_last_rtt_ms, (size_t)amount);
		}
	}
}

int raop_receiver_open(raop_receiver *receiver, uint16_t rtsp_port,
	uint16_t rtp_base_port, receiver_log_file *log)
{
	uint16_t p0 = rtp_base_port, p1 = 0, p2 = 0;
	if (receiver == NULL || (rtp_base_port != 0 && rtp_base_port > 65532))
		return 0;
	memset(receiver, 0, sizeof(*receiver));
	receiver->tcp_fd = receiver->client_fd = -1;
	receiver->audio_fd = receiver->control_fd = receiver->timing_fd = -1;
	receiver->peer_length = (socklen_t)sizeof(receiver->peer_address);
	receiver->log = log;
	receiver->tcp_fd = bind_socket(SOCK_STREAM, rtsp_port,
		&receiver->rtsp_port);
	if (receiver->tcp_fd < 0 || listen(receiver->tcp_fd, 1) < 0)
	{
#ifdef RAOP_VERBOSE_LOGGING
		log_event(receiver, receiver->tcp_fd < 0 ? "RTSP_BIND_FAIL" :
			"RTSP_LISTEN_FAIL", errno, 0);
#endif
		goto fail;
	}
	receiver->audio_fd = bind_socket(SOCK_DGRAM, p0, &receiver->audio_port);
	if (receiver->audio_fd < 0) {
#ifdef RAOP_VERBOSE_LOGGING
		log_event(receiver, "RTP_AUDIO_BIND_FAIL", errno, 0);
#endif
		goto fail;
	}
	if (rtp_base_port != 0) {
		p1 = (uint16_t)(rtp_base_port + 1);
		p2 = (uint16_t)(rtp_base_port + 2);
	}
	receiver->control_fd = bind_socket(SOCK_DGRAM, p1,
		&receiver->control_port);
	receiver->timing_fd = bind_socket(SOCK_DGRAM, p2,
		&receiver->timing_port);
	if (receiver->control_fd < 0) {
#ifdef RAOP_VERBOSE_LOGGING
		log_event(receiver, "RTP_CONTROL_BIND_FAIL", errno, 0);
#endif
		goto fail;
	}
	if (receiver->timing_fd < 0) {
#ifdef RAOP_VERBOSE_LOGGING
		log_event(receiver, "RTP_TIMING_BIND_FAIL", errno, 0);
#endif
		goto fail;
	}
	raop_session_init(&receiver->session, receiver->audio_port,
		receiver->control_port, receiver->timing_port);
	configure_session_diagnostics(receiver);
#ifdef RAOP_VERBOSE_LOGGING
	log_event(receiver, "RTSP_LISTEN_READY", receiver->rtsp_port, 0);
	log_event(receiver, "RTP_AUDIO_READY", receiver->audio_port, 0);
	log_event(receiver, "RTP_CONTROL_READY", receiver->control_port, 0);
	log_event(receiver, "RTP_TIMING_READY", receiver->timing_port, 0);
#endif
	return 1;
fail:
	raop_receiver_close(receiver);
	return 0;
}

int raop_receiver_configure_crypto(raop_receiver *receiver,
	raop_crypto_context *crypto, mbedtls_f_rng_t *rng, void *rng_context,
	const uint8_t mac[6])
{
	if (receiver == NULL || receiver->tcp_fd < 0 || crypto == NULL ||
		!crypto->initialized || rng == NULL || mac == NULL)
		return 0;
	receiver->crypto = crypto;
	receiver->rng = rng;
	receiver->rng_context = rng_context;
	memcpy(receiver->mac, mac, sizeof(receiver->mac));
	receiver->crypto_configured = 1;
	return 1;
}

int raop_receiver_poll(raop_receiver *receiver, unsigned int timeout_ms)
{
	fd_set read_set;
	struct timeval timeout;
	int max_fd;
	int result;
	uint64_t now;
	uint64_t flush_generation_before;
	if (receiver == NULL || receiver->tcp_fd < 0 || receiver->audio_fd < 0)
		return -1;
	flush_generation_before = receiver->flush_generation;
	now = monotonic_milliseconds();
	if (receiver->client_fd >= 0 && now != 0 &&
		((receiver->client_last_activity_ms != 0 &&
		now - receiver->client_last_activity_ms >= RAOP_CLIENT_IDLE_MS) ||
		 (receiver->request_started_ms != 0 &&
		now - receiver->request_started_ms >= RAOP_REQUEST_DEADLINE_MS))) {
		log_event(receiver, "RTSP_TIMEOUT", 0, receiver->request_length);
		disconnect_client(receiver);
	}
	FD_ZERO(&read_set);
	FD_SET(receiver->tcp_fd, &read_set);
	FD_SET(receiver->audio_fd, &read_set);
	FD_SET(receiver->control_fd, &read_set);
	FD_SET(receiver->timing_fd, &read_set);
	max_fd = receiver->tcp_fd;
	if (receiver->audio_fd > max_fd) max_fd = receiver->audio_fd;
	if (receiver->control_fd > max_fd) max_fd = receiver->control_fd;
	if (receiver->timing_fd > max_fd) max_fd = receiver->timing_fd;
	if (receiver->client_fd >= 0) {
		FD_SET(receiver->client_fd, &read_set);
		if (receiver->client_fd > max_fd) max_fd = receiver->client_fd;
	}
	timeout.tv_sec = (long)(timeout_ms / 1000U);
	timeout.tv_usec = (long)(timeout_ms % 1000U) * 1000L;
	result = select(max_fd + 1, &read_set, NULL, NULL, &timeout);
	if (result < 0) {
		if (errno != EINTR) {
#ifdef RAOP_VERBOSE_LOGGING
			log_event(receiver, "RTSP_SELECT_FAIL", errno, 0);
#endif
		}
		return errno == EINTR ? 0 : -1;
	}
	if (result == 0) {
		send_timing_request_if_due(receiver, monotonic_milliseconds());
		return 0;
	}
	if (FD_ISSET(receiver->tcp_fd, &read_set)) {
		struct sockaddr_in peer;
		socklen_t length = (socklen_t)sizeof(peer);
		int fd = accept(receiver->tcp_fd, (struct sockaddr *)&peer, &length);
		if (fd >= 0) {
			if (receiver->client_fd >= 0) {
#ifdef RAOP_VERBOSE_LOGGING
				log_event(receiver, "RTSP_BUSY_DROP", 0, 0);
#endif
				close_socket(fd);
			} else if (!configure_client_send_timeout(fd)) {
				log_event(receiver, "RTSP_SEND_TIMEOUT_SETUP_FAILED", errno, 0);
				close_socket(fd);
			} else {
				receiver->client_fd = fd;
				receiver->peer_address = peer;
				receiver->request_length = 0;
				receiver->request_started_ms = 0;
				receiver->client_last_activity_ms = monotonic_milliseconds();
				if (!initialize_client_session(receiver, fd)) {
					log_event(receiver, "CRYPTO_CONFIG_FAILED", -1, 0);
					disconnect_client(receiver);
				} else {
					memset(&receiver->audio_queue, 0,
						sizeof(receiver->audio_queue));
					receiver->last_audio_log_packets = 0;
					receiver->last_audio_log_payload_bytes = 0;
					receiver->last_audio_log_missing_packets = 0;
					receiver->last_audio_log_out_of_order_packets = 0;
					receiver->last_audio_log_duplicate_packets = 0;
					receiver->last_audio_log_late_packets = 0;
					receiver->last_audio_log_queue_drops = 0;
					log_event(receiver, "RTSP_CONNECTED", 0, 0);
				}
			}
		} else if (errno != EINTR && errno != EAGAIN && errno != EWOULDBLOCK) {
#ifdef RAOP_VERBOSE_LOGGING
			log_event(receiver, "RTSP_ACCEPT_FAIL", errno, 0);
#endif
		}
	}
	if (receiver->client_fd >= 0 && FD_ISSET(receiver->client_fd, &read_set)) {
		if (!receive_client(receiver)) {
			log_event(receiver, "RTSP_DISCONNECTED", 0, 0);
			disconnect_client(receiver);
		}
	}
	if (receiver->flush_generation == flush_generation_before &&
		FD_ISSET(receiver->audio_fd, &read_set))
		receive_udp(receiver, receiver->audio_fd, 0);
	if (FD_ISSET(receiver->control_fd, &read_set))
		receive_udp(receiver, receiver->control_fd, 1);
	if (FD_ISSET(receiver->timing_fd, &read_set))
		receive_udp(receiver, receiver->timing_fd, 2);
	send_timing_request_if_due(receiver, monotonic_milliseconds());
	return 1;
}

uint16_t raop_receiver_rtsp_port(const raop_receiver *receiver)
{
	return receiver == NULL ? 0 : receiver->rtsp_port;
}

uint16_t raop_receiver_audio_port(const raop_receiver *receiver)
{
	return receiver == NULL ? 0 : receiver->audio_port;
}

int raop_receiver_pop_audio(raop_receiver *receiver, raop_rtp_frame *frame)
{
	if (receiver == NULL || frame == NULL ||
		!raop_rtp_queue_pop(&receiver->audio_queue, frame))
		return 0;
	if (!raop_session_decrypt_audio(&receiver->session, frame->payload,
		frame->packet.payload_length)) {
		log_event(receiver, "RTP_DECRYPT_FAILED", -1,
			frame->packet.payload_length);
		mbedtls_platform_zeroize(frame->payload, sizeof(frame->payload));
		return 0;
	}
	if (receiver->decrypted_audio_packets != UINT64_MAX)
		++receiver->decrypted_audio_packets;
	if (receiver->decrypted_audio_packets == 1 ||
		receiver->decrypted_audio_packets % 128U == 0)
		log_event(receiver, "RTP_DECRYPT_OK",
			receiver->decrypted_audio_packets > (uint64_t)INT_MAX ? INT_MAX :
			(int)receiver->decrypted_audio_packets,
			frame->packet.payload_length);
	return 1;
}

void raop_receiver_close(raop_receiver *receiver)
{
	if (receiver == NULL)
		return;
	if (receiver->client_fd >= 0) close_socket(receiver->client_fd);
	if (receiver->tcp_fd >= 0) close_socket(receiver->tcp_fd);
	if (receiver->audio_fd >= 0) close_socket(receiver->audio_fd);
	if (receiver->control_fd >= 0) close_socket(receiver->control_fd);
	if (receiver->timing_fd >= 0) close_socket(receiver->timing_fd);
	mbedtls_platform_zeroize(receiver->session.audio_key,
		sizeof(receiver->session.audio_key));
	mbedtls_platform_zeroize(receiver->session.audio_iv,
		sizeof(receiver->session.audio_iv));
	memset(receiver, 0, sizeof(*receiver));
	receiver->tcp_fd = receiver->client_fd = -1;
	receiver->audio_fd = receiver->control_fd = receiver->timing_fd = -1;
}
