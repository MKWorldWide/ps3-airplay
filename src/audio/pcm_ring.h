#ifndef PS3_AIRPLAY_PCM_RING_H
#define PS3_AIRPLAY_PCM_RING_H

#include <stddef.h>
#include <stdint.h>

#define RAOP_PCM_RING_CAPACITY 16384U

typedef struct {
	int16_t samples[RAOP_PCM_RING_CAPACITY][2];
	uint32_t read_index;
	uint32_t write_index;
	uint32_t count;
	uint32_t phase;
	uint32_t fir_index;
	int16_t fir_history[64][2];
	int16_t current[2];
	int16_t next[2];
	uint64_t frames_written;
	uint64_t frames_read;
	uint64_t frames_dropped;
	uint64_t output_underflows;
	uint8_t input_channels;
	uint8_t have_current;
	uint8_t have_next;
} raop_pcm_ring;

void raop_pcm_ring_init(raop_pcm_ring *ring, uint8_t input_channels);
/* Discard buffered frames and resampler history, preserving cumulative stats. */
void raop_pcm_ring_clear(raop_pcm_ring *ring);
/* Input is signed 16-bit little-endian mono/stereo. Returns frames accepted. */
size_t raop_pcm_ring_push_le16(raop_pcm_ring *ring, const uint8_t *pcm,
	size_t byte_length);
/* Stereo float output at 48 kHz. 44.1 kHz FIR adds 31 input frames of delay. */
size_t raop_pcm_ring_render_float(raop_pcm_ring *ring, uint32_t input_rate,
	uint32_t output_rate, float *output, size_t output_frames);
/* Fill output with intentional silence without counting stream underflows. */
void raop_pcm_ring_render_silence(float *output, size_t output_frames);

#endif
