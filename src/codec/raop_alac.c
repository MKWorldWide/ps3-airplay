#include "raop_alac.h"

#include "alac.h"

#include <stdlib.h>
#include <string.h>

#define ALAC_INPUT_GUARD 32768U

struct raop_alac_decoder {
	alac_file *state;
	uint8_t *input;
	uint32_t channels;
	uint32_t frame_length;
};

static void *zero_array(size_t count, size_t element_size)
{
	if (count == 0 || element_size > SIZE_MAX / count)
		return NULL;
	return calloc(count, element_size);
}

static int offer_supported(const raop_sdp_offer *offer)
{
	if (offer == NULL || !offer->has_fmtp)
		return 0;
	return offer->fmtp[0] > 0 && offer->fmtp[0] <= RAOP_ALAC_FRAME_MAX &&
		offer->fmtp[1] == 0 && offer->fmtp[2] == 16 &&
		offer->fmtp[3] <= UINT8_MAX && offer->fmtp[4] <= UINT8_MAX &&
		offer->fmtp[5] <= 30 &&
		(offer->fmtp[6] == 1 || offer->fmtp[6] == 2) &&
		offer->fmtp[7] <= UINT8_MAX && offer->fmtp[8] <= UINT16_MAX &&
		offer->fmtp[9] <= UINT32_MAX && offer->fmtp[10] <= UINT32_MAX &&
		(offer->fmtp[10] == 44100 || offer->fmtp[10] == 48000);
}

raop_alac_decoder *raop_alac_create(const raop_sdp_offer *offer)
{
	raop_alac_decoder *decoder;
	alac_file *state;
	size_t samples;
	if (!offer_supported(offer))
		return NULL;
	decoder = calloc(1, sizeof(*decoder));
	if (decoder == NULL)
		return NULL;
	decoder->frame_length = offer->fmtp[0];
	decoder->channels = offer->fmtp[6];
	decoder->input = calloc(1, RAOP_ALAC_INPUT_MAX + ALAC_INPUT_GUARD);
	state = alac_create(16, (int)decoder->channels);
	if (decoder->input == NULL || state == NULL) {
		free(decoder->input);
		free(decoder);
		if (state != NULL)
			alac_free(state);
		return NULL;
	}
	decoder->state = state;
	state->setinfo_max_samples_per_frame = decoder->frame_length;
	state->setinfo_7a = (uint8_t)offer->fmtp[1];
	state->setinfo_sample_size = (uint8_t)offer->fmtp[2];
	state->setinfo_rice_historymult = (uint8_t)offer->fmtp[3];
	state->setinfo_rice_initialhistory = (uint8_t)offer->fmtp[4];
	state->setinfo_rice_kmodifier = (uint8_t)offer->fmtp[5];
	state->setinfo_7f = (uint8_t)offer->fmtp[6];
	state->setinfo_80 = (uint16_t)offer->fmtp[7];
	state->setinfo_82 = offer->fmtp[8];
	state->setinfo_86 = offer->fmtp[9];
	state->setinfo_8a_rate = offer->fmtp[10];
	samples = decoder->frame_length;
	state->predicterror_buffer_a = zero_array(samples, sizeof(int32_t));
	state->predicterror_buffer_b = zero_array(samples, sizeof(int32_t));
	state->outputsamples_buffer_a = zero_array(samples, sizeof(int32_t));
	state->outputsamples_buffer_b = zero_array(samples, sizeof(int32_t));
	state->uncompressed_bytes_buffer_a = zero_array(samples, sizeof(int32_t));
	state->uncompressed_bytes_buffer_b = zero_array(samples, sizeof(int32_t));
	if (state->predicterror_buffer_a == NULL || state->predicterror_buffer_b == NULL ||
		state->outputsamples_buffer_a == NULL || state->outputsamples_buffer_b == NULL ||
		state->uncompressed_bytes_buffer_a == NULL ||
		state->uncompressed_bytes_buffer_b == NULL) {
		raop_alac_destroy(decoder);
		return NULL;
	}
	state->bytespersample = (int)(decoder->channels * sizeof(int16_t));
	return decoder;
}

int raop_alac_decode(raop_alac_decoder *decoder, const uint8_t *packet,
	size_t packet_length, uint8_t *pcm, size_t pcm_capacity,
	size_t *pcm_length, uint32_t *sample_count)
{
	int output_bytes;
	size_t maximum_output;
	if (decoder == NULL || packet == NULL || packet_length == 0 ||
		packet_length > RAOP_ALAC_INPUT_MAX || pcm == NULL ||
		pcm_length == NULL || sample_count == NULL)
		return 0;
	*pcm_length = 0;
	*sample_count = 0;
	maximum_output = (size_t)decoder->frame_length * decoder->channels * 2U;
	if (pcm_capacity < maximum_output)
		return 0;
	memset(decoder->input, 0, RAOP_ALAC_INPUT_MAX + ALAC_INPUT_GUARD);
	memcpy(decoder->input, packet, packet_length);
	output_bytes = (int)pcm_capacity;
	alac_decode_frame(decoder->state, decoder->input, pcm, &output_bytes);
	if (output_bytes <= 0 || (size_t)output_bytes > maximum_output ||
		(output_bytes % (int)(decoder->channels * 2U)) != 0) {
		memset(pcm, 0, pcm_capacity);
		return 0;
	}
	*pcm_length = (size_t)output_bytes;
	*sample_count = (uint32_t)((size_t)output_bytes / (decoder->channels * 2U));
	return 1;
}

void raop_alac_destroy(raop_alac_decoder *decoder)
{
	if (decoder == NULL)
		return;
	if (decoder->state != NULL)
		alac_free(decoder->state);
	if (decoder->input != NULL) {
		memset(decoder->input, 0, RAOP_ALAC_INPUT_MAX + ALAC_INPUT_GUARD);
		free(decoder->input);
	}
	memset(decoder, 0, sizeof(*decoder));
	free(decoder);
}
