#include "../../src/crypto/raop_crypto.h"

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

static const uint8_t key[RAOP_AES_KEY_SIZE] = {
	0x2b, 0x7e, 0x15, 0x16, 0x28, 0xae, 0xd2, 0xa6,
	0xab, 0xf7, 0x15, 0x88, 0x09, 0xcf, 0x4f, 0x3c
};
static const uint8_t iv[RAOP_AES_IV_SIZE] = {
	0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
	0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f
};
static const uint8_t ciphertext[16] = {
	0x76, 0x49, 0xab, 0xac, 0x81, 0x19, 0xb2, 0x46,
	0xce, 0xe9, 0x8e, 0x9b, 0x12, 0xe9, 0x19, 0x7d
};
static const uint8_t plaintext[16] = {
	0x6b, 0xc1, 0xbe, 0xe2, 0x2e, 0x40, 0x9f, 0x96,
	0xe9, 0x3d, 0x7e, 0x11, 0x73, 0x93, 0x17, 0x2a
};

int main(void)
{
	uint8_t decoded[16];
	uint8_t packet[19];
	uint8_t output[19];
	uint8_t aes_key[RAOP_AES_KEY_SIZE];
	uint8_t recovered_key[RAOP_AES_KEY_SIZE];
	uint8_t ciphertext_key[RAOP_AIRPORT_RSA_SIZE];
	uint8_t signature[RAOP_AIRPORT_RSA_SIZE];
	uint8_t challenge[16];
	uint8_t ip[4] = { 192, 168, 1, 25 };
	uint8_t mac[6] = { 0x11, 0x22, 0x33, 0x44, 0x55, 0x66 };
	uint8_t signed_message[32] = { 0 };
	char challenge_b64[25];
	char wrapped_key_b64[RAOP_BASE64_ENCODED_MAX + 1];
	char response[RAOP_APPLE_RESPONSE_MAX];
	char padded_response[RAOP_APPLE_RESPONSE_MAX + 2];
	size_t b64_length = 0;
	raop_crypto_context crypto;
	mbedtls_rsa_context *rsa;
	static const char key_b64[] = "AAECAwQFBgcICQoLDA0ODw==";
	memset(decoded, 0xa5, sizeof(decoded));
	assert(raop_crypto_base64_decode(key_b64, sizeof(key_b64) - 1,
		decoded, sizeof(decoded), sizeof(decoded)));
	for (size_t i = 0; i < sizeof(decoded); ++i)
		assert(decoded[i] == i);
	assert(!raop_crypto_base64_decode("AAECAwQFBgcICQoLDA0ODw=A", 24,
		decoded, sizeof(decoded), sizeof(decoded)));
	assert(!raop_crypto_base64_decode(key_b64, sizeof(key_b64) - 1,
		decoded, sizeof(decoded) - 1, sizeof(decoded)));
	memset(decoded, 0xa5, sizeof(decoded));
	assert(!raop_crypto_base64_decode("AB==", 4, decoded, sizeof(decoded), 1));
	for (size_t i = 0; i < sizeof(decoded); ++i)
		assert(decoded[i] == 0);
	memset(decoded, 0xa5, sizeof(decoded));
	assert(!raop_crypto_base64_decode("AAB=", 4, decoded, sizeof(decoded), 2));
	for (size_t i = 0; i < sizeof(decoded); ++i)
		assert(decoded[i] == 0);
	assert(!raop_crypto_base64_decode("", 0, decoded, sizeof(decoded), 0));

	memcpy(packet, ciphertext, sizeof(ciphertext));
	packet[16] = 0xa1; packet[17] = 0xb2; packet[18] = 0xc3;
	assert(raop_crypto_decrypt_audio(key, iv, packet, sizeof(packet),
		output, sizeof(output)));
	assert(memcmp(output, plaintext, sizeof(plaintext)) == 0);
	assert(output[16] == 0xa1 && output[17] == 0xb2 && output[18] == 0xc3);

	memcpy(output, ciphertext, sizeof(ciphertext));
	assert(raop_crypto_decrypt_audio(key, iv, output, sizeof(ciphertext),
		output, sizeof(ciphertext)));
	assert(memcmp(output, plaintext, sizeof(plaintext)) == 0);
	assert(!raop_crypto_decrypt_audio(key, iv, packet, 0, output,
		sizeof(output)));
	assert(!raop_crypto_decrypt_audio(key, iv, packet, sizeof(packet), output,
		sizeof(output) - 1));

	assert(raop_crypto_context_init(&crypto, host_rng, NULL));
	rsa = mbedtls_pk_rsa(crypto.private_key);
	assert(rsa != NULL);
	memset(challenge, 0x07, sizeof(challenge));
	assert(mbedtls_base64_encode((unsigned char *)challenge_b64,
		sizeof(challenge_b64), &b64_length, challenge, sizeof(challenge)) == 0);
	challenge_b64[b64_length] = '\0';
	assert(raop_crypto_decode_challenge(challenge_b64, b64_length, decoded));
	assert(memcmp(decoded, challenge, sizeof(challenge)) == 0);
	assert(raop_crypto_decode_challenge(challenge_b64, b64_length - 2, decoded));
	assert(memcmp(decoded, challenge, sizeof(challenge)) == 0);
	assert(!raop_crypto_decode_challenge("%%%", 3, decoded));

	for (size_t i = 0; i < sizeof(aes_key); ++i)
		aes_key[i] = (uint8_t)(i * 7U + 3U);
	mbedtls_rsa_set_padding(rsa, MBEDTLS_RSA_PKCS_V21, MBEDTLS_MD_SHA1);
	assert(mbedtls_rsa_rsaes_oaep_encrypt(rsa, host_rng, NULL, NULL, 0,
		sizeof(aes_key), aes_key, ciphertext_key) == 0);
	assert(mbedtls_base64_encode((unsigned char *)wrapped_key_b64,
		sizeof(wrapped_key_b64), &b64_length, ciphertext_key,
		sizeof(ciphertext_key)) == 0);
	wrapped_key_b64[b64_length] = '\0';
	assert(raop_crypto_decrypt_session_key(&crypto, wrapped_key_b64,
		b64_length, host_rng, NULL, recovered_key));
	assert(memcmp(recovered_key, aes_key, sizeof(aes_key)) == 0);
	memset(recovered_key, 0xa5, sizeof(recovered_key));
	assert(!raop_crypto_decrypt_session_key(&crypto, "invalid", 7,
		host_rng, NULL, recovered_key));
	for (size_t i = 0; i < sizeof(recovered_key); ++i)
		assert(recovered_key[i] == 0);

	assert(raop_crypto_apple_response(&crypto, challenge, ip, mac,
		host_rng, NULL, response, sizeof(response)));
	assert(strlen(response) == 342);
	assert(strchr(response, '=') == NULL);
	memcpy(padded_response, response, strlen(response));
	padded_response[342] = '=';
	padded_response[343] = '=';
	padded_response[344] = '\0';
	assert(raop_crypto_base64_decode(padded_response, 344, signature,
		sizeof(signature), sizeof(signature)));
	memcpy(signed_message, challenge, sizeof(challenge));
	memcpy(signed_message + 16, ip, sizeof(ip));
	memcpy(signed_message + 20, mac, sizeof(mac));
	mbedtls_rsa_set_padding(rsa, MBEDTLS_RSA_PKCS_V15, MBEDTLS_MD_NONE);
	assert(mbedtls_rsa_pkcs1_verify(rsa, MBEDTLS_MD_NONE,
		sizeof(signed_message), signed_message, signature) == 0);
	raop_crypto_context_free(&crypto);

	puts("RAOP base64, AES-CBC, AirPort RSA-OAEP and Apple-Response: PASS");
	return 0;
}
