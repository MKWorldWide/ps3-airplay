#define _POSIX_C_SOURCE 200809L

#include "rtsp_listener.h"

#include "rtsp_diagnostic.h"

#include <errno.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <time.h>

#if defined(__powerpc64__)
#include <sys/socket.h>
#include <sys/select.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#else
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <unistd.h>
#endif

static void close_socket(int socket_fd)
{
#if defined(__powerpc64__)
	(void)socketclose(socket_fd);
#else
	(void)close(socket_fd);
#endif
}

static int wait_readable(int socket_fd, unsigned int timeout_ms)
{
	fd_set read_set;
	struct timeval timeout;
	int result;
	FD_ZERO(&read_set);
	FD_SET(socket_fd, &read_set);
	timeout.tv_sec = (long)(timeout_ms / 1000U);
	timeout.tv_usec = (long)(timeout_ms % 1000U) * 1000L;
	do {
		result = select(socket_fd + 1, &read_set, NULL, NULL, &timeout);
	} while (result < 0 && errno == EINTR);
	return result;
}

static int header_complete(const char *data, size_t length)
{
	size_t i;
	if (length < 4)
		return 0;
	for (i = 0; i + 3 < length; ++i) {
		if (data[i] == '\r' && data[i + 1] == '\n' &&
			data[i + 2] == '\r' && data[i + 3] == '\n')
			return 1;
	}
	return 0;
}

static void make_timestamp(char timestamp[21])
{
	time_t now = time(NULL);
	struct tm *utc = gmtime(&now);
	if (utc == NULL || strftime(timestamp, 21, "%Y-%m-%dT%H:%M:%SZ", utc) == 0)
		memcpy(timestamp, "1970-01-01T00:00:00Z", 21);
}

static uint64_t monotonic_milliseconds(void)
{
	struct timespec now;
	if (clock_gettime(CLOCK_MONOTONIC, &now) != 0)
		return 0;
	return (uint64_t)now.tv_sec * 1000U + (uint64_t)now.tv_nsec / 1000000U;
}

static const char *event_for(const rtsp_diag_result *result)
{
	if (result->status == 200 && strcmp(result->method, "OPTIONS") == 0)
		return "RTSP_OPTIONS";
	if (result->status == 501)
		return "RTSP_UNSUPPORTED";
	return "RTSP_BAD_REQUEST";
}

static int send_all(int socket_fd, const char *data, size_t length)
{
	size_t sent = 0;
	while (sent < length) {
		int amount = (int)send(socket_fd, data + sent, length - sent, 0);
		if (amount < 0 && errno == EINTR)
			continue;
		if (amount <= 0)
			return 0;
		sent += (size_t)amount;
	}
	return 1;
}

static int serve_client(rtsp_listener *listener, int client_fd)
{
	char request[RTSP_DIAG_REQUEST_MAX];
	char response[256];
	char timestamp[21];
	rtsp_diag_result result;
	size_t length = 0;
	int handled = 0;
	uint64_t deadline = monotonic_milliseconds() +
		RTSP_LISTENER_CLIENT_TIMEOUT_MS;

	while (length < sizeof(request) && !header_complete(request, length)) {
		uint64_t now = monotonic_milliseconds();
		unsigned int remaining;
		int ready;
		int received;
		if (now == 0 || now >= deadline)
			break;
		remaining = (unsigned int)(deadline - now);
		ready = wait_readable(client_fd, remaining);
		if (ready <= 0)
			break;
		received = (int)recv(client_fd, request + length,
			sizeof(request) - length, 0);
		if (received < 0 && errno == EINTR)
			continue;
		if (received <= 0)
			break;
		length += (size_t)received;
	}

	if (length > 0 && rtsp_diag_handle(request, length, response,
		sizeof(response), &result)) {
		make_timestamp(timestamp);
		if (listener->log != NULL)
			(void)receiver_log_file_event(listener->log, timestamp,
				event_for(&result), result.status, length);
		handled = send_all(client_fd, response, strlen(response));
	}
	close_socket(client_fd);
	return handled;
}

int rtsp_listener_open(rtsp_listener *listener, uint16_t port,
	receiver_log_file *log)
{
	struct sockaddr_in address;
	socklen_t address_length = (socklen_t)sizeof(address);
	int reuse = 1;
	int fd;
	if (listener == NULL)
		return 0;
	memset(listener, 0, sizeof(*listener));
	listener->socket_fd = -1;
	fd = socket(AF_INET, SOCK_STREAM, 0);
	if (fd < 0)
		return 0;
	(void)setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse));
	memset(&address, 0, sizeof(address));
	address.sin_family = AF_INET;
	address.sin_addr.s_addr = htonl(INADDR_ANY);
	address.sin_port = htons(port);
	if (bind(fd, (struct sockaddr *)&address, sizeof(address)) < 0 ||
		listen(fd, 1) < 0 ||
		getsockname(fd, (struct sockaddr *)&address, &address_length) < 0) {
		close_socket(fd);
		return 0;
	}
	listener->socket_fd = fd;
	listener->port = ntohs(address.sin_port);
	listener->log = log;
	return 1;
}

int rtsp_listener_poll(rtsp_listener *listener, unsigned int timeout_ms)
{
	int ready;
	int client_fd;
	if (listener == NULL || listener->socket_fd < 0)
		return -1;
	ready = wait_readable(listener->socket_fd, timeout_ms);
	if (ready <= 0)
		return ready;
	client_fd = accept(listener->socket_fd, NULL, NULL);
	if (client_fd < 0)
		return errno == EINTR ? 0 : -1;
	return serve_client(listener, client_fd) ? 1 : -1;
}

uint16_t rtsp_listener_port(const rtsp_listener *listener)
{
	return listener == NULL ? 0 : listener->port;
}

void rtsp_listener_close(rtsp_listener *listener)
{
	if (listener == NULL)
		return;
	if (listener->socket_fd >= 0)
		close_socket(listener->socket_fd);
	listener->socket_fd = -1;
	listener->port = 0;
	listener->log = NULL;
}
