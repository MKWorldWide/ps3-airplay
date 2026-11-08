#define _POSIX_C_SOURCE 200809L

#include "../../src/codec/raop_alac.h"
#include "../../src/crypto/raop_crypto.h"
#include "../../src/protocol/raop_receiver.h"
#include "../../src/audio/pcm_ring.h"
#include "../../src/audio/pcm_spsc.h"

#include <arpa/inet.h>
#include <assert.h>
#include <errno.h>
#include <fcntl.h>
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <time.h>
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
	raop_pcm_spsc decoded_queue;
	struct sockaddr_in address;
	uint8_t pcm[RAOP_ALAC_PCM_MAX];
	int16_t queued_pcm[RAOP_ALAC_PCM_MAX / sizeof(int16_t)];
	float output[256U * 2U];
	uint8_t rtp_packet[12 + RAOP_RTP_PAYLOAD_MAX] = {0x80, 96};
	char sdp[1024], request[2048], response[RAOP_SESSION_RESPONSE_MAX];
	int client_fd, sender_fd, request_length;
	size_t pcm_length = 0;
	size_t queued_frames;
	size_t rendered_frames;
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
	assert(receiver.decrypted_audio_packets == 1);
	assert(frame.packet.sequence == 1 && frame.packet.payload_length == encoded_length);
	assert(memcmp(frame.payload, encoded, encoded_length) == 0);
	decoder = raop_alac_create(&receiver.session.offer);
	assert(decoder != NULL);
	assert(raop_alac_decode(decoder, frame.payload, frame.packet.payload_length,
		pcm, sizeof(pcm), &pcm_length, &samples));
	assert(samples == 353 && pcm_length == expected_length);
	assert(memcmp(pcm, expected, expected_length) == 0);
	raop_pcm_spsc_init(&decoded_queue);
	assert(raop_pcm_spsc_push_le16(&decoded_queue, pcm, pcm_length, 2, 1) ==
		samples);
	queued_frames = raop_pcm_spsc_pop(&decoded_queue, queued_pcm, samples, 1);
	assert(queued_frames == samples);
	raop_pcm_ring_init(&ring, 2);
	assert(raop_pcm_ring_push_native(&ring, queued_pcm, queued_frames) == samples);
	assert(ring.frames_written == samples && ring.count == samples);
	rendered_frames = raop_pcm_ring_render_float(&ring, 44100, 48000,
		output, 256);
	assert(rendered_frames == 256 && ring.output_underflows == 0);
	{
		float peak = 0.0f;
		for (size_t i = 0; i < rendered_frames * 2U; ++i) {
			float magnitude = output[i] < 0.0f ? -output[i] : output[i];
			if (magnitude > peak)
				peak = magnitude;
		}
		assert(peak > 0.01f);
	}
	raop_alac_destroy(decoder);
	close(sender_fd);
	close(client_fd);
	raop_receiver_close(&receiver);
	puts("RAOP RTSP + encrypted RTP + ALAC + SPSC + 44.1-to-48 kHz output: PASS");
}

static void receive_rtsp_response(int fd, char *response, size_t capacity)
{
	size_t total = 0;
	struct timeval timeout = {3, 0};
	assert(setsockopt(fd, SOL_SOCKET, SO_RCVTIMEO, &timeout,
		sizeof(timeout)) == 0);
	while (total + 1 < capacity) {
		ssize_t amount = recv(fd, response + total,
			capacity - total - 1, 0);
		assert(amount > 0);
		total += (size_t)amount;
		response[total] = '\0';
		if (strstr(response, "\r\n\r\n") != NULL)
			return;
	}
	assert(0 && "RTSP response exceeded probe buffer");
}

static void send_remote_request(int fd, const char *request,
	char *response, size_t response_capacity)
{
	send_all_fd(fd, request, strlen(request));
	receive_rtsp_response(fd, response, response_capacity);
	assert(strstr(response, "RTSP/1.0 200 OK") != NULL);
}

static void verify_remote_audio_pipeline(const char *host, uint16_t rtsp_port,
	uint16_t audio_port, const char *wrapped_b64, const char *iv_b64,
	const uint8_t *encrypted_rtp, size_t rtp_length)
{
	struct sockaddr_in control_address, audio_address;
	struct addrinfo hints, *resolved = NULL;
	struct timespec settle = {2, 0};
	char sdp[1024], request[2048], response[RAOP_SESSION_RESPONSE_MAX];
	char service[6];
	unsigned int packet_index;
	int client_fd, sender_fd, request_length;
	memset(&control_address, 0, sizeof(control_address));
	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;
	snprintf(service, sizeof(service), "%u", (unsigned int)rtsp_port);
	assert(getaddrinfo(host, service, &hints, &resolved) == 0);
	assert(resolved != NULL && resolved->ai_addrlen == sizeof(control_address));
	memcpy(&control_address, resolved->ai_addr, sizeof(control_address));
	freeaddrinfo(resolved);
	client_fd = socket(AF_INET, SOCK_STREAM, 0);
	assert(client_fd >= 0);
	assert(connect(client_fd, (struct sockaddr *)&control_address,
		sizeof(control_address)) == 0);
	send_remote_request(client_fd,
		"OPTIONS * RTSP/1.0\r\nCSeq: 1\r\n\r\n",
		response, sizeof(response));
	snprintf(sdp, sizeof(sdp),
		"v=0\r\n"
		"m=audio 0 RTP/AVP 96\r\n"
		"a=rtpmap:96 AppleLossless/44100/2\r\n"
		"a=fmtp:96 4096 0 16 40 10 14 2 255 0 0 44100\r\n"
		"a=rsaaeskey:%s\r\na=aesiv:%s\r\na=et:1\r\n",
		wrapped_b64, iv_b64);
	request_length = snprintf(request, sizeof(request),
		"ANNOUNCE rtsp://receiver/stream RTSP/1.0\r\n"
		"CSeq: 2\r\nContent-Type: application/sdp\r\n"
		"Content-Length: %zu\r\n\r\n%s", strlen(sdp), sdp);
	assert(request_length > 0 && (size_t)request_length < sizeof(request));
	send_remote_request(client_fd, request, response, sizeof(response));
	request_length = snprintf(request, sizeof(request),
		"SETUP rtsp://receiver/stream RTSP/1.0\r\nCSeq: 3\r\n"
		"Transport: RTP/AVP/UDP;unicast;mode=record;"
		"control_port=40000;timing_port=40001\r\n\r\n");
	assert(request_length > 0 && (size_t)request_length < sizeof(request));
	send_remote_request(client_fd, request, response, sizeof(response));
	send_remote_request(client_fd,
		"RECORD rtsp://receiver/stream RTSP/1.0\r\nCSeq: 4\r\n\r\n",
		response, sizeof(response));
	sender_fd = socket(AF_INET, SOCK_DGRAM, 0);
	assert(sender_fd >= 0);
	memset(&audio_address, 0, sizeof(audio_address));
	audio_address.sin_family = AF_INET;
	audio_address.sin_port = htons(audio_port);
	audio_address.sin_addr = control_address.sin_addr;
	for (packet_index = 0; packet_index < 128; ++packet_index) {
		uint8_t packet[12 + RAOP_RTP_PAYLOAD_MAX];
		ssize_t sent;
		assert(rtp_length <= sizeof(packet));
		memcpy(packet, encrypted_rtp, rtp_length);
		packet[2] = (uint8_t)((packet_index + 1U) >> 8);
		packet[3] = (uint8_t)(packet_index + 1U);
		packet[4] = (uint8_t)((packet_index * 353U) >> 24);
		packet[5] = (uint8_t)((packet_index * 353U) >> 16);
		packet[6] = (uint8_t)((packet_index * 353U) >> 8);
		packet[7] = (uint8_t)(packet_index * 353U);
		sent = sendto(sender_fd, packet, rtp_length, 0,
			(struct sockaddr *)&audio_address, sizeof(audio_address));
		assert(sent == (ssize_t)rtp_length);
	}
	(void)nanosleep(&settle, NULL);
	send_remote_request(client_fd,
		"TEARDOWN rtsp://receiver/stream RTSP/1.0\r\nCSeq: 5\r\n\r\n",
		response, sizeof(response));
	close(sender_fd);
	close(client_fd);
	puts("RPCS3 RAOP ANNOUNCE/SETUP/RECORD + 128 RTP fixture packets: PASS");
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

int main(int argc, char **argv)
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
	const char *remote_host = NULL;
	uint16_t remote_rtsp_port = 0, remote_audio_port = 0;
	if (argc != 1 && argc != 4) {
		fprintf(stderr, "usage: %s [--remote IPV4 RTSP_PORT]\n", argv[0]);
		return 2;
	}
	if (argc == 4) {
		char *end = NULL;
		unsigned long port;
		if (strcmp(argv[1], "--remote") != 0) {
			fprintf(stderr, "unknown option\n");
			return 2;
		}
		remote_host = argv[2];
		port = strtoul(argv[3], &end, 10);
		if (end == argv[3] || *end != '\0' || port == 0 || port > 65535) {
			fprintf(stderr, "invalid remote RTSP port\n");
			return 2;
		}
		remote_rtsp_port = (uint16_t)port;
		remote_audio_port = 6000;
	}

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
	if (remote_host != NULL)
		verify_remote_audio_pipeline(remote_host, remote_rtsp_port,
			remote_audio_port, wrapped_b64, iv_b64, rtp,
			12 + encoded_length);
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
	/* Incompressible stereo frame crosses the old 1400-byte limit and has
	 * an 8-byte clear AES tail. Exercise queue -> decrypt -> decode -> PCM. */
	{
		uint8_t *noise, *reference;
		size_t noise_length, reference_length, encrypted_length;
		noise = read_file("tests/protocol/fixtures/alac-noise-352.packet", &noise_length);
		reference = read_file("tests/protocol/fixtures/alac-noise-352.pcm", &reference_length);
		assert(noise_length == 1416);
		memcpy(rtp + 12, noise, noise_length);
		encrypted_length = noise_length & ~(size_t)15;
		memcpy(packet_iv, aes_iv, sizeof(packet_iv));
		mbedtls_aes_init(&aes);
		assert(mbedtls_aes_setkey_enc(&aes, aes_key, 128) == 0);
		assert(mbedtls_aes_crypt_cbc(&aes, MBEDTLS_AES_ENCRYPT, encrypted_length,
			packet_iv, noise, rtp + 12) == 0);
		mbedtls_aes_free(&aes);
		raop_rtp_queue_flush(&receiver.audio_queue);
		assert(raop_rtp_queue_push(&receiver.audio_queue, rtp, 12 + noise_length));
		assert(raop_receiver_pop_audio(&receiver, &frame));
		decoder = raop_alac_create(&receiver.session.offer);
		assert(decoder != NULL);
		assert(raop_alac_decode(decoder, frame.payload, noise_length, pcm,
			sizeof(pcm), &pcm_length, &samples));
		assert(samples == 352 && pcm_length == reference_length);
		assert(memcmp(pcm, reference, reference_length) == 0);
		raop_alac_destroy(decoder);free(noise);free(reference);
		puts("1416-byte encrypted ALAC with clear AES tail: exact PCM PASS");
	}
	raop_crypto_context_free(&crypto);
	free(encoded);
	free(expected);
	puts("RAOP encrypted RTP payload through receiver decrypt and ALAC PCM: PASS");
	return 0;
}
