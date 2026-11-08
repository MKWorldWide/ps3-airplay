#ifndef PS3_AIRPLAY_RAOP_ALAC_H
#define PS3_AIRPLAY_RAOP_ALAC_H

#include <stddef.h>
#include <stdint.h>
#include "../audio/raop_audio_limits.h"

#include "../protocol/raop_sdp.h"

#define RAOP_ALAC_FRAME_MAX 4096
#define RAOP_ALAC_INPUT_MAX RAOP_ENCODED_AUDIO_MAX
#define RAOP_ALAC_PCM_MAX (RAOP_ALAC_FRAME_MAX * 2U * sizeof(int16_t))

typedef struct raop_alac_decoder raop_alac_decoder;

/* Supports the bounded 16-bit mono/stereo AirPlay ALAC subset. PCM is
 * returned as signed 16-bit little-endian interleaved samples. */
raop_alac_decoder *raop_alac_create(const raop_sdp_offer *offer);
int raop_alac_decode(raop_alac_decoder *decoder, const uint8_t *packet,
	size_t packet_length, uint8_t *pcm, size_t pcm_capacity,
	size_t *pcm_length, uint32_t *sample_count);
void raop_alac_destroy(raop_alac_decoder *decoder);

#endif
