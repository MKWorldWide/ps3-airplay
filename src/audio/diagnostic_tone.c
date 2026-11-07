#include "diagnostic_tone.h"

#include <math.h>
#include <string.h>

#define TONE_PEAK_LEVEL 0.05
#define TONE_RAMP_FRAMES 2400U
#define TONE_PI 3.14159265358979323846

void raop_diagnostic_tone_init(raop_diagnostic_tone *tone)
{
	if (tone != NULL)
		tone->frame_index = 0;
}

size_t raop_diagnostic_tone_render(raop_diagnostic_tone *tone,
	float *stereo_output, size_t output_capacity_frames)
{
	size_t frame;
	size_t rendered;
	if (stereo_output == NULL)
		return 0;
	if (tone == NULL) {
		memset(stereo_output, 0, output_capacity_frames * 2U * sizeof(float));
		return 0;
	}
	if (tone->frame_index >= RAOP_DIAGNOSTIC_TONE_FRAMES) {
		memset(stereo_output, 0, output_capacity_frames * 2U * sizeof(float));
		return 0;
	}
	rendered = output_capacity_frames;
	if (rendered > RAOP_DIAGNOSTIC_TONE_FRAMES - tone->frame_index)
		rendered = RAOP_DIAGNOSTIC_TONE_FRAMES - tone->frame_index;
	for (frame = 0; frame < rendered; ++frame) {
		uint32_t index = tone->frame_index + (uint32_t)frame;
		uint32_t remaining = RAOP_DIAGNOSTIC_TONE_FRAMES - index - 1U;
		double envelope = 1.0;
		double phase = 2.0 * TONE_PI * RAOP_DIAGNOSTIC_TONE_FREQUENCY *
			(double)index / RAOP_DIAGNOSTIC_TONE_RATE;
		float sample;
		if (index < TONE_RAMP_FRAMES)
			envelope = (double)index / TONE_RAMP_FRAMES;
		if (remaining < TONE_RAMP_FRAMES &&
			envelope > (double)remaining / TONE_RAMP_FRAMES)
			envelope = (double)remaining / TONE_RAMP_FRAMES;
		sample = (float)(TONE_PEAK_LEVEL * envelope * sin(phase));
		stereo_output[frame * 2U] = sample;
		stereo_output[frame * 2U + 1U] = sample;
	}
	if (rendered < output_capacity_frames)
		memset(stereo_output + rendered * 2U, 0,
			(output_capacity_frames - rendered) * 2U * sizeof(float));
	tone->frame_index += (uint32_t)rendered;
	return rendered;
}

int raop_diagnostic_tone_complete(const raop_diagnostic_tone *tone)
{
	return tone != NULL && tone->frame_index >= RAOP_DIAGNOSTIC_TONE_FRAMES;
}
