#include "pcm_spsc.h"

#include <string.h>

static int16_t read_le16(const uint8_t *bytes)
{
	uint16_t value = (uint16_t)bytes[0] | ((uint16_t)bytes[1] << 8);
	int32_t signed_value = (value & 0x8000U) ? (int32_t)value - 65536 : value;
	return (int16_t)signed_value;
}

void raop_pcm_spsc_init(raop_pcm_spsc *queue)
{
	if (queue != NULL)
		memset(queue, 0, sizeof(*queue));
}

size_t raop_pcm_spsc_push_le16(raop_pcm_spsc *queue, const uint8_t *pcm,
	size_t byte_length, uint8_t channels, uint32_t generation)
{
	uint32_t write_index;
	uint32_t read_index;
	size_t frame_bytes;
	size_t frames;
	size_t accepted = 0;
	if (queue == NULL || pcm == NULL || (channels != 1 && channels != 2))
		return 0;
	frame_bytes = (size_t)channels * 2U;
	if (byte_length == 0 || byte_length % frame_bytes != 0)
		return 0;
	frames = byte_length / frame_bytes;
	write_index = __atomic_load_n(&queue->write_index, __ATOMIC_RELAXED);
	read_index = __atomic_load_n(&queue->read_index, __ATOMIC_ACQUIRE);
	while (accepted < frames && write_index - read_index < RAOP_PCM_SPSC_CAPACITY) {
		size_t offset = accepted * frame_bytes;
		uint32_t slot = write_index & (RAOP_PCM_SPSC_CAPACITY - 1U);
		int16_t left = read_le16(pcm + offset);
		int16_t right = channels == 2 ? read_le16(pcm + offset + 2U) : left;
		queue->samples[slot][0] = left;
		queue->samples[slot][1] = right;
		queue->generation[slot] = generation;
		++write_index;
		++accepted;
	}
	if (accepted != 0)
		__atomic_store_n(&queue->write_index, write_index, __ATOMIC_RELEASE);
	queue->frames_dropped += frames - accepted;
	return accepted;
}

size_t raop_pcm_spsc_pop(raop_pcm_spsc *queue, int16_t *stereo,
	size_t frame_capacity, uint32_t generation)
{
	uint32_t read_index;
	uint32_t write_index;
	size_t count = 0;
	if (queue == NULL || stereo == NULL || frame_capacity == 0)
		return 0;
	read_index = __atomic_load_n(&queue->read_index, __ATOMIC_RELAXED);
	write_index = __atomic_load_n(&queue->write_index, __ATOMIC_ACQUIRE);
	while (count < frame_capacity && read_index != write_index) {
		uint32_t slot = read_index & (RAOP_PCM_SPSC_CAPACITY - 1U);
		if (queue->generation[slot] != generation) {
			++read_index;
			continue;
		}
		stereo[count * 2U] = queue->samples[slot][0];
		stereo[count * 2U + 1U] = queue->samples[slot][1];
		++read_index;
		++count;
	}
	if (read_index != __atomic_load_n(&queue->read_index, __ATOMIC_RELAXED))
		__atomic_store_n(&queue->read_index, read_index, __ATOMIC_RELEASE);
	return count;
}

uint32_t raop_pcm_spsc_count(const raop_pcm_spsc *queue)
{
	uint32_t read_index;
	uint32_t write_index;
	if (queue == NULL)
		return 0;
	read_index = __atomic_load_n(&queue->read_index, __ATOMIC_ACQUIRE);
	write_index = __atomic_load_n(&queue->write_index, __ATOMIC_ACQUIRE);
	return write_index - read_index;
}

void raop_pcm_spsc_discard(raop_pcm_spsc *queue)
{
	uint32_t write_index;
	if (queue == NULL)
		return;
	write_index = __atomic_load_n(&queue->write_index, __ATOMIC_ACQUIRE);
	__atomic_store_n(&queue->read_index, write_index, __ATOMIC_RELEASE);
}
