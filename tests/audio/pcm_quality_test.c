#include "../../src/audio/pcm_ring.h"

#include <math.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#define INPUT_RATE 44100U
#define OUTPUT_RATE 48000U
#define BLOCK_INPUT_FRAMES 441U
#define BLOCK_OUTPUT_FRAMES 480U
#define BLOCK_COUNT 100U
#define PREBUFFER_FRAMES (BLOCK_INPUT_FRAMES * 2U)
#define TONE_PEAK 0.70
#define AUDIO_PI 3.14159265358979323846

static void write_le16(uint8_t *out, int16_t sample)
{
	uint16_t bits = (uint16_t)sample;
	out[0] = (uint8_t)bits;
	out[1] = (uint8_t)(bits >> 8);
}

static void push_tone(raop_pcm_ring *ring, uint8_t *input, size_t frames,
	size_t first_frame, unsigned int frequency)
{
	size_t i;
	for (i = 0; i < frames; ++i) {
		double phase = 2.0 * AUDIO_PI * (double)frequency *
			(double)(first_frame + i) / (double)INPUT_RATE;
		int16_t sample = (int16_t)lrint(TONE_PEAK * 32767.0 * sin(phase));
		write_le16(input + i * 4U, sample);
		write_le16(input + i * 4U + 2U, sample);
	}
	if (raop_pcm_ring_push_le16(ring, input, frames * 4U) != frames) {
		fputs("PCM quality test input was not fully accepted\n", stderr);
		exit(1);
	}
}

static void measure_tone(unsigned int frequency, int comma)
{
	raop_pcm_ring ring;
	uint8_t input[PREBUFFER_FRAMES * 4U];
	float block[BLOCK_OUTPUT_FRAMES * 2U];
	float output[BLOCK_COUNT * BLOCK_OUTPUT_FRAMES];
	size_t input_cursor = 0;
	size_t output_cursor = 0;
	size_t n;
	double sum_error = 0.0, sum_signal = 0.0, peak_error = 0.0;
	double in_phase = 0.0, quadrature = 0.0;
	double rms_error, rms_signal, amplitude, snr_db, amplitude_ratio;

	raop_pcm_ring_init(&ring, 2);
	push_tone(&ring, input, PREBUFFER_FRAMES, input_cursor, frequency);
	input_cursor += PREBUFFER_FRAMES;
	for (n = 0; n < BLOCK_COUNT; ++n) {
		size_t i;
		if (raop_pcm_ring_render_float(&ring, INPUT_RATE, OUTPUT_RATE,
			block, BLOCK_OUTPUT_FRAMES) != BLOCK_OUTPUT_FRAMES) {
			fputs("PCM quality test rendered an unexpected frame count\n", stderr);
			exit(1);
		}
		for (i = 0; i < BLOCK_OUTPUT_FRAMES; ++i)
			output[output_cursor + i] = block[i * 2U];
		output_cursor += BLOCK_OUTPUT_FRAMES;
		push_tone(&ring, input, BLOCK_INPUT_FRAMES, input_cursor, frequency);
		input_cursor += BLOCK_INPUT_FRAMES;
	}
	if (output_cursor != OUTPUT_RATE || ring.output_underflows != 0 ||
		ring.frames_dropped != 0) {
		fprintf(stderr, "PCM quality test stream integrity failed at %u Hz\n",
			frequency);
		exit(1);
	}

	/* Compare the steady-state output with the ideal 48 kHz tone. */
	for (n = 100; n < OUTPUT_RATE - 100U; ++n) {
		double phase = 2.0 * AUDIO_PI * (double)frequency *
			(double)n / (double)OUTPUT_RATE;
		double expected = (TONE_PEAK * 32767.0 / 32768.0) * sin(phase);
		double error = (double)output[n] - expected;
		sum_error += error * error;
		sum_signal += expected * expected;
		if (fabs(error) > peak_error)
			peak_error = fabs(error);
		in_phase += (double)output[n] * sin(phase);
		quadrature += (double)output[n] * cos(phase);
	}
	rms_error = sqrt(sum_error / (double)(OUTPUT_RATE - 200U));
	rms_signal = sqrt(sum_signal / (double)(OUTPUT_RATE - 200U));
	amplitude = 2.0 * sqrt(in_phase * in_phase + quadrature * quadrature) /
		(double)(OUTPUT_RATE - 200U);
	amplitude_ratio = amplitude / (TONE_PEAK * 32767.0 / 32768.0);
	snr_db = 20.0 * log10(rms_signal / rms_error);

	printf("%s{\"frequency_hz\":%u,\"output_frames\":%zu,"
		"\"rms_error\":%.9g,\"peak_error\":%.9g,"
		"\"snr_db\":%.6f,\"amplitude_ratio\":%.9g,"
		"\"input_frames_read\":%llu,\"underflows\":%llu}\n",
		comma ? "," : "", frequency, output_cursor, rms_error, peak_error,
		snr_db, amplitude_ratio, (unsigned long long)ring.frames_read,
		(unsigned long long)ring.output_underflows);
}

int main(void)
{
	static const unsigned int frequencies[] = {100U, 1000U, 5000U,
		10000U, 15000U, 20000U};
	size_t i;
	puts("{\"test\":\"production_pcm_ring_44k1_to_48k\",\"cases\":[");
	for (i = 0; i < sizeof(frequencies) / sizeof(frequencies[0]); ++i)
		measure_tone(frequencies[i], i != 0);
	puts("]}");
	return 0;
}
