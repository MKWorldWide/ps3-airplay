#include "diagnostic_tone.h"

#include <math.h>
#include <stdio.h>
#include <stdlib.h>

#define BLOCK_FRAMES 1024U
#define TEST_PI 3.14159265358979323846

static void require(int condition, const char *message)
{
	if (!condition) {
		fprintf(stderr, "diagnostic tone test failed: %s\n", message);
		exit(1);
	}
}

int main(void)
{
	raop_diagnostic_tone tone;
	float block[BLOCK_FRAMES * 2U];
	uint64_t frames = 0;
	uint64_t rising_crossings = 0;
	float previous = 0.0f;
	raop_diagnostic_tone_init(&tone);
	while (!raop_diagnostic_tone_complete(&tone)) {
		size_t generated = raop_diagnostic_tone_render(&tone, block,
			BLOCK_FRAMES);
		size_t i;
		require(generated > 0 && generated <= BLOCK_FRAMES,
			"render stays bounded and advances");
		for (i = 0; i < generated; ++i) {
			float sample = block[i * 2U];
			require(sample == block[i * 2U + 1U],
				"tone is centered in both channels");
			require(fabsf(sample) <= 0.05001f,
				"peak level stays at or below five percent");
			if (frames >= 4800U && frames < 235200U &&
				previous <= 0.0f && sample > 0.0f)
				++rising_crossings;
			previous = sample;
			++frames;
		}
	}
	require(frames == RAOP_DIAGNOSTIC_TONE_FRAMES,
		"duration is exactly five seconds at 48 kHz");
	require(rising_crossings >= 2111U && rising_crossings <= 2113U,
		"frequency is 440 Hz within one cycle over the measured interval");
	require(block[(frames % BLOCK_FRAMES) * 2U] == 0.0f,
		"final partial block is padded with silence");
	for (size_t i = 0; i < BLOCK_FRAMES * 2U; ++i)
		block[i] = 1.0f;
	require(raop_diagnostic_tone_render(&tone, block, BLOCK_FRAMES) == 0,
		"completed tone generates no extra frames");
	for (size_t i = 0; i < BLOCK_FRAMES * 2U; ++i)
		require(block[i] == 0.0f, "completed block is silent");
	printf("bounded 440 Hz five-second diagnostic tone: PASS\n");
	return 0;
}
