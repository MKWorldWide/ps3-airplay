#define _POSIX_C_SOURCE 200809L

#include "../../src/codec/raop_alac.h"
#include "../../src/crypto/raop_crypto.h"
#include "../../src/protocol/raop_receiver.h"
#include "../../src/audio/pcm_ring.h"

#include <arpa/inet.h>
#include <assert.h>
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

#include <mbedtls/aes.h>
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

static uint8_t *read_file(const char *path, size_t *length)
{
	FILE *file = fopen(path, "rb");
	uint8_t *data;
	long file_length;
	assert(file != NULL);
	assert(fseek(file, 0, SEEK_END) == 0);
	file_length = ftell(file);
	assert(file_length > 0);
	assert(fseek(file, 0, SEEK_SET) == 0);
	data = malloc((size_t)file_length);
	assert(data != NULL);
	assert(fread(data, 1, (size_t)file_length, file) == (size_t)file_length);
	assert(fclose(file) == 0);
	*length = (size_t)file_length;
	return data;
}

static void write_bits(uint8_t *buffer, size_t bit_offset, unsigned int bit_count,
	uint32_t value)
{
	unsigned int i;
	for (i = 0; i < bit_count; ++i) {
		size_t position = bit_offset + i;
		uint8_t mask = (uint8_t)(1U << (7U - (position & 7U)));
		unsigned int shift = bit_count - i - 1U;
		if (((value >> shift) & 1U) != 0)
			buffer[position >> 3] |= mask;
		else
			buffer[position >> 3] &= (uint8_t)~mask;
	}
}

static uint32_t next_random(uint32_t *state)
{
	uint32_t value = *state;
	value ^= value << 13;
	value ^= value >> 17;
	value ^= value << 5;
	*state = value;
	return value;
}

static void send_all_fd(int fd, const char *data, size_t length)
{
	size_t sent = 0;
	while (sent < length) {
		ssize_t amount = send(fd, data + sent, length - sent, 0);
		assert(amount > 0);
		sent += (size_t)amount;
	}
}

static void send_request_and_pump(raop_receiver *receiver, int fd,
	const char *request, char *response, size_t response_capacity)
{
	size_t total = 0;
	unsigned int attempt;
	send_all_fd(fd, request, strlen(request));
	for (attempt = 0; attempt < 100 && total + 1 < response_capacity;
		++attempt) {
		ssize_t amount;
		(void)raop_receiver_poll(receiver, 20);
		amount = recv(fd, response + total, response_capacity - total - 1,
			MSG_DONTWAIT);
		if (amount > 0) {
			total += (size_t)amount;
			response[total] = '\0';
			if (strstr(response, "\r\n\r\n") != NULL)
				return;
		} else if (amount < 0 && errno != EAGAIN && errno != EWOULDBLOCK &&
			errno != EINTR) {
			assert(0 && "RTSP socket receive failed");
		}
	}
	assert(0 && "timed out waiting for RTSP response");
}

static void verify_socket_audio_pipeline(raop_crypto_context *crypto,
	const uint8_t mac[6], const char *wrapped_b64, const char *iv_b64,
	const uint8_t *encoded, size_t encoded_length, const uint8_t *expected,
	size_t expected_length, const uint8_t *encrypted_rtp, size_t rtp_length)
{
	raop_receiver receiver = {0};
	raop_rtp_frame frame;
	raop_alac_decoder *decoder;
	raop_pcm_ring ring;
	struct sockaddr_in address;
	uint8_t pcm[RAOP_ALAC_PCM_MAX];
	uint8_t rtp_packet[12 + RAOP_RTP_PAYLOAD_MAX] = {0x80, 96};
	char sdp[1024], request[2048], response[RAOP_SESSION_RESPONSE_MAX];
	int client_fd, sender_fd, request_length;
	size_t pcm_length = 0;
	uint32_t samples = 0;
	unsigned int attempt;
	static const char setup_transport[] =
		"RTP/AVP/UDP;unicast;mode=record;control_port=40000;timing_port=40001";

	assert(encoded_length == rtp_length - 12U);
	assert(raop_receiver_open(&receiver, 0, 0, NULL));
	assert(raop_receiver_configure_crypto(&receiver, crypto, host_rng, NULL, mac));
	client_fd = socket(AF_INET, SOCK_STREAM, 0);
	assert(client_fd >= 0);
	memset(&address, 0, sizeof(address));
	address.sin_family = AF_INET;
	address.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
	address.sin_port = htons(raop_receiver_rtsp_port(&receiver));
	assert(connect(client_fd, (struct sockaddr *)&address, sizeof(address)) == 0);
	(void)raop_receiver_poll(&receiver, 20); /* accept the TCP control peer */

	send_request_and_pump(&receiver, client_fd,
		"OPTIONS * RTSP/1.0\r\nCSeq: 1\r\n\r\n", response, sizeof(response));
	assert(strstr(response, "RTSP/1.0 200 OK") != NULL);
	assert(strstr(response, "Public: OPTIONS, ANNOUNCE") != NULL);

	snprintf(sdp, sizeof(sdp),
		"v=0\r\n"
		"m=audio 0 RTP/AVP 96\r\n"
		"a=rtpmap:96 AppleLossless/44100/2\r\n"
		"a=fmtp:96 4096 0 16 40 10 14 2 255 0 0 44100\r\n"
		"a=rsaaeskey:%s\r\na=aesiv:%s\r\na=et:1\r\n",
		wrapped_b64, iv_b64);
	request_length = snprintf(request, sizeof(request),
		"ANNOUNCE rtsp://127.0.0.1/stream RTSP/1.0\r\n"
		"CSeq: 2\r\nContent-Type: application/sdp\r\n"
		"Content-Length: %zu\r\n\r\n%s", strlen(sdp), sdp);
	assert(request_length > 0 && (size_t)request_length < sizeof(request));
	{
		size_t first_part = (size_t)request_length / 2U;
		send_all_fd(client_fd, request, first_part);
		(void)raop_receiver_poll(&receiver, 20);
		send_all_fd(client_fd, request + first_part,
			(size_t)request_length - first_part);
		response[0] = '\0';
		{
			size_t total = 0;
			unsigned int poll;
			for (poll = 0; poll < 100 && total + 1 < sizeof(response); ++poll) {
				ssize_t amount;
				(void)raop_receiver_poll(&receiver, 20);
				amount = recv(client_fd, response + total,
					sizeof(response) - total - 1, MSG_DONTWAIT);
				if (amount > 0) {
					total += (size_t)amount;
					response[total] = '\0';
					if (strstr(response, "\r\n\r\n") != NULL)
						break;
				} else if (amount < 0 && errno != EAGAIN &&
					errno != EWOULDBLOCK && errno != EINTR) {
					assert(0 && "fragmented ANNOUNCE receive failed");
				}
			}
			assert(strstr(response, "RTSP/1.0 200 OK") != NULL);
		}
	}
	assert(receiver.session.has_audio_key && receiver.session.offer.has_fmtp);

	request_length = snprintf(request, sizeof(request),
		"SETUP rtsp://127.0.0.1/stream RTSP/1.0\r\nCSeq: 3\r\n"
		"Transport: %s\r\n\r\n", setup_transport);
	assert(request_length > 0 && (size_t)request_length < sizeof(request));
	send_request_and_pump(&receiver, client_fd, request, response, sizeof(response));
	assert(strstr(response, "RTSP/1.0 200 OK") != NULL);
	assert(strstr(response, "server_port=") != NULL);
	send_request_and_pump(&receiver, client_fd,
		"RECORD rtsp://127.0.0.1/stream RTSP/1.0\r\nCSeq: 4\r\n\r\n",
		response, sizeof(response));
	assert(strstr(response, "RTSP/1.0 200 OK") != NULL);
	assert(receiver.session.state == RAOP_SESSION_RECORDING);

	memcpy(rtp_packet + 12, encrypted_rtp + 12, encoded_length);
	memcpy(rtp_packet, encrypted_rtp, 12);
	sender_fd = socket(AF_INET, SOCK_DGRAM, 0);
	assert(sender_fd >= 0);
	address.sin_port = htons(raop_receiver_audio_port(&receiver));
	assert(sendto(sender_fd, rtp_packet, rtp_length, 0,
		(struct sockaddr *)&address, sizeof(address)) == (ssize_t)rtp_length);
	for (attempt = 0; attempt < 100 && receiver.audio_queue.count == 0;
		++attempt)
		(void)raop_receiver_poll(&receiver, 20);
	assert(receiver.audio_queue.count == 1);
	assert(raop_receiver_pop_audio(&receiver, &frame));
	assert(frame.packet.sequence == 1 && frame.packet.payload_length == encoded_length);
	assert(memcmp(frame.payload, encoded, encoded_length) == 0);
	decoder = raop_alac_create(&receiver.session.offer);
	assert(decoder != NULL);
	assert(raop_alac_decode(decoder, frame.payload, frame.packet.payload_length,
		pcm, sizeof(pcm), &pcm_length, &samples));
	assert(samples == 353 && pcm_length == expected_length);
	assert(memcmp(pcm, expected, expected_length) == 0);
	raop_pcm_ring_init(&ring, 2);
	assert(raop_pcm_ring_push_le16(&ring, pcm, pcm_length) == samples);
	assert(ring.frames_written == samples && ring.count == samples);
	raop_alac_destroy(decoder);
	close(sender_fd);
	close(client_fd);
	raop_receiver_close(&receiver);
	puts("RAOP TCP RTSP + UDP RTP + decrypt + ALAC + PCM ring loopback: PASS");
}

static void malformed_smoke(raop_alac_decoder *decoder,
	const uint8_t *valid_packet, size_t valid_packet_length)
{
	uint32_t state = 0x00c0ffeeU;
	uint8_t input[RAOP_ALAC_INPUT_MAX];
	uint8_t output[RAOP_ALAC_PCM_MAX];
	unsigned int case_index;
	assert(valid_packet != NULL && valid_packet_length >= 8 &&
		valid_packet_length <= sizeof(input));
	for (case_index = 0; case_index < 4096; ++case_index) {
		size_t length = valid_packet_length;
		size_t bit_count = length * 8U;
		unsigned int mutations = 1U + next_random(&state) % 8U;
		unsigned int mutation;
		memcpy(input, valid_packet, length);
		/* Preserve the fixed 54-bit ALAC frame header; mutate the compressed
		 * payload so the sanitizer reaches entropy and predictor decoding. */
		for (mutation = 0; mutation < mutations; ++mutation) {
			size_t bit = 54U + next_random(&state) % (bit_count - 54U);
			input[bit >> 3] ^= (uint8_t)(1U << (7U - (bit & 7U)));
		}
		if ((case_index & 3U) == 0)
			length = 8U + next_random(&state) % (valid_packet_length - 7U);
		{
			size_t pcm_length;
			uint32_t samples;
			(void)raop_alac_decode(decoder, input, length, output,
				sizeof(output), &pcm_length, &samples);
		}
	}
	for (case_index = 0; case_index < 256; ++case_index) {
		size_t length = 32U +
			(next_random(&state) % (RAOP_ALAC_INPUT_MAX - 31U));
		size_t i;
		for (i = 0; i < length; ++i)
			input[i] = (uint8_t)next_random(&state);
		{
			size_t pcm_length;
			uint32_t samples;
			(void)raop_alac_decode(decoder, input, length, output,
				sizeof(output), &pcm_length, &samples);
		}
	}
}

int main(void)
{
	static const uint8_t aes_key[RAOP_AES_KEY_SIZE] = {
		0x2b, 0x7e, 0x15, 0x16, 0x28, 0xae, 0xd2, 0xa6,
		0xab, 0xf7, 0x15, 0x88, 0x09, 0xcf, 0x4f, 0x3c
	};
	static const uint8_t aes_iv[RAOP_AES_IV_SIZE] = {
		0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
		0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f
	};
	static const uint8_t ipv4[4] = {192, 0, 2, 10};
	static const uint8_t mac[6] = {0x02, 0, 0, 0, 0, 1};
	raop_crypto_context crypto;
	raop_receiver receiver = {0};
	raop_alac_decoder *decoder;
	raop_alac_decoder *limited_decoder;
	mbedtls_rsa_context *rsa;
	mbedtls_aes_context aes;
	raop_rtp_frame frame;
	raop_sdp_offer limited_offer;
	uint8_t wrapped_key[RAOP_AIRPORT_RSA_SIZE];
	uint8_t *encoded, *expected, *actual;
	uint8_t malformed[RAOP_ALAC_INPUT_MAX];
	uint8_t packet_iv[RAOP_AES_IV_SIZE];
	uint8_t rtp[12 + RAOP_RTP_PAYLOAD_MAX] = {0x80, 96};
	uint8_t pcm[RAOP_ALAC_PCM_MAX];
	char wrapped_b64[RAOP_BASE64_ENCODED_MAX + 1];
	char iv_b64[25], sdp[1024], request[2048];
	char response[RAOP_SESSION_RESPONSE_MAX];
	size_t encoded_length, expected_length, wrapped_length = 0, iv_length = 0;
	size_t pcm_length = 0;
	uint32_t samples = 0;
	int request_length;

	encoded = read_file("tests/protocol/fixtures/alac-ffmpeg-352.packet",
		&encoded_length);
	expected = read_file("tests/protocol/fixtures/alac-ffmpeg-352.pcm",
		&expected_length);
	assert(encoded_length <= RAOP_RTP_PAYLOAD_MAX);
	assert((encoded_length % RAOP_AES_IV_SIZE) == 0);
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
	assert(mbedtls_base64_encode((unsigned char *)iv_b64, sizeof(iv_b64),
		&iv_length, aes_iv, sizeof(aes_iv)) == 0);
	iv_b64[iv_length] = '\0';

	mbedtls_aes_init(&aes);
	assert(mbedtls_aes_setkey_enc(&aes, aes_key, 128) == 0);
	memcpy(packet_iv, aes_iv, sizeof(packet_iv));
	assert(mbedtls_aes_crypt_cbc(&aes, MBEDTLS_AES_ENCRYPT, encoded_length,
		packet_iv, encoded, rtp + 12) == 0);
	mbedtls_aes_free(&aes);
	rtp[2] = 0;
	rtp[3] = 1;
	rtp[7] = 1;
	rtp[11] = 1;
	verify_socket_audio_pipeline(&crypto, mac, wrapped_b64, iv_b64,
		encoded, encoded_length, expected, expected_length, rtp,
		12 + encoded_length);

	raop_session_init(&receiver.session, 6000, 6001, 6002);
	assert(raop_session_configure_crypto(&receiver.session, &crypto, host_rng,
		NULL, ipv4, mac));
	snprintf(sdp, sizeof(sdp),
		"v=0\r\nm=audio 0 RTP/AVP 96\r\n"
		"a=rtpmap:96 AppleLossless/44100/2\r\n"
		"a=fmtp:96 4096 0 16 40 10 14 2 255 0 0 44100\r\n"
		"a=rsaaeskey:%s\r\na=aesiv:%s\r\na=et:1\r\n",
		wrapped_b64, iv_b64);
	request_length = snprintf(request, sizeof(request),
		"ANNOUNCE rtsp://receiver/stream RTSP/1.0\r\nCSeq: 1\r\n"
		"Content-Type: application/sdp\r\nContent-Length: %zu\r\n\r\n%s",
		strlen(sdp), sdp);
	assert(request_length > 0 && (size_t)request_length < sizeof(request));
	assert(raop_session_handle(&receiver.session, request,
		(size_t)request_length, response, sizeof(response)));
	assert(strstr(response, "RTSP/1.0 200 OK") != NULL);
	assert(receiver.session.has_audio_key);
	assert(receiver.session.offer.has_fmtp);

	assert(raop_rtp_queue_push(&receiver.audio_queue, rtp,
		12 + encoded_length));
	assert(raop_receiver_pop_audio(&receiver, &frame));
	assert(frame.packet.sequence == 1);
	assert(frame.packet.payload_length == encoded_length);
	assert(memcmp(frame.payload, encoded, encoded_length) == 0);

	decoder = raop_alac_create(&receiver.session.offer);
	assert(decoder != NULL);
	actual = pcm;
	assert(raop_alac_decode(decoder, frame.payload, frame.packet.payload_length,
		actual, sizeof(pcm), &pcm_length, &samples));
	assert(samples == 353);
	assert(pcm_length == expected_length);
	assert(memcmp(actual, expected, expected_length) == 0);

	raop_alac_destroy(decoder);
	limited_offer = receiver.session.offer;
	limited_offer.fmtp[0] = 352;
	limited_decoder = raop_alac_create(&limited_offer);
	assert(limited_decoder != NULL);
	assert(!raop_alac_decode(limited_decoder, frame.payload,
		frame.packet.payload_length, pcm, sizeof(pcm), &pcm_length, &samples));
	memcpy(malformed, frame.payload, frame.packet.payload_length);
	write_bits(malformed, 22, 32, UINT32_MAX);
	assert(!raop_alac_decode(limited_decoder, malformed,
		frame.packet.payload_length, pcm, sizeof(pcm), &pcm_length, &samples));
	raop_alac_destroy(limited_decoder);
	puts("ALAC frame sample count bounded, including UINT32_MAX: PASS");
	decoder = raop_alac_create(&receiver.session.offer);
	assert(decoder != NULL);
	malformed_smoke(decoder, frame.payload, frame.packet.payload_length);
	raop_alac_destroy(decoder);
	puts("ALAC deterministic sanitizer smoke: PASS (4096 seeded mutations + 256 random packets)");
	raop_crypto_context_free(&crypto);
	free(encoded);
	free(expected);
	puts("RAOP encrypted RTP payload through receiver decrypt and ALAC PCM: PASS");
	return 0;
}
