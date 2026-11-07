#ifndef PS3_AIRPLAY_RTSP_LISTENER_H
#define PS3_AIRPLAY_RTSP_LISTENER_H

#include <stdint.h>

#include "../observability/receiver_log.h"

#define RTSP_LISTENER_CLIENT_TIMEOUT_MS 750

typedef struct {
	int socket_fd;
	uint16_t port;
	receiver_log_file *log;
} rtsp_listener;

/* Network initialization is owned by the caller. Port 0 selects an OS port. */
int rtsp_listener_open(rtsp_listener *listener, uint16_t port,
	receiver_log_file *log);

/* Poll for one client. Returns 1 after handling a request, 0 on timeout, -1 on error. */
int rtsp_listener_poll(rtsp_listener *listener, unsigned int timeout_ms);
uint16_t rtsp_listener_port(const rtsp_listener *listener);
void rtsp_listener_close(rtsp_listener *listener);

#endif
