#ifndef PS3_AIRPLAY_RAOP_RECEIVER_H
#define PS3_AIRPLAY_RAOP_RECEIVER_H

#include <stddef.h>
#include <stdint.h>
#include <sys/socket.h>
#include <netinet/in.h>

#include "raop_rtp.h"
#include "raop_session.h"
#include "../observability/receiver_log.h"

typedef struct {
	int tcp_fd;
	int client_fd;
	int audio_fd;
	int control_fd;
	int timing_fd;
	uint16_t rtsp_port;
	uint16_t audio_port;
	uint16_t control_port;
	uint16_t timing_port;
	struct sockaddr_in peer_address;
	socklen_t peer_length;
	char request[RAOP_SESSION_REQUEST_MAX];
	size_t request_length;
	uint64_t request_started_ms;
	uint64_t client_last_activity_ms;
	raop_session session;
	raop_crypto_context *crypto;
	mbedtls_f_rng_t *rng;
	void *rng_context;
	uint8_t mac[6];
	uint8_t crypto_configured;
	raop_rtp_queue audio_queue;
	receiver_log_file *log;
	uint64_t control_packets;
	uint64_t timing_packets;
	uint64_t timing_requests;
	uint64_t timing_responses;
	uint64_t timing_bad_packets;
	uint64_t timing_next_send_ms;
	uint64_t timing_outstanding_ms;
	uint16_t timing_sequence;
	uint16_t timing_outstanding_sequence;
	uint16_t timing_last_rtt_ms;
	uint16_t timing_max_rtt_ms;
	uint8_t timing_outstanding;
	uint16_t resend_sequence;
	uint16_t resend_first_sequence;
	uint16_t resend_packet_count;
	uint16_t resend_received_mask;
	uint8_t resend_range_valid;
	uint64_t resend_requests;
	uint64_t resend_failures;
	uint64_t resend_recovered;
	uint64_t resend_rejected;
	uint64_t resend_last_send_ms;
	uint64_t last_audio_log_packets;
} raop_receiver;

/* Opens one TCP control endpoint and three bounded UDP endpoints. */
int raop_receiver_open(raop_receiver *receiver, uint16_t rtsp_port,
	uint16_t rtp_base_port, receiver_log_file *log);
/* Configure crypto and MAC for future accepted clients; local IPv4 is taken
 * from each accepted TCP socket. */
int raop_receiver_configure_crypto(raop_receiver *receiver,
	raop_crypto_context *crypto, mbedtls_f_rng_t *rng, void *rng_context,
	const uint8_t mac[6]);
/* Polls control/data sockets for at most timeout_ms; one bounded batch per call. */
int raop_receiver_poll(raop_receiver *receiver, unsigned int timeout_ms);
uint16_t raop_receiver_rtsp_port(const raop_receiver *receiver);
uint16_t raop_receiver_audio_port(const raop_receiver *receiver);
int raop_receiver_pop_audio(raop_receiver *receiver, raop_rtp_frame *frame);
void raop_receiver_close(raop_receiver *receiver);

#endif
