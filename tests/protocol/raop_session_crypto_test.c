#include "../../src/crypto/raop_crypto.h"
#include "../../src/protocol/raop_session.h"

#include <assert.h>
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include <mbedtls/base64.h>
#include <mbedtls/md.h>
#include <mbedtls/rsa.h>

static int host_rng(void *context, unsigned char *output, size_t length)
{
	int fd;
	size_t offset = 0;
	(void)context;
	fd = open("/dev/urandom", O_RDONLY);
	if (fd < 0)
		return -1;
	while (offset < length) {
		ssize_t count = read(fd, output + offset, length - offset);
		if (count < 0 && errno == EINTR)
			continue;
		if (count <= 0) {
			(void)close(fd);
			return -1;
		}
		offset += (size_t)count;
	}
	(void)close(fd);
	return 0;
}

static int make_request(raop_session *session, const char *request,
	char *response, size_t response_capacity)
{
	return raop_session_handle(session, request, strlen(request), response,
		response_capacity);
}

typedef struct {
	unsigned int sdp_started;
	unsigned int sdp_completed;
	unsigned int key_started;
	unsigned int key_completed;
	unsigned int validation_completed;
	unsigned int content_type_check_completed;
	unsigned int response_started;
	unsigned int response_completed;
	int key_result;
} diagnostic_counts;

static void collect_diagnostic(void *context, const char *event, int value,
	size_t bytes)
{
	diagnostic_counts *counts = context;
	(void)bytes;
	if (strcmp(event, "ANNOUNCE_SDP_PARSE_START") == 0)
		++counts->sdp_started;
	else if (strcmp(event, "ANNOUNCE_SDP_PARSE_DONE") == 0)
		++counts->sdp_completed;
	else if (strcmp(event, "ANNOUNCE_KEY_UNWRAP_START") == 0)
		++counts->key_started;
	else if (strcmp(event, "ANNOUNCE_KEY_UNWRAP_DONE") == 0) {
		++counts->key_completed;
		counts->key_result = value;
	} else if (strcmp(event, "ANNOUNCE_VALIDATE_DONE") == 0)
		++counts->validation_completed;
	else if (strcmp(event, "ANNOUNCE_CONTENT_TYPE_CHECK_DONE") == 0 && value == 1)
		++counts->content_type_check_completed;
	else if (strcmp(event, "ANNOUNCE_RESPONSE_BUILD_START") == 0)
		++counts->response_started;
	else if (strcmp(event, "ANNOUNCE_RESPONSE_BUILD_DONE") == 0)
		++counts->response_completed;
}

int main(void)
{
	static const uint8_t aes_key[16] = {
		0x2b, 0x7e, 0x15, 0x16, 0x28, 0xae, 0xd2, 0xa6,
		0xab, 0xf7, 0x15, 0x88, 0x09, 0xcf, 0x4f, 0x3c
	};
	static const uint8_t aes_iv[16] = {
		0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
		0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f
	};
	static const uint8_t encrypted_audio[16] = {
		0x76, 0x49, 0xab, 0xac, 0x81, 0x19, 0xb2, 0x46,
		0xce, 0xe9, 0x8e, 0x9b, 0x12, 0xe9, 0x19, 0x7d
	};
	static const uint8_t clear_audio[16] = {
		0x6b, 0xc1, 0xbe, 0xe2, 0x2e, 0x40, 0x9f, 0x96,
		0xe9, 0x3d, 0x7e, 0x11, 0x73, 0x93, 0x17, 0x2a
	};
	uint8_t wrapped_key[RAOP_AIRPORT_RSA_SIZE];
	uint8_t audio[sizeof(encrypted_audio)];
	uint8_t ipv4[4] = { 192, 0, 2, 25 };
	uint8_t mac[6] = { 0x02, 0x00, 0x00, 0x00, 0x00, 0x01 };
	char wrapped_b64[RAOP_BASE64_ENCODED_MAX + 1];
	char iv_b64[25];
	char challenge_b64[25];
	char sdp[1024];
	char request[2048];
	char response[RAOP_SESSION_RESPONSE_MAX];
	size_t wrapped_length = 0, iv_length = 0, challenge_length = 0;
	int length;
	mbedtls_rsa_context *rsa;
	raop_crypto_context crypto;
	raop_session session;
	diagnostic_counts diagnostics = {0};

	assert(raop_crypto_context_init(&crypto, host_rng, NULL));
	rsa = mbedtls_pk_rsa(crypto.private_key);
	assert(rsa != NULL);
	mbedtls_rsa_set_padding(rsa, MBEDTLS_RSA_PKCS_V21, MBEDTLS_MD_SHA1);
	assert(mbedtls_rsa_rsaes_oaep_encrypt(rsa, host_rng, NULL, NULL, 0,
		sizeof(aes_key), aes_key, wrapped_key) == 0);
	assert(mbedtls_base64_encode((unsigned char *)wrapped_b64,
		sizeof(wrapped_b64) - 1, &wrapped_length, wrapped_key,
		sizeof(wrapped_key)) == 0);
	wrapped_b64[wrapped_length] = '\0';
	assert(mbedtls_base64_encode((unsigned char *)iv_b64,
		sizeof(iv_b64), &iv_length, aes_iv, sizeof(aes_iv)) == 0);
	iv_b64[iv_length] = '\0';
	assert(mbedtls_base64_encode((unsigned char *)challenge_b64,
		sizeof(challenge_b64), &challenge_length,
		aes_iv, sizeof(aes_iv)) == 0);
	challenge_b64[challenge_length] = '\0';

	raop_session_init(&session, 6000, 6001, 6002);
	raop_session_set_diagnostic(&session, collect_diagnostic, &diagnostics);
	assert(raop_session_configure_crypto(&session, &crypto, host_rng, NULL,
		ipv4, mac));
	snprintf(request, sizeof(request),
		"OPTIONS rtsp://ps3/stream RTSP/1.0\r\nCSeq: 1\r\n"
		"Apple-Challenge: %s\r\n\r\n", challenge_b64);
	assert(make_request(&session, request, response, sizeof(response)));
	assert(strstr(response, "RTSP/1.0 200 OK") != NULL);
	assert(strstr(response, "Apple-Response: ") != NULL);

	length = snprintf(sdp, sizeof(sdp),
		"v=0\r\nm=audio 0 RTP/AVP 96\r\n"
		"a=rsaaeskey:%s\r\na=aesiv:%s\r\na=et:1\r\n",
		wrapped_b64, iv_b64);
	assert(length > 0 && (size_t)length < sizeof(sdp));
	snprintf(request, sizeof(request),
		"ANNOUNCE rtsp://ps3/stream RTSP/1.0\r\nCSeq: 2\r\n"
		"Content-Type: application/sdp\r\nContent-Length: %d\r\n\r\n%s",
		length, sdp);
	assert(make_request(&session, request, response, sizeof(response)));
	assert(strstr(response, "RTSP/1.0 200 OK") != NULL);
	assert(session.state == RAOP_SESSION_ANNOUNCED && session.has_audio_key);
	assert(diagnostics.sdp_started == 1 && diagnostics.sdp_completed == 1);
	assert(diagnostics.key_started == 1 && diagnostics.key_completed == 1 &&
		diagnostics.key_result == 1);
	assert(diagnostics.validation_completed == 1 &&
		diagnostics.content_type_check_completed == 1 &&
		diagnostics.response_started == 1 &&
		diagnostics.response_completed == 1);

	snprintf(request, sizeof(request),
		"SETUP rtsp://ps3/stream RTSP/1.0\r\nCSeq: 3\r\n"
		"Transport: RTP/AVP/UDP;unicast;mode=record;control_port=5001;"
		"timing_port=5002\r\n\r\n");
	assert(make_request(&session, request, response, sizeof(response)));
	snprintf(request, sizeof(request),
		"RECORD rtsp://ps3/stream RTSP/1.0\r\nCSeq: 4\r\n\r\n");
	assert(make_request(&session, request, response, sizeof(response)));
	assert(session.state == RAOP_SESSION_RECORDING);
	memcpy(audio, encrypted_audio, sizeof(audio));
	assert(raop_session_decrypt_audio(&session, audio, sizeof(audio)));
	assert(memcmp(audio, clear_audio, sizeof(audio)) == 0);

	snprintf(request, sizeof(request),
		"TEARDOWN rtsp://ps3/stream RTSP/1.0\r\nCSeq: 5\r\n\r\n");
	assert(make_request(&session, request, response, sizeof(response)));
	assert(!session.has_audio_key && session.audio_key[0] == 0 &&
		session.audio_iv[0] == 0);
	raop_crypto_context_free(&crypto);

	puts("RAOP RTSP challenge, ANNOUNCE key recovery and RTP decrypt: PASS");
	return 0;
}
