#include "../../src/codec/raop_alac.h"

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static uint8_t *read_file(const char *path, size_t *length)
{
	FILE *file = fopen(path, "rb");
	uint8_t *data;
	long file_length;
	assert(file != NULL);
	assert(fseek(file, 0, SEEK_END) == 0);
	file_length = ftell(file);
	assert(file_length >= 0);
	assert(fseek(file, 0, SEEK_SET) == 0);
	data = malloc((size_t)file_length);
	assert(data != NULL);
	assert(fread(data, 1, (size_t)file_length, file) == (size_t)file_length);
	assert(fclose(file) == 0);
	*length = (size_t)file_length;
	return data;
}

int main(void)
{
	raop_sdp_offer offer = {0};
	raop_alac_decoder *decoder;
	uint8_t *packet;
	uint8_t *expected;
	uint8_t pcm[RAOP_ALAC_PCM_MAX];
	size_t packet_length, expected_length, pcm_length = 0;
	uint32_t samples = 0;
	offer.has_fmtp = 1;
	offer.fmtp[0] = 4096;
	offer.fmtp[1] = 0;
	offer.fmtp[2] = 16;
	offer.fmtp[3] = 40;
	offer.fmtp[4] = 10;
	offer.fmtp[5] = 14;
	offer.fmtp[6] = 2;
	offer.fmtp[7] = 255;
	offer.fmtp[8] = 0;
	offer.fmtp[9] = 0;
	offer.fmtp[10] = 44100;
	packet = read_file("tests/protocol/fixtures/alac-ffmpeg-352.packet", &packet_length);
	expected = read_file("tests/protocol/fixtures/alac-ffmpeg-352.pcm", &expected_length);
	decoder = raop_alac_create(&offer);
	assert(decoder != NULL);
	assert(raop_alac_decode(decoder, packet, packet_length, pcm, sizeof(pcm),
		&pcm_length, &samples));
	assert(samples == 353);
	assert(pcm_length == expected_length);
	assert(memcmp(pcm, expected, expected_length) == 0);
	pcm_length = 0;
	assert(raop_alac_decode(decoder, packet, 1, pcm, sizeof(pcm),
		&pcm_length, &samples) || pcm_length == 0);
	pcm_length = 0;
	assert(raop_alac_decode(decoder, packet, 16, pcm, sizeof(pcm),
		&pcm_length, &samples) || pcm_length == 0);
	assert(!raop_alac_decode(decoder, packet, RAOP_ALAC_INPUT_MAX + 1U,
		pcm, sizeof(pcm), &pcm_length, &samples));
	raop_alac_destroy(decoder);
	offer.fmtp[2] = 24;
	assert(raop_alac_create(&offer) == NULL);
	free(expected);
	free(packet);
	puts("RAOP ALAC decode against FFmpeg PCM: PASS (353 samples)");
	return 0;
}
