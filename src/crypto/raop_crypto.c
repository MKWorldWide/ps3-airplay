#include "raop_crypto.h"

#include <string.h>

#include <mbedtls/aes.h>
#include <mbedtls/base64.h>
#include <mbedtls/md.h>
#include <mbedtls/pk.h>
#include <mbedtls/platform_util.h>
#include <mbedtls/rsa.h>

#include "airport_private_key.h"

#if defined(RAOP_USE_PSL1GHT)
#include <lv2/system.h>
#define RAOP_RANDOM_NUMBER_MAX_SIZE RANDOM_NUMBER_MAX_SIZE
#elif defined(__powerpc64__)
#include <sys/random_number.h>
#define RAOP_RANDOM_NUMBER_MAX_SIZE RANDOM_NUMBER_MAX_SIZE
#endif

static int base64_character(unsigned char c)
{
	return (c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z') ||
		(c >= '0' && c <= '9') || c == '+' || c == '/';
}

static unsigned int base64_value(unsigned char c)
{
	if (c >= 'A' && c <= 'Z')
		return (unsigned int)(c - 'A');
	if (c >= 'a' && c <= 'z')
		return (unsigned int)(c - 'a') + 26U;
	if (c >= '0' && c <= '9')
		return (unsigned int)(c - '0') + 52U;
	return c == '+' ? 62U : 63U;
}

static int reject_base64(uint8_t *decoded, size_t decoded_capacity)
{
	if (decoded != NULL)
		mbedtls_platform_zeroize(decoded, decoded_capacity);
	return 0;
}

int raop_crypto_base64_decode(const char *encoded, size_t encoded_length,
	uint8_t *decoded, size_t decoded_capacity, size_t expected_length)
{
	size_t i;
	size_t output_length = 0;
	size_t padding = 0;
	int result;
	if (encoded == NULL || decoded == NULL || encoded_length == 0 ||
		encoded_length > RAOP_BASE64_ENCODED_MAX ||
		(encoded_length & 3U) != 0 || expected_length > decoded_capacity ||
		expected_length > RAOP_BASE64_DECODED_MAX ||
		encoded_length != 4U * ((expected_length + 2U) / 3U))
		return reject_base64(decoded, decoded_capacity);
	if (encoded[encoded_length - 1] == '=') {
		padding = 1;
		if (encoded_length > 1 && encoded[encoded_length - 2] == '=')
			padding = 2;
	}
	for (i = 0; i < encoded_length; ++i) {
		if (i < encoded_length - padding) {
			if (!base64_character((unsigned char)encoded[i]))
				return reject_base64(decoded, decoded_capacity);
		} else if (encoded[i] != '=') {
			return reject_base64(decoded, decoded_capacity);
		}
	}
	if ((padding == 1 && (expected_length % 3U) != 2U) ||
		(padding == 2 && (expected_length % 3U) != 1U) ||
		(padding == 0 && (expected_length % 3U) != 0U))
		return reject_base64(decoded, decoded_capacity);
	/* Require canonical unused bits, not just a decodable base64 spelling. */
	if ((padding == 1 &&
		(base64_value((unsigned char)encoded[encoded_length - 2]) & 3U) != 0) ||
		(padding == 2 &&
		(base64_value((unsigned char)encoded[encoded_length - 3]) & 15U) != 0))
		return reject_base64(decoded, decoded_capacity);
	result = mbedtls_base64_decode(decoded, decoded_capacity, &output_length,
		(const unsigned char *)encoded, encoded_length);
	if (result != 0 || output_length != expected_length) {
		return reject_base64(decoded, decoded_capacity);
	}
	return 1;
}

int raop_crypto_decrypt_audio(const uint8_t key[RAOP_AES_KEY_SIZE],
	const uint8_t iv[RAOP_AES_IV_SIZE], const uint8_t *payload,
	size_t payload_length, uint8_t *plaintext, size_t plaintext_capacity)
{
	mbedtls_aes_context aes;
	uint8_t packet_iv[RAOP_AES_IV_SIZE];
	size_t encrypted_length;
	int result;
	if (key == NULL || iv == NULL || payload == NULL || plaintext == NULL ||
	payload_length == 0 || payload_length > RAOP_AUDIO_PAYLOAD_MAX ||
	plaintext_capacity < payload_length)
		return 0;
	memmove(plaintext, payload, payload_length);
	encrypted_length = payload_length & ~(size_t)(RAOP_AES_IV_SIZE - 1U);
	if (encrypted_length == 0)
		return 1;
	memcpy(packet_iv, iv, sizeof(packet_iv));
	mbedtls_aes_init(&aes);
	result = mbedtls_aes_setkey_dec(&aes, key, 128);
	if (result == 0)
		result = mbedtls_aes_crypt_cbc(&aes, MBEDTLS_AES_DECRYPT,
			encrypted_length, packet_iv, plaintext, plaintext);
	mbedtls_aes_free(&aes);
	mbedtls_platform_zeroize(packet_iv, sizeof(packet_iv));
	return result == 0;
}

int raop_crypto_context_init(raop_crypto_context *context,
	mbedtls_f_rng_t *rng, void *rng_context)
{
	int result;
	if (context == NULL || rng == NULL)
		return 0;
	memset(context, 0, sizeof(*context));
	mbedtls_pk_init(&context->private_key);
	result = mbedtls_pk_parse_key(&context->private_key,
		raop_airport_private_key_pem, sizeof(raop_airport_private_key_pem),
		NULL, 0, rng, rng_context);
	if (result != 0 ||
		!mbedtls_pk_can_do(&context->private_key, MBEDTLS_PK_RSA) ||
		mbedtls_pk_get_bitlen(&context->private_key) != 2048U) {
		mbedtls_pk_free(&context->private_key);
		mbedtls_platform_zeroize(context, sizeof(*context));
		return 0;
	}
	context->initialized = 1;
	return 1;
}

void raop_crypto_context_free(raop_crypto_context *context)
{
	if (context == NULL)
		return;
	mbedtls_pk_free(&context->private_key);
	mbedtls_platform_zeroize(context, sizeof(*context));
}

int raop_crypto_decode_challenge(const char *encoded, size_t encoded_length,
	uint8_t challenge[16])
{
	char padded[24];
	if (encoded == NULL || challenge == NULL ||
		(encoded_length != 22 && encoded_length != 24)) {
		if (challenge != NULL)
			mbedtls_platform_zeroize(challenge, 16);
		return 0;
	}
	if (encoded_length == 24)
		return raop_crypto_base64_decode(encoded, encoded_length, challenge,
			16, 16);
	memcpy(padded, encoded, 22);
	padded[22] = '=';
	padded[23] = '=';
	return raop_crypto_base64_decode(padded, sizeof(padded), challenge, 16, 16);
}

int raop_crypto_decrypt_session_key(raop_crypto_context *context,
	const char *encoded_key, size_t encoded_key_length,
	mbedtls_f_rng_t *rng, void *rng_context,
	uint8_t aes_key[RAOP_AES_KEY_SIZE])
{
	uint8_t ciphertext[RAOP_AIRPORT_RSA_SIZE];
	uint8_t plaintext[RAOP_AIRPORT_RSA_SIZE];
	mbedtls_rsa_context *rsa;
	size_t plaintext_length = 0;
	int result;
	if (aes_key != NULL)
		mbedtls_platform_zeroize(aes_key, RAOP_AES_KEY_SIZE);
	if (context == NULL || !context->initialized || encoded_key == NULL ||
		rng == NULL || aes_key == NULL ||
		!raop_crypto_base64_decode(encoded_key, encoded_key_length,
			ciphertext, sizeof(ciphertext), sizeof(ciphertext)))
		return 0;
	rsa = mbedtls_pk_rsa(context->private_key);
	if (rsa == NULL) {
		mbedtls_platform_zeroize(ciphertext, sizeof(ciphertext));
		return 0;
	}
	mbedtls_rsa_set_padding(rsa, MBEDTLS_RSA_PKCS_V21, MBEDTLS_MD_SHA1);
	result = mbedtls_rsa_rsaes_oaep_decrypt(rsa, rng, rng_context,
		NULL, 0, &plaintext_length, ciphertext, plaintext, sizeof(plaintext));
	if (result == 0 && plaintext_length == RAOP_AES_KEY_SIZE)
		memcpy(aes_key, plaintext, RAOP_AES_KEY_SIZE);
	else
		result = -1;
	mbedtls_platform_zeroize(ciphertext, sizeof(ciphertext));
	mbedtls_platform_zeroize(plaintext, sizeof(plaintext));
	return result == 0;
}

int raop_crypto_apple_response(raop_crypto_context *context,
	const uint8_t challenge[16], const uint8_t ipv4[4], const uint8_t mac[6],
	mbedtls_f_rng_t *rng, void *rng_context,
	char *response, size_t response_capacity)
{
	uint8_t message[32] = { 0 };
	uint8_t signature[RAOP_AIRPORT_RSA_SIZE];
	char encoded_signature[345];
	mbedtls_rsa_context *rsa;
	size_t encoded_length = 0;
	int result;
	if (response != NULL && response_capacity != 0)
		response[0] = '\0';
	if (context == NULL || !context->initialized || challenge == NULL ||
		ipv4 == NULL || mac == NULL || rng == NULL || response == NULL ||
		response_capacity < RAOP_APPLE_RESPONSE_MAX)
		return 0;
	memcpy(message, challenge, 16);
	memcpy(message + 16, ipv4, 4);
	memcpy(message + 20, mac, 6);
	rsa = mbedtls_pk_rsa(context->private_key);
	if (rsa == NULL)
		return 0;
	mbedtls_rsa_set_padding(rsa, MBEDTLS_RSA_PKCS_V15, MBEDTLS_MD_NONE);
	result = mbedtls_rsa_pkcs1_sign(rsa, rng, rng_context, MBEDTLS_MD_NONE,
		(unsigned int)sizeof(message), message, signature);
	if (result == 0)
		result = mbedtls_base64_encode((unsigned char *)encoded_signature,
			sizeof(encoded_signature), &encoded_length, signature, sizeof(signature));
	if (result == 0) {
		while (encoded_length != 0 && encoded_signature[encoded_length - 1] == '=')
			--encoded_length;
		if (encoded_length + 1 > response_capacity)
			result = -1;
		else {
			memcpy(response, encoded_signature, encoded_length);
			response[encoded_length] = '\0';
		}
	}
	mbedtls_platform_zeroize(message, sizeof(message));
	mbedtls_platform_zeroize(signature, sizeof(signature));
	mbedtls_platform_zeroize(encoded_signature, sizeof(encoded_signature));
	if (result != 0) {
		response[0] = '\0';
		return 0;
	}
	return encoded_length == 342;
}

#if defined(__powerpc64__)
int raop_crypto_ps3_rng(void *unused, unsigned char *output, size_t length)
{
	(void)unused;
	if (output == NULL && length != 0)
		return -1;
	while (length != 0) {
#if defined(RAOP_USE_PSL1GHT)
		uint64_t chunk = length > RAOP_RANDOM_NUMBER_MAX_SIZE ?
			RAOP_RANDOM_NUMBER_MAX_SIZE : (uint64_t)length;
		if (sysGetRandomNumber(output, chunk) != 0)
			return -1;
#else
		uint64_t chunk = length > RAOP_RANDOM_NUMBER_MAX_SIZE ?
			RAOP_RANDOM_NUMBER_MAX_SIZE : (uint64_t)length;
		if (sys_get_random_number(output, chunk) != 0)
			return -1;
#endif
		output += (size_t)chunk;
		length -= (size_t)chunk;
	}
	return 0;
}
#endif
