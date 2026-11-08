#include "pcm_ring.h"

#include <string.h>
#include "resampler_coefficients.h"

static int16_t read_le16(const uint8_t *bytes)
{
	uint16_t value = (uint16_t)bytes[0] | ((uint16_t)bytes[1] << 8);
	int32_t signed_value = (value & 0x8000U) ? (int32_t)value - 65536 : value;
	return (int16_t)signed_value;
}

static int pop_frame(raop_pcm_ring *ring, int16_t sample[2])
{
	if (ring->count == 0)
		return 0;
	sample[0] = ring->samples[ring->read_index][0];
	sample[1] = ring->samples[ring->read_index][1];
	ring->read_index = (ring->read_index + 1U) % RAOP_PCM_RING_CAPACITY;
	--ring->count;
	++ring->frames_read;
	return 1;
}

void raop_pcm_ring_init(raop_pcm_ring *ring, uint8_t input_channels)
{
	if (ring == NULL)
		return;
	memset(ring, 0, sizeof(*ring));
	if (input_channels == 1 || input_channels == 2)
		ring->input_channels = input_channels;
}

void raop_pcm_ring_clear(raop_pcm_ring *ring)
{
	if (ring == NULL)
		return;
	ring->read_index = 0;
	ring->write_index = 0;
	ring->count = 0;
	ring->phase = 0;
	ring->fir_index = 0;
	memset(ring->fir_history, 0, sizeof(ring->fir_history));
	ring->current[0] = ring->current[1] = 0;
	ring->next[0] = ring->next[1] = 0;
	ring->have_current = 0;
	ring->have_next = 0;
}

size_t raop_pcm_ring_push_le16(raop_pcm_ring *ring, const uint8_t *pcm,
	size_t byte_length)
{
	size_t frame_bytes, frames, accepted = 0, i;
	if (ring == NULL || pcm == NULL || ring->input_channels == 0)
		return 0;
	frame_bytes = (size_t)ring->input_channels * 2U;
	if (byte_length == 0 || (byte_length % frame_bytes) != 0)
		return 0;
	frames = byte_length / frame_bytes;
	for (i = 0; i < frames; ++i) {
		int16_t left, right;
		uint32_t next;
		if (ring->count == RAOP_PCM_RING_CAPACITY)
			break;
		left = read_le16(pcm + i * frame_bytes);
		right = ring->input_channels == 2 ?
			read_le16(pcm + i * frame_bytes + 2U) : left;
		ring->samples[ring->write_index][0] = left;
		ring->samples[ring->write_index][1] = right;
		next = (ring->write_index + 1U) % RAOP_PCM_RING_CAPACITY;
		ring->write_index = next;
		++ring->count;
		++ring->frames_written;
		++accepted;
	}
	ring->frames_dropped += frames - accepted;
	return accepted;
}

size_t raop_pcm_ring_render_float(raop_pcm_ring *ring, uint32_t input_rate,
	uint32_t output_rate, float *output, size_t output_frames)
{
	size_t rendered;
	if (ring == NULL || output == NULL ||
		(input_rate != 44100 && input_rate != 48000) || output_rate != 48000)
		return 0;
	if (input_rate == output_rate) {
		ring->have_current = 0;
		ring->have_next = 0;
		ring->phase = 0;
		for (rendered = 0; rendered < output_frames; ++rendered) {
			int16_t sample[2];
			if (!pop_frame(ring, sample)) {
				output[rendered * 2U] = 0.0f;
				output[rendered * 2U + 1U] = 0.0f;
				++ring->output_underflows;
			} else {
				output[rendered * 2U] = (float)sample[0] / 32768.0f;
				output[rendered * 2U + 1U] = (float)sample[1] / 32768.0f;
			}
		}
		return rendered;
	}
	for (rendered = 0; rendered < output_frames; ++rendered) {
		unsigned int channel;
		if (!ring->have_current) {
			if (!pop_frame(ring, ring->current)) {
				output[rendered * 2U] = 0.0f;
				output[rendered * 2U + 1U] = 0.0f;
				++ring->output_underflows;
				continue;
			}
			ring->have_current = 1;
            /* Edge extension gives deterministic startup without stale history. */
            ring->fir_index = 0;
            for (unsigned int k = 0; k < 64; ++k) {
                ring->fir_history[k][0] = ring->current[0];
                ring->fir_history[k][1] = ring->current[1];
            }
			ring->phase = 0;
		}
		if (!ring->have_next) {
			if (!pop_frame(ring, ring->next)) {
				ring->have_current = 0;
				output[rendered * 2U] = 0.0f;
				output[rendered * 2U + 1U] = 0.0f;
				++ring->output_underflows;
				continue;
			}
			ring->have_next = 1;
		}
        /* phase is an exact multiple of 300: the 160 rational phases
         * never accumulate floating-point timing error. */
        const float *coeff = raop_src_coefficients[ring->phase / 300U];
        for (channel = 0; channel < 2; ++channel) {
            float sum = 0.0f;
            for (unsigned int k = 0; k < 64; ++k)
                sum += coeff[k] * ring->fir_history[(ring->fir_index - k) & 63U][channel];
            output[rendered * 2U + channel] = sum / 32768.0f;
        }
		ring->phase += input_rate;
		while (ring->phase >= output_rate) {
			ring->current[0] = ring->next[0];
			ring->current[1] = ring->next[1];
            ring->fir_index = (ring->fir_index + 1U) & 63U;
            ring->fir_history[ring->fir_index][0] = ring->current[0];
            ring->fir_history[ring->fir_index][1] = ring->current[1];
			if (!pop_frame(ring, ring->next)) {
				ring->have_current = 0;
				ring->have_next = 0;
				ring->phase = 0;
				break;
			}
			ring->phase -= output_rate;
		}
	}
	return rendered;
}

void raop_pcm_ring_render_silence(float *output, size_t output_frames)
{
	if (output == NULL)
		return;
	memset(output, 0, output_frames * 2U * sizeof(*output));
}
