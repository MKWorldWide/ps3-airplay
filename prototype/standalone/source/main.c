#include <audio/audio.h>
#include <lv2/systime.h>
#include <ppu-types.h>
#include <sys/event_queue.h>
#include <sysutil/sysutil.h>

#include "pcm_ring.h"

#include <math.h>
#include <stdio.h>

#define SAMPLE_RATE 48000.0f
#define TONE_SECONDS 5
#define TONE_FREQUENCY_HZ 440.0f
#define TONE_AMPLITUDE 0.04f
#define FADE_SECONDS 0.25f

static volatile int g_exit_requested;
static u64 g_samples_written;
static raop_pcm_ring g_pcm_ring;

static void on_system_event(u64 status, u64 param, void *userdata)
{
	(void)param;
	(void)userdata;
	if (status == SYSUTIL_EXIT_GAME)
		g_exit_requested = 1;
}

static void queue_tone_block(void)
{
	const u64 total_samples = (u64)(SAMPLE_RATE * TONE_SECONDS);
	const u64 fade_samples = (u64)(SAMPLE_RATE * FADE_SECONDS);
	const float phase_step = 2.0f * 3.14159265358979323846f *
		TONE_FREQUENCY_HZ / SAMPLE_RATE;
	uint8_t pcm[AUDIO_BLOCK_SAMPLES * 4];

	for (u32 i = 0; i < AUDIO_BLOCK_SAMPLES; ++i) {
		float envelope = TONE_AMPLITUDE;
		if (g_samples_written >= total_samples) {
			envelope = 0.0f;
		} else if (g_samples_written > total_samples - fade_samples) {
			envelope *= (float)(total_samples - g_samples_written) /
				(float)fade_samples;
		}

		const float sample = envelope * sinf(phase_step *
			(float)(g_samples_written % (u64)SAMPLE_RATE));
		int16_t value = (int16_t)(sample * 32767.0f);
		uint16_t bits = (uint16_t)value;
		pcm[i * 4] = (uint8_t)bits;
		pcm[i * 4 + 1] = (uint8_t)(bits >> 8);
		pcm[i * 4 + 2] = (uint8_t)bits;
		pcm[i * 4 + 3] = (uint8_t)(bits >> 8);
		++g_samples_written;
	}
	(void)raop_pcm_ring_push_le16(&g_pcm_ring, pcm, sizeof(pcm));
}

static void render_block(float *buffer)
{
	(void)raop_pcm_ring_render_float(&g_pcm_ring, (uint32_t)SAMPLE_RATE,
		48000, buffer, AUDIO_BLOCK_SAMPLES);
}

static void fill_all_blocks(const audioPortConfig *config)
{
	float *audio_data = (float *)(u64)config->audioDataStart;
	for (u32 block = 0; block < config->numBlocks; ++block)
		queue_tone_block();
	for (u32 block = 0; block < config->numBlocks; ++block)
		render_block(audio_data + block * config->channelCount *
			AUDIO_BLOCK_SAMPLES);
}

int main(void)
{
	int result = 1;
	int audio_initialized = 0;
	int event_queue_created = 0;
	int notify_queue_set = 0;
	int port_open = 0;
	int port_started = 0;
	int callback_registered = 0;
	u32 port_number = 0;
	sys_event_queue_t event_queue;
	sys_ipc_key_t event_key;
	audioPortParam params = {0};
	audioPortConfig config = {0};
	raop_pcm_ring_init(&g_pcm_ring, 2);

	if (sysUtilRegisterCallback(SYSUTIL_EVENT_SLOT0, on_system_event, NULL) != 0) {
		printf("sysUtilRegisterCallback failed\n");
		goto cleanup;
	}
	callback_registered = 1;

	if (audioInit() != 0) {
		printf("audioInit failed\n");
		goto cleanup;
	}
	audio_initialized = 1;

	params.numChannels = AUDIO_PORT_2CH;
	params.numBlocks = AUDIO_BLOCK_8;
	params.attrib = AUDIO_PORT_INITLEVEL;
	params.level = 0.20f;
	if (audioPortOpen(&params, &port_number) != 0) {
		printf("audioPortOpen failed\n");
		goto cleanup;
	}
	port_open = 1;

	if (audioGetPortConfig(port_number, &config) != 0) {
		printf("audioGetPortConfig failed\n");
		goto cleanup;
	}

	if (audioCreateNotifyEventQueue(&event_queue, &event_key) != 0) {
		printf("audioCreateNotifyEventQueue failed\n");
		goto cleanup;
	}
	event_queue_created = 1;
	if (audioSetNotifyEventQueue(event_key) != 0) {
		printf("audioSetNotifyEventQueue failed\n");
		goto cleanup;
	}
	notify_queue_set = 1;
	if (sysEventQueueDrain(event_queue) != 0) {
		printf("sysEventQueueDrain failed\n");
		goto cleanup;
	}

	fill_all_blocks(&config);
	queue_tone_block();
	if (audioPortStart(port_number) != 0) {
		printf("audioPortStart failed\n");
		goto cleanup;
	}
	port_started = 1;
	printf("Playing a low-volume 440 Hz stereo tone for five seconds.\n");

	const u64 deadline = (u64)sysGetSystemTime() + 5000000ULL;
	while (!g_exit_requested && (u64)sysGetSystemTime() < deadline) {
		sys_event_t event;
		s32 wait_result = sysEventQueueReceive(event_queue, &event, 20000);
		sysUtilCheckCallback();
		if (g_exit_requested)
			break;
		if (wait_result != 0) {
			printf("Audio event wait failed: %d\n", wait_result);
			goto cleanup;
		}

		volatile u64 *read_index = (volatile u64 *)(u64)config.readIndex;
		u32 next_block = (u32)((*read_index + 1) % config.numBlocks);
		float *audio_data = (float *)(u64)config.audioDataStart;
		render_block(audio_data + next_block * config.channelCount *
			AUDIO_BLOCK_SAMPLES);
		queue_tone_block();
	}
	result = 0;

cleanup:
	if (port_started)
		audioPortStop(port_number);
	if (notify_queue_set)
		audioRemoveNotifyEventQueue(event_key);
	if (port_open)
		audioPortClose(port_number);
	if (event_queue_created)
		sysEventQueueDestroy(event_queue, 0);
	if (audio_initialized)
		audioQuit();
	if (callback_registered)
		sysUtilUnregisterCallback(SYSUTIL_EVENT_SLOT0);
	return result;
}
