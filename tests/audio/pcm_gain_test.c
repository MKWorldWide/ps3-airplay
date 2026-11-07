#include "../../src/audio/pcm_gain.h"

#include <assert.h>
#include <math.h>
#include <stdio.h>

int main(void)
{
	raop_pcm_gain gain;
	float samples[8] = { 1.0f, -1.0f, 1.0f, -1.0f,
		1.0f, -1.0f, 1.0f, -1.0f };
	raop_pcm_gain_init(&gain);
	raop_pcm_gain_set_target(&gain, 0.25f);
	raop_pcm_gain_apply(&gain, samples, 4);
	assert(samples[0] < 1.0f && samples[0] > 0.25f);
	assert(samples[7] == -0.25f);
	assert(gain.current == 0.25f && gain.target == 0.25f);
	raop_pcm_gain_set_target(&gain, 4.0f);
	assert(gain.target == 1.0f);
	raop_pcm_gain_set_target(&gain, -1.0f);
	assert(gain.target == 0.0f);
	raop_pcm_gain_apply(&gain, samples, 4);
	assert(samples[6] == 0.0f && samples[7] == 0.0f);
	raop_pcm_gain_set_target(&gain, NAN);
	assert(gain.target == 0.0f);
	puts("PCM gain bounds, mute, and ramp: PASS");
	return 0;
}
