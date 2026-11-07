#ifndef PS3_AIRPLAY_RAOP_SESSION_H
#define PS3_AIRPLAY_RAOP_SESSION_H

#include <stddef.h>
#include <stdint.h>
#include "raop_sdp.h"
#include "../crypto/raop_crypto.h"

#define RAOP_SESSION_REQUEST_MAX 8192
#define RAOP_SESSION_RESPONSE_MAX 768
#define RAOP_SESSION_SDP_MAX 4096

typedef enum {
	RAOP_SESSION_IDLE = 0,
	RAOP_SESSION_ANNOUNCED,
	RAOP_SESSION_SETUP,
	RAOP_SESSION_RECORDING,
	RAOP_SESSION_PAUSED
} raop_session_state;

typedef struct {
	raop_session_state state;
	uint32_t requests;
	uint32_t volume_updates;
	int32_t volume_millidb;
	float volume_gain;
	uint8_t volume_muted;
	uint16_t audio_port;
	uint16_t control_port;
	uint16_t timing_port;
	uint16_t peer_control_port;
	uint16_t peer_timing_port;
	raop_crypto_context *crypto;
	mbedtls_f_rng_t *rng;
	void *rng_context;
	uint8_t local_ipv4[4];
	uint8_t mac[6];
	uint8_t crypto_configured;
	uint8_t has_audio_key;
	uint8_t audio_key[RAOP_AES_KEY_SIZE];
	uint8_t audio_iv[RAOP_AES_IV_SIZE];
	raop_sdp_offer offer;
} raop_session;

void raop_session_init(raop_session *session, uint16_t audio_port,
	uint16_t control_port, uint16_t timing_port);
int raop_session_configure_crypto(raop_session *session,
	raop_crypto_context *crypto, mbedtls_f_rng_t *rng, void *rng_context,
	const uint8_t local_ipv4[4], const uint8_t mac[6]);
/* Decrypts a queued RAOP audio payload in place when this session is encrypted. */
int raop_session_decrypt_audio(const raop_session *session,
	uint8_t *payload, size_t payload_length);

/*
 * Handle one complete bounded RTSP message, including any Content-Length body.
 * Request content is parsed in-place and never copied into the response or logs.
 * Supports a deliberately limited classic RAOP control sequence, including
 * Apple-Challenge replies and ANNOUNCE key negotiation when crypto is set up.
 * ALAC decoding remains a separate layer.
 * Returns 1 when a response was produced, otherwise 0 for malformed input.
 */
int raop_session_handle(raop_session *session, const char *request,
	size_t request_length, char *response, size_t response_capacity);

#endif
