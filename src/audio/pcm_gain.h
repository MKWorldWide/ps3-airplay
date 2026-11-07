#ifndef PS3_AIRPLAY_PCM_GAIN_H
#define PS3_AIRPLAY_PCM_GAIN_H

#include <stddef.h>

typedef struct {
	float current;
	float target;
} raop_pcm_gain;

void raop_pcm_gain_init(raop_pcm_gain *gain);
/* Clamp to [0, 1]; NaN and negative inputs become mute. */
void raop_pcm_gain_set_target(raop_pcm_gain *gain, float target);
/* Apply a per-block linear ramp to interleaved stereo float samples. */
void raop_pcm_gain_apply(raop_pcm_gain *gain, float *stereo,
	size_t frames);

#endif
