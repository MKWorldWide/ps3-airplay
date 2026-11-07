#define _POSIX_C_SOURCE 200809L

#include "../../src/protocol/raop_receiver.h"

#include <arpa/inet.h>
#include <assert.h>
#include <errno.h>
#include <fcntl.h>
#include <pthread.h>
#include <stdatomic.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <time.h>
#include <unistd.h>

typedef struct {
	raop_receiver *receiver;
	pthread_mutex_t receiver_lock;
	atomic_int stop;
	atomic_int audio_seen;
	atomic_int timing_seen;
	atomic_uint audio_packets_seen;
	atomic_uint missing_packets_seen;
	atomic_uint resend_requests_seen;
	atomic_uint resend_recovered_seen;
	atomic_uint resend_rejected_seen;
} receiver_thread_args;

static void *run_receiver(void *opaque)
{
	receiver_thread_args *args = (receiver_thread_args *)opaque;
	while (atomic_load(&args->stop) == 0) {
		pthread_mutex_lock(&args->receiver_lock);
		(void)raop_receiver_poll(args->receiver, 20);
		if (args->receiver->audio_queue.stats.packets != 0)
			atomic_store_explicit(&args->audio_seen, 1, memory_order_release);
		atomic_store_explicit(&args->audio_packets_seen,
			(unsigned int)args->receiver->audio_queue.stats.packets,
			memory_order_release);
		atomic_store_explicit(&args->missing_packets_seen,
			(unsigned int)args->receiver->audio_queue.stats.missing_packets,
			memory_order_release);
		atomic_store_explicit(&args->resend_requests_seen,
			(unsigned int)args->receiver->resend_requests,
			memory_order_release);
		atomic_store_explicit(&args->resend_recovered_seen,
			(unsigned int)args->receiver->resend_recovered,
			memory_order_release);
		atomic_store_explicit(&args->resend_rejected_seen,
			(unsigned int)args->receiver->resend_rejected,
			memory_order_release);
		if (args->receiver->timing_responses != 0)
			atomic_store_explicit(&args->timing_seen, 1, memory_order_release);
		pthread_mutex_unlock(&args->receiver_lock);
		(void)nanosleep(&(struct timespec){0, 1000000L}, NULL);
	}
	return NULL;
}

static void write_request(int fd, const char *request)
{
	size_t length = strlen(request), sent = 0;
	while (sent < length) {
		ssize_t amount = send(fd, request + sent, length - sent, 0);
		assert(amount > 0);
		sent += (size_t)amount;
	}
}

static void receive_response(int fd, const char *request, char *response,
	size_t capacity)
{
	size_t total = 0;
	while (total + 1 < capacity) {
		ssize_t amount = recv(fd, response + total, capacity - total - 1, 0);
		if (amount <= 0) {
			fprintf(stderr, "no response for: %.48s\n", request);
			assert(amount > 0);
		}
		total += (size_t)amount;
		response[total] = '\0';
		if (strstr(response, "\r\n\r\n") != NULL)
			return;
	}
	assert(0 && "RTSP response exceeded test buffer");
}

static void send_request(int fd, const char *request, char *response,
	size_t capacity)
{
	write_request(fd, request);
	receive_response(fd, request, response, capacity);
}

static uint64_t receiver_test_milliseconds(void)
{
	struct timespec now;
	assert(clock_gettime(CLOCK_MONOTONIC, &now) == 0);
	return (uint64_t)now.tv_sec * 1000U + (uint64_t)now.tv_nsec / 1000000U;
}

static int receiver_has_client(receiver_thread_args *args)
{
	int connected;
	pthread_mutex_lock(&args->receiver_lock);
	connected = args->receiver->client_fd >= 0;
	pthread_mutex_unlock(&args->receiver_lock);
	return connected;
}

static void verify_slow_reader_disconnect(receiver_thread_args *args,
	uint16_t rtsp_port)
{
	static const char request[] = "OPTIONS * RTSP/1.0\r\nCSeq: 7\r\n\r\n";
	struct sockaddr_in target = {0};
	struct timespec pause = {0, 1000000L};
	uint64_t started_ms, elapsed_ms;
	size_t offset = 0;
	unsigned int attempt;
	int client_fd, small_buffer = 256, flags, disconnected = 0;
	client_fd = socket(AF_INET, SOCK_STREAM, 0);
	assert(client_fd >= 0);
	assert(setsockopt(client_fd, SOL_SOCKET, SO_RCVBUF, &small_buffer,
		sizeof(small_buffer)) == 0);
	target.sin_family = AF_INET;
	target.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
	target.sin_port = htons(rtsp_port);
	assert(connect(client_fd, (struct sockaddr *)&target, sizeof(target)) == 0);
	for (attempt = 0; attempt < 100 && !receiver_has_client(args); ++attempt)
		(void)nanosleep(&pause, NULL);
	assert(receiver_has_client(args));
	pthread_mutex_lock(&args->receiver_lock);
	assert(setsockopt(args->receiver->client_fd, SOL_SOCKET, SO_SNDBUF,
		&small_buffer, sizeof(small_buffer)) == 0);
	pthread_mutex_unlock(&args->receiver_lock);
	flags = fcntl(client_fd, F_GETFL, 0);
	assert(flags >= 0 && fcntl(client_fd, F_SETFL, flags | O_NONBLOCK) == 0);
	started_ms = receiver_test_milliseconds();
	for (attempt = 0; attempt < 5000 && receiver_has_client(args); ++attempt) {
		ssize_t amount = send(client_fd, request + offset,
			sizeof(request) - 1U - offset, MSG_DONTWAIT | MSG_NOSIGNAL);
		if (amount > 0) {
			offset += (size_t)amount;
			if (offset == sizeof(request) - 1U)
				offset = 0;
		} else if (errno != EAGAIN && errno != EWOULDBLOCK && errno != EINTR) {
			break;
		}
		(void)nanosleep(&pause, NULL);
	}
	disconnected = !receiver_has_client(args);
	elapsed_ms = receiver_test_milliseconds() - started_ms;
	close(client_fd);
	assert(disconnected);
	assert(elapsed_ms <= 4000U);
}

static void send_rtp(uint16_t port, uint16_t sequence)
{
	struct sockaddr_in target;
	uint8_t packet[16] = {0x80, 96, 0, 0, 0, 0, 0, 1,
		0x11, 0x22, 0x33, 0x44, 0xde, 0xad, 0xbe, 0xef};
	int fd = socket(AF_INET, SOCK_DGRAM, 0);
	assert(fd >= 0);
	packet[2] = (uint8_t)(sequence >> 8);
	packet[3] = (uint8_t)sequence;
	memset(&target, 0, sizeof(target));
	target.sin_family = AF_INET;
	target.sin_addr.s_addr = inet_addr("127.0.0.1");
	target.sin_port = htons(port);
	assert(sendto(fd, packet, sizeof(packet), 0,
		(struct sockaddr *)&target, sizeof(target)) == (ssize_t)sizeof(packet));
	close(fd);
}

static void send_empty_datagram(int fd, uint16_t port)
{
	struct sockaddr_in address;
	memset(&address, 0, sizeof(address));
	address.sin_family = AF_INET;
	address.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
	address.sin_port = htons(port);
	assert(sendto(fd, "", 0, 0, (struct sockaddr *)&address,
		sizeof(address)) == 0);
}

static void send_resend(int fd, uint16_t port, uint16_t sequence,
	int include_payload)
{
	struct sockaddr_in target;
	uint8_t packet[20] = {0x80, 0xd6, 0, 1,
		0x80, 96, 0, 0, 0, 0, 0, 2, 0x11, 0x22, 0x33, 0x44,
		0xca, 0xfe, 0xba, 0xbe};
	size_t length = include_payload ? sizeof(packet) : 16;
	packet[6] = (uint8_t)(sequence >> 8);
	packet[7] = (uint8_t)sequence;
	memset(&target, 0, sizeof(target));
	target.sin_family = AF_INET;
	target.sin_addr.s_addr = inet_addr("127.0.0.1");
	target.sin_port = htons(port);
	assert(sendto(fd, packet, length, 0, (struct sockaddr *)&target,
		sizeof(target)) == (ssize_t)length);
}

int main(void)
{
	raop_receiver receiver;
	receiver_thread_args args;
	pthread_t thread;
	struct sockaddr_in target;
	struct sockaddr_in peer_udp;
	struct timeval timeout = {2, 0};
	char response[RAOP_SESSION_RESPONSE_MAX];
	char setup[512];
	char log_contents[4096];
	uint8_t resend_request[16];
	const char *log_path = "/tmp/raop_receiver_timing_test.log";
	size_t log_size;
	FILE *log_read;
	receiver_log_file log;
	uint8_t timing_request[64];
	uint8_t timing_reply[32] = {0};
	socklen_t peer_udp_length = (socklen_t)sizeof(peer_udp);
	ssize_t timing_length;
	ssize_t resend_length;
	const char *announce = "ANNOUNCE rtsp://127.0.0.1/stream RTSP/1.0\r\n"
		"CSeq: 2\r\nContent-Type: application/sdp\r\n"
		"Content-Length: 25\r\n\r\nv=0\r\nm=audio 0 RTP/AVP 96";
	int fd, peer_udp_fd, wrong_udp_fd;
	unsigned int wait;
	assert(receiver_log_file_open(&log, log_path));
	assert(raop_receiver_open(&receiver, 0, 0, &log));
	assert(raop_receiver_rtsp_port(&receiver) != 0);
	assert(raop_receiver_audio_port(&receiver) != 0);
	args.receiver = &receiver;
	atomic_init(&args.stop, 0);
	assert(pthread_mutex_init(&args.receiver_lock, NULL) == 0);
	atomic_init(&args.audio_seen, 0);
	atomic_init(&args.timing_seen, 0);
	atomic_init(&args.audio_packets_seen, 0);
	atomic_init(&args.missing_packets_seen, 0);
	atomic_init(&args.resend_requests_seen, 0);
	atomic_init(&args.resend_recovered_seen, 0);
	atomic_init(&args.resend_rejected_seen, 0);
	assert(pthread_create(&thread, NULL, run_receiver, &args) == 0);

	fd = socket(AF_INET, SOCK_STREAM, 0);
	assert(fd >= 0);
	assert(setsockopt(fd, SOL_SOCKET, SO_RCVTIMEO, &timeout,
		sizeof(timeout)) == 0);
	memset(&target, 0, sizeof(target));
	target.sin_family = AF_INET;
	target.sin_addr.s_addr = inet_addr("127.0.0.1");
	target.sin_port = htons(raop_receiver_rtsp_port(&receiver));
	assert(connect(fd, (struct sockaddr *)&target, sizeof(target)) == 0);
	peer_udp_fd = socket(AF_INET, SOCK_DGRAM, 0);
	assert(peer_udp_fd >= 0);
	wrong_udp_fd = socket(AF_INET, SOCK_DGRAM, 0);
	assert(wrong_udp_fd >= 0);
	{
		struct sockaddr_in wrong_peer = {0};
		wrong_peer.sin_family = AF_INET;
		wrong_peer.sin_addr.s_addr = inet_addr("127.0.0.1");
		assert(bind(wrong_udp_fd, (struct sockaddr *)&wrong_peer,
			sizeof(wrong_peer)) == 0);
	}
	memset(&peer_udp, 0, sizeof(peer_udp));
	peer_udp.sin_family = AF_INET;
	peer_udp.sin_addr.s_addr = inet_addr("127.0.0.1");
	assert(bind(peer_udp_fd, (struct sockaddr *)&peer_udp, sizeof(peer_udp)) == 0);
	assert(getsockname(peer_udp_fd, (struct sockaddr *)&peer_udp,
		&peer_udp_length) == 0);
	assert(setsockopt(peer_udp_fd, SOL_SOCKET, SO_RCVTIMEO, &timeout,
		sizeof(timeout)) == 0);

	send_request(fd, "OPTIONS * RTSP/1.0\r\nCSeq: 1\r\n\r\n",
		response, sizeof(response));
	assert(strstr(response, "RTSP/1.0 200 OK") != NULL);
	{
		struct timeval send_timeout = {0, 0};
		socklen_t timeout_length = (socklen_t)sizeof(send_timeout);
		pthread_mutex_lock(&args.receiver_lock);
		assert(receiver.client_fd >= 0);
		assert(getsockopt(receiver.client_fd, SOL_SOCKET, SO_SNDTIMEO,
			&send_timeout, &timeout_length) == 0);
		pthread_mutex_unlock(&args.receiver_lock);
		assert(send_timeout.tv_sec == 2 && send_timeout.tv_usec == 0);
	}
	send_request(fd, announce, response, sizeof(response));
	assert(strstr(response, "RTSP/1.0 200 OK") != NULL);
	snprintf(setup, sizeof(setup), "SETUP rtsp://127.0.0.1/stream RTSP/1.0\r\n"
		"CSeq: 3\r\nTransport: RTP/AVP/UDP;unicast;mode=record;control_port=%u;"
		"timing_port=%u\r\n\r\n", ntohs(peer_udp.sin_port),
		ntohs(peer_udp.sin_port));
	send_request(fd, setup, response, sizeof(response));
	assert(strstr(response, "RTSP/1.0 200 OK") != NULL);
	assert(strstr(response, "server_port=") != NULL);
	send_request(fd, "RECORD rtsp://127.0.0.1/stream RTSP/1.0\r\n"
		"CSeq: 4\r\n\r\n", response, sizeof(response));
	assert(strstr(response, "RTSP/1.0 200 OK") != NULL);
	timing_length = recvfrom(peer_udp_fd, timing_request, sizeof(timing_request),
		0, NULL, NULL);
	assert(timing_length == 32);
	assert(timing_request[0] == 0x80 && timing_request[1] == 0xd2);
	timing_reply[0] = 0x80;
	timing_reply[1] = 0xd3;
	timing_reply[2] = timing_request[2];
	timing_reply[3] = timing_request[3];
	target.sin_port = htons(receiver.timing_port);
	assert(sendto(peer_udp_fd, timing_reply, 8, 0,
		(struct sockaddr *)&target, sizeof(target)) == 8);
	(void)nanosleep(&(struct timespec){0, 50000000L}, NULL);
	assert(sendto(peer_udp_fd, timing_reply, sizeof(timing_reply), 0,
		(struct sockaddr *)&target, sizeof(target)) == (ssize_t)sizeof(timing_reply));
	for (wait = 0; wait < 50 &&
		!atomic_load_explicit(&args.timing_seen, memory_order_acquire); ++wait)
		(void)nanosleep(&(struct timespec){0, 10000000L}, NULL);
	assert(atomic_load_explicit(&args.timing_seen, memory_order_acquire));
	pthread_mutex_lock(&args.receiver_lock);
	assert(receiver.timing_requests >= 1);
	assert(receiver.timing_responses == 1);
	assert(receiver.timing_bad_packets == 1);
	pthread_mutex_unlock(&args.receiver_lock);

	send_rtp(raop_receiver_audio_port(&receiver), 42);
	for (wait = 0; wait < 50 &&
		!atomic_load_explicit(&args.audio_seen, memory_order_acquire); ++wait)
		(void)nanosleep(&(struct timespec){0, 10000000L}, NULL);
	assert(atomic_load_explicit(&args.audio_seen, memory_order_acquire));
	pthread_mutex_lock(&args.receiver_lock);
	assert(receiver.audio_queue.stats.packets == 1);
	assert(receiver.audio_queue.stats.payload_bytes == 4);
	{
		raop_rtp_frame frame;
		assert(raop_receiver_pop_audio(&receiver, &frame));
		assert(frame.packet.sequence == 42);
		assert(frame.packet.payload_length == 4);
		assert(memcmp(frame.payload, "\xde\xad\xbe\xef", 4) == 0);
	}
	pthread_mutex_unlock(&args.receiver_lock);
	send_rtp(raop_receiver_audio_port(&receiver), 45);
	for (wait = 0; wait < 50 &&
		atomic_load_explicit(&args.audio_packets_seen, memory_order_acquire) < 2;
		++wait)
		(void)nanosleep(&(struct timespec){0, 10000000L}, NULL);
	assert(atomic_load_explicit(&args.audio_packets_seen, memory_order_acquire) == 2);
	assert(atomic_load_explicit(&args.missing_packets_seen,
		memory_order_acquire) == 2);
	for (;;) {
		uint8_t incoming[64];
		resend_length = recvfrom(peer_udp_fd, incoming, sizeof(incoming),
			0, NULL, NULL);
		if (resend_length == 8 && incoming[0] == 0x80 && incoming[1] == 0xd5) {
			memcpy(resend_request, incoming, (size_t)resend_length);
			break;
		}
		assert(resend_length == 32 && incoming[0] == 0x80 &&
			incoming[1] == 0xd2);
	}
	assert(resend_request[4] == 0 && resend_request[5] == 43);
	assert(resend_request[6] == 0 && resend_request[7] == 2);
	for (wait = 0; wait < 50 &&
		atomic_load_explicit(&args.resend_requests_seen,
		memory_order_acquire) < 1; ++wait)
		(void)nanosleep(&(struct timespec){0, 10000000L}, NULL);
	assert(atomic_load_explicit(&args.resend_requests_seen,
		memory_order_acquire) == 1);
	send_resend(wrong_udp_fd, receiver.control_port, 43, 1);
	for (wait = 0; wait < 50 &&
		atomic_load_explicit(&args.resend_rejected_seen,
		memory_order_acquire) < 1; ++wait)
		(void)nanosleep(&(struct timespec){0, 10000000L}, NULL);
	assert(atomic_load_explicit(&args.resend_rejected_seen,
		memory_order_acquire) == 1);
	send_resend(peer_udp_fd, receiver.control_port, 100, 1);
	for (wait = 0; wait < 50 &&
		atomic_load_explicit(&args.resend_rejected_seen,
		memory_order_acquire) < 2; ++wait)
		(void)nanosleep(&(struct timespec){0, 10000000L}, NULL);
	assert(atomic_load_explicit(&args.resend_rejected_seen,
		memory_order_acquire) == 2);
	send_resend(peer_udp_fd, receiver.control_port, 43, 0);
	for (wait = 0; wait < 50 &&
		atomic_load_explicit(&args.resend_rejected_seen,
		memory_order_acquire) < 3; ++wait)
		(void)nanosleep(&(struct timespec){0, 10000000L}, NULL);
	assert(atomic_load_explicit(&args.resend_rejected_seen,
		memory_order_acquire) == 3);
	send_resend(peer_udp_fd, receiver.control_port, 43, 1);
	for (wait = 0; wait < 50 &&
		atomic_load_explicit(&args.resend_recovered_seen,
		memory_order_acquire) < 1; ++wait)
		(void)nanosleep(&(struct timespec){0, 10000000L}, NULL);
	assert(atomic_load_explicit(&args.resend_recovered_seen,
		memory_order_acquire) == 1);
	send_resend(peer_udp_fd, receiver.control_port, 43, 1);
	for (wait = 0; wait < 50 &&
		atomic_load_explicit(&args.resend_rejected_seen,
		memory_order_acquire) < 4; ++wait)
		(void)nanosleep(&(struct timespec){0, 10000000L}, NULL);
	assert(atomic_load_explicit(&args.resend_rejected_seen,
		memory_order_acquire) == 4);
	pthread_mutex_lock(&args.receiver_lock);
	{
		raop_rtp_frame frame;
		assert(raop_receiver_pop_audio(&receiver, &frame));
		assert(frame.packet.sequence == 43);
		assert(memcmp(frame.payload, "\xca\xfe\xba\xbe", 4) == 0);
		assert(raop_receiver_pop_audio(&receiver, &frame));
		assert(frame.packet.sequence == 45);
	}
	pthread_mutex_unlock(&args.receiver_lock);
	send_rtp(raop_receiver_audio_port(&receiver), 46);
	for (wait = 0; wait < 50 &&
		atomic_load_explicit(&args.audio_packets_seen, memory_order_acquire) < 3;
		++wait)
		(void)nanosleep(&(struct timespec){0, 10000000L}, NULL);
	assert(atomic_load_explicit(&args.audio_packets_seen,
		memory_order_acquire) == 3);
	pthread_mutex_lock(&args.receiver_lock);
	assert(receiver.audio_queue.count == 1);
	pthread_mutex_unlock(&args.receiver_lock);

	pthread_mutex_lock(&args.receiver_lock);
	send_rtp(raop_receiver_audio_port(&receiver), 47);
	send_empty_datagram(peer_udp_fd, raop_receiver_audio_port(&receiver));
	write_request(fd, "FLUSH rtsp://127.0.0.1/stream RTSP/1.0\r\n"
		"CSeq: 5\r\n\r\n");
	pthread_mutex_unlock(&args.receiver_lock);
	receive_response(fd, "FLUSH", response, sizeof(response));
	assert(strstr(response, "RTSP/1.0 200 OK") != NULL);
	pthread_mutex_lock(&args.receiver_lock);
	assert(receiver.flush_generation == 1);
	assert(receiver.audio_queue.count == 0);
	assert(!receiver.audio_queue.stats.have_sequence);
	assert(receiver.audio_queue.stats.packets == 3);
	pthread_mutex_unlock(&args.receiver_lock);
	send_resend(peer_udp_fd, receiver.control_port, 44, 1);
	for (wait = 0; wait < 50 &&
		atomic_load_explicit(&args.resend_rejected_seen,
		memory_order_acquire) < 5; ++wait)
		(void)nanosleep(&(struct timespec){0, 10000000L}, NULL);
	assert(atomic_load_explicit(&args.resend_rejected_seen,
		memory_order_acquire) == 5);
	send_rtp(raop_receiver_audio_port(&receiver), 1);
	for (wait = 0; wait < 50 &&
		atomic_load_explicit(&args.audio_packets_seen, memory_order_acquire) < 4;
		++wait)
		(void)nanosleep(&(struct timespec){0, 10000000L}, NULL);
	assert(atomic_load_explicit(&args.audio_packets_seen,
		memory_order_acquire) >= 4);
	pthread_mutex_lock(&args.receiver_lock);
	assert(receiver.audio_queue.count == 1);
	assert(receiver.audio_queue.stats.have_sequence);
	assert(receiver.audio_queue.stats.last_sequence == 1);
	pthread_mutex_unlock(&args.receiver_lock);
	send_request(fd, "TEARDOWN rtsp://127.0.0.1/stream RTSP/1.0\r\n"
		"CSeq: 6\r\n\r\n", response, sizeof(response));
	assert(strstr(response, "RTSP/1.0 200 OK") != NULL);
	close(fd);
	for (wait = 0; wait < 100 && receiver_has_client(&args); ++wait)
		(void)nanosleep(&(struct timespec){0, 1000000L}, NULL);
	assert(!receiver_has_client(&args));
	verify_slow_reader_disconnect(&args, raop_receiver_rtsp_port(&receiver));
	close(peer_udp_fd);
	close(wrong_udp_fd);
	atomic_store(&args.stop, 1);
	assert(pthread_join(thread, NULL) == 0);
	pthread_mutex_destroy(&args.receiver_lock);
	raop_receiver_close(&receiver);
	receiver_log_file_close(&log);
	log_read = fopen(log_path, "rb");
	assert(log_read != NULL);
	log_size = fread(log_contents, 1, sizeof(log_contents) - 1, log_read);
	log_contents[log_size] = '\0';
	fclose(log_read);
	assert(strstr(log_contents, "RTP_TIMING_REQUEST") != NULL);
	assert(strstr(log_contents, "RTP_TIMING_BAD") != NULL);
	assert(strstr(log_contents, "RTP_TIMING_REPLY") != NULL);
	assert(strstr(log_contents, "RTP_RESEND_REQUEST") != NULL);
	assert(strstr(log_contents, "RTP_RESEND_RECOVERED") != NULL);
	assert(strstr(log_contents, "RTP_RESEND_REJECTED") != NULL);
	assert(strstr(log_contents,
		"event=RTP_QUEUE_FLUSH value=1 bytes=0") != NULL);
	assert(strstr(log_contents,
		"event=RTP_SOCKET_FLUSH value=2 bytes=0") != NULL);
	assert(strstr(log_contents, "event=RTSP_SEND_TIMEOUT") != NULL);
	assert(unlink(log_path) == 0);
	puts("RAOP persistent RTSP + UDP RTP loopback and bounded slow-reader disconnect: PASS");
	return 0;
}
