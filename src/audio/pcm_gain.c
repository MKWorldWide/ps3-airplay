#include "pcm_gain.h"

#include <string.h>

void raop_pcm_gain_init(raop_pcm_gain *gain)
{
	if (gain == NULL)
		return;
	gain->current = 1.0f;
	gain->target = 1.0f;
}

void raop_pcm_gain_set_target(raop_pcm_gain *gain, float target)
{
	if (gain == NULL)
		return;
	if (!(target >= 0.0f))
		target = 0.0f;
	else if (target > 1.0f)
		target = 1.0f;
	gain->target = target;
}

void raop_pcm_gain_apply(raop_pcm_gain *gain, float *stereo,
	size_t frames)
{
	size_t i;
	float step;
	if (gain == NULL || stereo == NULL || frames == 0)
		return;
	step = (gain->target - gain->current) / (float)frames;
	for (i = 0; i < frames; ++i) {
		float left, right;
		gain->current += step;
		left = stereo[i * 2U] * gain->current;
		right = stereo[i * 2U + 1U] * gain->current;
		stereo[i * 2U] = left > 1.0f ? 1.0f :
			(left < -1.0f ? -1.0f : left);
		stereo[i * 2U + 1U] = right > 1.0f ? 1.0f :
			(right < -1.0f ? -1.0f : right);
	}
	gain->current = gain->target;
}
