#ifndef PS3_AIRPLAY_RAOP_CRYPTO_H
#define PS3_AIRPLAY_RAOP_CRYPTO_H

#include <stddef.h>
#include <stdint.h>
#include "../audio/raop_audio_limits.h"
#include <mbedtls/pk.h>

#define RAOP_AES_KEY_SIZE 16
#define RAOP_AES_IV_SIZE 16
#define RAOP_AUDIO_PAYLOAD_MAX RAOP_ENCODED_AUDIO_MAX
#define RAOP_BASE64_ENCODED_MAX 512
#define RAOP_BASE64_DECODED_MAX (RAOP_BASE64_ENCODED_MAX / 4 * 3)
#define RAOP_AIRPORT_RSA_SIZE 256
#define RAOP_APPLE_RESPONSE_MAX 343

typedef struct {
	mbedtls_pk_context private_key;
	uint8_t initialized;
} raop_crypto_context;

/* Load the fixed AirPort Express receiver key using the caller's RNG for
 * private-key blinding. The same context may be reused serially. */
int raop_crypto_context_init(raop_crypto_context *context,
	mbedtls_f_rng_t *rng, void *rng_context);
void raop_crypto_context_free(raop_crypto_context *context);

/* Decode the fixed 16-byte Apple-Challenge in canonical padded or unpadded
 * standard base64 form. */
int raop_crypto_decode_challenge(const char *encoded, size_t encoded_length,
	uint8_t challenge[16]);

/* Recover the 16-byte AES key from SDP rsaaeskey (RSA-OAEP/SHA-1). */
int raop_crypto_decrypt_session_key(raop_crypto_context *context,
	const char *encoded_key, size_t encoded_key_length,
	mbedtls_f_rng_t *rng, void *rng_context,
	uint8_t aes_key[RAOP_AES_KEY_SIZE]);

/* Build the classic IPv4 Apple-Response, base64-encoded without padding. */
int raop_crypto_apple_response(raop_crypto_context *context,
	const uint8_t challenge[16], const uint8_t ipv4[4], const uint8_t mac[6],
	mbedtls_f_rng_t *rng, void *rng_context,
	char *response, size_t response_capacity);

#if defined(__powerpc64__)
/* PS3DK system RNG adapter for production RSA private operations. */
int raop_crypto_ps3_rng(void *unused, unsigned char *output, size_t length);
#endif

/* Strictly decode canonical padded base64 into a caller-owned fixed buffer. */
int raop_crypto_base64_decode(const char *encoded, size_t encoded_length,
	uint8_t *decoded, size_t decoded_capacity, size_t expected_length);

/* Decrypt full AES-128-CBC blocks with a fresh copy of the RAOP IV per packet.
 * The incomplete trailing block is transmitted in clear and copied unchanged. */
int raop_crypto_decrypt_audio(const uint8_t key[RAOP_AES_KEY_SIZE],
	const uint8_t iv[RAOP_AES_IV_SIZE], const uint8_t *payload,
	size_t payload_length, uint8_t *plaintext, size_t plaintext_capacity);

#endif
