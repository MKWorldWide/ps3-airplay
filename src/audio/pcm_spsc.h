#ifndef PS3_AIRPLAY_PCM_SPSC_H
#define PS3_AIRPLAY_PCM_SPSC_H

#include <stddef.h>
#include <stdint.h>

#define RAOP_PCM_SPSC_CAPACITY 16384U

/* Bounded SPSC bridge between the RAOP decoder and resident audio worker. */
typedef struct {
	int16_t samples[RAOP_PCM_SPSC_CAPACITY][2];
	uint32_t generation[RAOP_PCM_SPSC_CAPACITY];
	uint32_t read_index;
	uint32_t write_index;
	uint64_t frames_dropped;
} raop_pcm_spsc;

void raop_pcm_spsc_init(raop_pcm_spsc *queue);
/* Input is signed 16-bit little-endian mono/stereo interleaved PCM. */
size_t raop_pcm_spsc_push_le16(raop_pcm_spsc *queue, const uint8_t *pcm,
	size_t byte_length, uint8_t channels, uint32_t generation);
/* Pops stereo frames; returns the number copied. */
size_t raop_pcm_spsc_pop(raop_pcm_spsc *queue, int16_t *stereo,
	size_t frame_capacity, uint32_t generation);
uint32_t raop_pcm_spsc_count(const raop_pcm_spsc *queue);
/* Consumer-only discard, used when the RAOP session generation changes. */
void raop_pcm_spsc_discard(raop_pcm_spsc *queue);

#endif
