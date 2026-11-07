#ifndef PS3_AIRPLAY_RAOP_SDP_H
#define PS3_AIRPLAY_RAOP_SDP_H

#include <stddef.h>
#include <stdint.h>

#define RAOP_SDP_KEY_B64_MAX 512
#define RAOP_SDP_IV_B64_MAX 64
#define RAOP_SDP_FMTP_VALUES 11

typedef struct {
	uint32_t fmtp[RAOP_SDP_FMTP_VALUES];
	char rsa_aes_key[RAOP_SDP_KEY_B64_MAX];
	char aes_iv[RAOP_SDP_IV_B64_MAX];
	size_t rsa_aes_key_length;
	size_t aes_iv_length;
	uint8_t has_version;
	uint8_t has_audio;
	uint8_t has_alac;
	uint8_t has_fmtp;
	uint8_t has_rsa_aes_key;
	uint8_t has_aes_iv;
	uint8_t has_encryption_type;
	uint8_t encryption_type;
} raop_sdp_offer;

/* Parse the bounded SDP subset used by RAOP. Base session/audio lines are
 * required; codec and encryption lines are retained when present. */
int raop_sdp_parse(const char *data, size_t length, raop_sdp_offer *offer);

#endif
