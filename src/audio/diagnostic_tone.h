#ifndef PS3_AIRPLAY_DIAGNOSTIC_TONE_H
#define PS3_AIRPLAY_DIAGNOSTIC_TONE_H

#include <stddef.h>
#include <stdint.h>

#define RAOP_DIAGNOSTIC_TONE_RATE 48000U
#define RAOP_DIAGNOSTIC_TONE_FREQUENCY 440U
#define RAOP_DIAGNOSTIC_TONE_SECONDS 5U
#define RAOP_DIAGNOSTIC_TONE_FRAMES \
	(RAOP_DIAGNOSTIC_TONE_RATE * RAOP_DIAGNOSTIC_TONE_SECONDS)

typedef struct {
	uint32_t frame_index;
} raop_diagnostic_tone;

void raop_diagnostic_tone_init(raop_diagnostic_tone *tone);
/* Writes stereo 48 kHz float frames, padding the block with silence at end. */
size_t raop_diagnostic_tone_render(raop_diagnostic_tone *tone,
	float *stereo_output, size_t output_capacity_frames);
int raop_diagnostic_tone_complete(const raop_diagnostic_tone *tone);

#endif
