#include "../../src/audio/pcm_ring.h"

#include <assert.h>
#include <math.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static void write_le16(uint8_t *out, int16_t value)
{
	uint16_t bits = (uint16_t)value;
	out[0] = (uint8_t)bits;
	out[1] = (uint8_t)(bits >> 8);
}

int main(void)
{
	raop_pcm_ring ring;
	uint8_t samples[12];
	float output[8];
	raop_pcm_ring_init(&ring, 2);
	write_le16(samples, -32768);
	write_le16(samples + 2, 32767);
	write_le16(samples + 4, -16384);
	write_le16(samples + 6, 16384);
	write_le16(samples + 8, 0);
	write_le16(samples + 10, 0);
	assert(raop_pcm_ring_push_le16(&ring, samples, sizeof(samples)) == 3);
	assert(raop_pcm_ring_render_float(&ring, 48000, 48000, output, 2) == 2);
	assert(output[0] == -1.0f);
	assert(output[1] > 0.99f && output[1] <= 1.0f);
	assert(output[2] < -0.49f && output[2] > -0.51f);
	assert(output[3] > 0.49f && output[3] < 0.51f);
	assert(ring.frames_written == 3);
	assert(ring.frames_read == 2);
	assert(raop_pcm_ring_render_float(&ring, 48000, 48000, output, 1) == 1);
	assert(output[0] == 0.0f && output[1] == 0.0f);
	assert(ring.frames_read == 3);
	assert(raop_pcm_ring_render_float(&ring, 48000, 48000, output, 1) == 1);
	assert(output[0] == 0.0f && output[1] == 0.0f);
	assert(ring.output_underflows == 1);
	assert(raop_pcm_ring_render_float(&ring, 44100, 44100, output, 1) == 0);

	{
		uint8_t *large = (uint8_t *)calloc(RAOP_PCM_RING_CAPACITY + 2U, 4);
		assert(large != NULL);
		raop_pcm_ring_init(&ring, 2);
		assert(raop_pcm_ring_push_le16(&ring, large,
			(RAOP_PCM_RING_CAPACITY + 2U) * 4U) == RAOP_PCM_RING_CAPACITY);
		assert(ring.frames_dropped == 2);
		free(large);
	}

	{
		int16_t mono[441];
		uint8_t mono_bytes[441 * 2];
		size_t i;
		size_t nonzero = 0;
		float resampled[480 * 2];
		for (i = 0; i < 441; ++i)
			write_le16(mono_bytes + i * 2, mono[i] = 8192);
		raop_pcm_ring_init(&ring, 1);
		assert(raop_pcm_ring_push_le16(&ring, mono_bytes, sizeof(mono_bytes)) == 441);
		assert(raop_pcm_ring_render_float(&ring, 44100, 48000,
			resampled, 480) == 480);
		for (i = 0; i < 480; ++i) {
			if (resampled[i * 2] > 0.24f)
				++nonzero;
			assert(resampled[i * 2] == resampled[i * 2 + 1]);
		}
		assert(nonzero > 470);
		assert(ring.output_underflows < 10);
	}

	{
		uint8_t buffered[12];
		uint64_t written, read, underflows, dropped;
		write_le16(buffered, 12000);
		write_le16(buffered + 2, -12000);
		write_le16(buffered + 4, 8000);
		write_le16(buffered + 6, -8000);
		write_le16(buffered + 8, 4000);
		write_le16(buffered + 10, -4000);
		raop_pcm_ring_init(&ring, 2);
		assert(raop_pcm_ring_push_le16(&ring, buffered, sizeof(buffered)) == 3);
		assert(raop_pcm_ring_render_float(&ring, 44100, 48000, output, 1) == 1);
		assert(ring.have_current && ring.have_next);
		written = ring.frames_written;
		read = ring.frames_read;
		underflows = ring.output_underflows;
		dropped = ring.frames_dropped;
		raop_pcm_ring_clear(&ring);
		assert(ring.count == 0 && ring.read_index == 0 && ring.write_index == 0);
		assert(ring.phase == 0 && !ring.have_current && !ring.have_next);
		assert(ring.frames_written == written && ring.frames_read == read);
		assert(ring.output_underflows == underflows && ring.frames_dropped == dropped);
		raop_pcm_ring_render_silence(output, 1);
		assert(output[0] == 0.0f && output[1] == 0.0f);
		assert(ring.output_underflows == underflows);
		assert(raop_pcm_ring_render_float(&ring, 48000, 48000, output, 1) == 1);
		assert(output[0] == 0.0f && output[1] == 0.0f);
		assert(ring.output_underflows == underflows + 1);
	}

	puts("bounded PCM ring, stereo conversion and 44.1-to-48 kHz resampling: PASS");
	return 0;
}
