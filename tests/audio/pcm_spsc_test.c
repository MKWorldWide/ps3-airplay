#include "../../src/audio/pcm_spsc.h"

#include <assert.h>
#include <pthread.h>
#include <sched.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#define STRESS_FRAMES 1000000U

static raop_pcm_spsc queue;
static volatile int producer_done;

static void *producer(void *unused)
{
	uint32_t i;
	uint8_t frame[4];
	(void)unused;
	for (i = 0; i < STRESS_FRAMES; ++i) {
		uint16_t left = (uint16_t)i;
		uint16_t right = (uint16_t)~i;
		frame[0] = (uint8_t)left;
		frame[1] = (uint8_t)(left >> 8);
		frame[2] = (uint8_t)right;
		frame[3] = (uint8_t)(right >> 8);
		while (raop_pcm_spsc_push_le16(&queue, frame, sizeof(frame), 2, 0) != 1)
			(void)sched_yield();
	}
	__atomic_store_n(&producer_done, 1, __ATOMIC_RELEASE);
	return NULL;
}

static void test_mono_stereo_and_overflow(void)
{
	uint8_t stereo[] = {0x34, 0x12, 0xcc, 0xed};
	uint8_t mono[] = {0x00, 0x80, 0xff, 0x7f};
	uint8_t full[RAOP_PCM_SPSC_CAPACITY * 2U];
	int16_t output[6];
	raop_pcm_spsc_init(&queue);
	assert(raop_pcm_spsc_push_le16(&queue, stereo, sizeof(stereo), 2, 0) == 1);
	assert(raop_pcm_spsc_push_le16(&queue, mono, sizeof(mono), 1, 0) == 2);
	assert(raop_pcm_spsc_count(&queue) == 3);
	assert(raop_pcm_spsc_pop(&queue, output, 3, 0) == 3);
	assert(output[0] == 0x1234 && output[1] == (int16_t)0xedcc);
	assert(output[2] == INT16_MIN && output[3] == INT16_MIN);
	assert(output[4] == INT16_MAX && output[5] == INT16_MAX);
	assert(raop_pcm_spsc_push_le16(&queue, mono, sizeof(mono), 3, 0) == 0);
	assert(raop_pcm_spsc_push_le16(&queue, stereo, 3, 2, 0) == 0);
	memset(full, 0, sizeof(full));
	raop_pcm_spsc_discard(&queue);
	assert(raop_pcm_spsc_push_le16(&queue, full, sizeof(full), 1, 0) ==
		RAOP_PCM_SPSC_CAPACITY);
	assert(raop_pcm_spsc_push_le16(&queue, mono, sizeof(mono), 1, 0) == 0);
	assert(queue.frames_dropped == 2);
	raop_pcm_spsc_discard(&queue);
	assert(raop_pcm_spsc_count(&queue) == 0);
	assert(raop_pcm_spsc_push_le16(&queue, mono, sizeof(mono), 1, 4) == 2);
	assert(raop_pcm_spsc_pop(&queue, output, 2, 5) == 0);
	assert(raop_pcm_spsc_count(&queue) == 0);
}

static void test_concurrent_ordering(void)
{
	pthread_t thread;
	uint32_t received = 0;
	int16_t output[512 * 2];
	raop_pcm_spsc_init(&queue);
	__atomic_store_n(&producer_done, 0, __ATOMIC_RELEASE);
	assert(pthread_create(&thread, NULL, producer, NULL) == 0);
	while (received < STRESS_FRAMES) {
		size_t count = raop_pcm_spsc_pop(&queue, output, 512, 0);
		size_t i;
		for (i = 0; i < count; ++i) {
			uint16_t expected = (uint16_t)received;
			assert((uint16_t)output[i * 2U] == expected);
			assert((uint16_t)output[i * 2U + 1U] == (uint16_t)~expected);
			++received;
		}
		if (count == 0)
			(void)sched_yield();
	}
	assert(__atomic_load_n(&producer_done, __ATOMIC_ACQUIRE));
	assert(pthread_join(thread, NULL) == 0);
	assert(raop_pcm_spsc_count(&queue) == 0);
}

int main(void)
{
	test_mono_stereo_and_overflow();
	test_concurrent_ordering();
	puts("bounded PCM SPSC queue and concurrency tests: PASS");
	return 0;
}
