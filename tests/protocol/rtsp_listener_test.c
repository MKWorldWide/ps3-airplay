#define _POSIX_C_SOURCE 200809L

#include "../../src/protocol/rtsp_listener.h"

#include <arpa/inet.h>
#include <assert.h>
#include <errno.h>
#include <fcntl.h>
#include <pthread.h>
#include <stdatomic.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <time.h>
#include <unistd.h>

typedef struct {
	rtsp_listener *listener;
	atomic_int stop;
} server_thread_args;

static void *poll_server(void *opaque)
{
	server_thread_args *args = (server_thread_args *)opaque;
	while (atomic_load(&args->stop) == 0)
		(void)rtsp_listener_poll(args->listener, 20);
	return NULL;
}

static size_t send_text(int fd, const char *text)
{
	size_t length = strlen(text);
	size_t sent = 0;
	while (sent < length) {
		ssize_t result = send(fd, text + sent, length - sent, 0);
		assert(result > 0);
		sent += (size_t)result;
	}
	return sent;
}

static size_t exchange(uint16_t port, const char *request, char *response,
	size_t capacity)
{
	struct sockaddr_in address;
	int fd = socket(AF_INET, SOCK_STREAM, 0);
	ssize_t amount;
	size_t total = 0;
	assert(fd >= 0);
	memset(&address, 0, sizeof(address));
	address.sin_family = AF_INET;
	address.sin_addr.s_addr = inet_addr("127.0.0.1");
	address.sin_port = htons(port);
	assert(connect(fd, (struct sockaddr *)&address, sizeof(address)) == 0);
	if (strncmp(request, "OPTIONS", 7) == 0) {
		struct timespec delay = {0, 10000000L};
		const char *split = strstr(request, "CSeq:");
		assert(split != NULL);
		assert(send_text(fd, "OPTIONS * RTSP/1.0\r\n") > 0);
		(void)nanosleep(&delay, NULL);
		(void)send_text(fd, split);
	} else {
		(void)send_text(fd, request);
	}
	while ((amount = recv(fd, response + total, capacity - total - 1, 0)) > 0) {
		total += (size_t)amount;
		if (total >= capacity - 1)
			break;
	}
	response[total] = '\0';
	close(fd);
	return total;
}

static void assert_slow_client_is_bounded(uint16_t port)
{
	struct sockaddr_in address;
	struct timespec delay = {0, 200000000L};
	char byte = 'O';
	char response[256];
	int fd = socket(AF_INET, SOCK_STREAM, 0);
	ssize_t result;
	size_t response_length = 0;
	int closed = 0;
	unsigned int attempt;
	assert(fd >= 0);
	memset(&address, 0, sizeof(address));
	address.sin_family = AF_INET;
	address.sin_addr.s_addr = inet_addr("127.0.0.1");
	address.sin_port = htons(port);
	assert(connect(fd, (struct sockaddr *)&address, sizeof(address)) == 0);
	assert(fcntl(fd, F_SETFL, fcntl(fd, F_GETFL, 0) | O_NONBLOCK) == 0);
	assert(send(fd, &byte, 1, 0) == 1);
	(void)nanosleep(&delay, NULL);
	byte = 'P';
	assert(send(fd, &byte, 1, 0) == 1);
	(void)nanosleep(&delay, NULL);
	byte = 'T';
	assert(send(fd, &byte, 1, 0) == 1);
	/* More than the total header deadline, less than a reset per-byte timeout. */
	delay.tv_nsec = 450000000L;
	(void)nanosleep(&delay, NULL);
	for (attempt = 0; attempt < 10 && !closed; ++attempt) {
		result = recv(fd, response + response_length,
			sizeof(response) - response_length - 1, 0);
		if (result == 0) {
			closed = 1;
		} else if (result > 0) {
			response_length += (size_t)result;
			response[response_length] = '\0';
		} else {
			assert(errno == EAGAIN || errno == EWOULDBLOCK);
			(void)nanosleep(&(struct timespec){0, 10000000L}, NULL);
		}
	}
	assert(closed);
	assert(strstr(response, "RTSP/1.0 400") != NULL);
	close(fd);
}

int main(void)
{
	char path[] = "/tmp/ps3-airplay-log-XXXXXX";
	char response[256];
	char log_contents[RECEIVER_LOG_MAX_BYTES + 1];
	char long_request[512];
	int temporary_fd = mkstemp(path);
	FILE *readback;
	struct stat metadata;
	receiver_log_file log;
	rtsp_listener listener;
	server_thread_args args;
	pthread_t thread;
	unsigned int i;
	size_t amount;

	assert(temporary_fd >= 0);
	close(temporary_fd);
	assert(receiver_log_file_open(&log, path));
	assert(rtsp_listener_open(&listener, 0, &log));
	assert(rtsp_listener_port(&listener) != 0);
	args.listener = &listener;
	atomic_init(&args.stop, 0);
	assert(pthread_create(&thread, NULL, poll_server, &args) == 0);

	(void)exchange(rtsp_listener_port(&listener),
		"OPTIONS * RTSP/1.0\r\nCSeq: 7\r\n\r\n", response,
		sizeof(response));
	assert(strstr(response, "RTSP/1.0 200 OK") != NULL);
	assert(strstr(response, "CSeq: 7") != NULL);

	(void)exchange(rtsp_listener_port(&listener),
		"ANNOUNCE rtsp://receiver/session RTSP/1.0\r\n"
		"CSeq: 8\r\nAuthorization: very-secret-value\r\n\r\n",
		response, sizeof(response));
	assert(strstr(response, "RTSP/1.0 501 Not Implemented") != NULL);
	assert_slow_client_is_bounded(rtsp_listener_port(&listener));

	atomic_store(&args.stop, 1);
	assert(pthread_join(thread, NULL) == 0);
	rtsp_listener_close(&listener);
	receiver_log_file_close(&log);

	readback = fopen(path, "rb");
	assert(readback != NULL);
	amount = fread(log_contents, 1, sizeof(log_contents) - 1, readback);
	log_contents[amount] = '\0';
	assert(fclose(readback) == 0);
	assert(strstr(log_contents, "very-secret-value") == NULL);
	assert(strstr(log_contents, "RTSP_OPTIONS") != NULL);
	assert(strstr(log_contents, "RTSP_UNSUPPORTED") != NULL);

	assert(receiver_log_file_open(&log, path));
	for (i = 0; i < 400; ++i) {
		(void)snprintf(long_request, sizeof(long_request),
			"%04u-11-06T18:00:00Z", i);
		assert(receiver_log_file_event(&log, long_request,
			"RTSP_HEARTBEAT", (int)i, i));
	}
	receiver_log_file_close(&log);

	assert(stat(path, &metadata) == 0);
	assert((size_t)metadata.st_size <= RECEIVER_LOG_MAX_BYTES);
	assert(unlink(path) == 0);
	puts("rtsp listener and capped log: PASS");
	return 0;
}
