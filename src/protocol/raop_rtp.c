#include "raop_rtp.h"

#include <string.h>

static uint16_t read_u16(const uint8_t *data)
{
	return (uint16_t)(((uint16_t)data[0] << 8) | data[1]);
}

static uint32_t read_u32(const uint8_t *data)
{
	return ((uint32_t)data[0] << 24) | ((uint32_t)data[1] << 16) |
		((uint32_t)data[2] << 8) | (uint32_t)data[3];
}

int raop_rtp_parse(const uint8_t *data, size_t length,
	raop_rtp_packet *packet)
{
	size_t header_length;
	size_t payload_length;
	uint8_t csrc_count;
	uint8_t padding;
	uint8_t extension;
	if (data == NULL || packet == NULL || length < 12 ||
		(data[0] >> 6) != 2)
		return 0;
	csrc_count = (uint8_t)(data[0] & 0x0f);
	padding = (uint8_t)(data[0] & 0x20);
	extension = (uint8_t)(data[0] & 0x10);
	header_length = 12U + (size_t)csrc_count * 4U;
	if (header_length > length || header_length > RAOP_RTP_HEADER_MAX)
		return 0;
	if (extension != 0) {
		size_t extension_length;
		if (length - header_length < 4)
			return 0;
		extension_length = 4U + (size_t)read_u16(data + header_length + 2) * 4U;
		if (extension_length > length - header_length ||
			header_length + extension_length > RAOP_RTP_HEADER_MAX)
			return 0;
		header_length += extension_length;
	}
	payload_length = length - header_length;
	if (padding != 0) {
		size_t padding_length = data[length - 1];
		if (padding_length == 0 || padding_length > payload_length)
			return 0;
		payload_length -= padding_length;
	}
	if (payload_length == 0 || payload_length > RAOP_RTP_PAYLOAD_MAX)
		return 0;
	packet->payload_type = (uint8_t)(data[1] & 0x7f);
	packet->sequence = read_u16(data + 2);
	packet->timestamp = read_u32(data + 4);
	packet->ssrc = read_u32(data + 8);
	packet->header_length = header_length;
	packet->payload_length = payload_length;
	return 1;
}

static void account_packet(raop_rtp_stats *stats,
	const raop_rtp_packet *packet)
{
	int16_t difference;
	stats->last_missing_count = 0;
	if (!stats->have_sequence) {
		stats->last_sequence = packet->sequence;
		stats->have_sequence = 1;
	} else {
		difference = (int16_t)(uint16_t)(packet->sequence -
			stats->last_sequence);
		if (difference > 1) {
			stats->missing_packets += (uint16_t)(difference - 1);
			stats->last_missing_sequence =
				(uint16_t)(stats->last_sequence + 1);
			stats->last_missing_count = (uint16_t)(difference - 1);
			stats->last_sequence = packet->sequence;
		} else if (difference == 1) {
			stats->last_sequence = packet->sequence;
		} else {
			++stats->out_of_order_packets;
		}
	}
	++stats->packets;
	stats->payload_bytes += packet->payload_length;
}

static int insert_packet(raop_rtp_queue *queue, const uint8_t *data,
	const raop_rtp_packet *parsed, int recovered)
{
	raop_rtp_frame *frame;
	size_t insert_at = queue->count, move;
	if (queue->have_popped_sequence &&
		(int16_t)(uint16_t)(parsed->sequence - queue->last_popped_sequence) <= 0) {
		++queue->stats.late_packets;
		return 0;
	}
	for (move = 0; move < queue->count; ++move) {
		raop_rtp_frame *queued = &queue->frames[
			(queue->read_index + move) % RAOP_RTP_QUEUE_CAPACITY];
		int16_t relative = (int16_t)(uint16_t)(parsed->sequence -
			queued->packet.sequence);
		if (relative == 0) {
			++queue->stats.duplicate_packets;
			return 0;
		}
		if (relative < 0) {
			insert_at = move;
			break;
		}
	}
	if (queue->count == RAOP_RTP_QUEUE_CAPACITY) {
		++queue->stats.queue_drops;
		return 0;
	}
	for (move = queue->count; move > insert_at; --move) {
		queue->frames[(queue->read_index + move) % RAOP_RTP_QUEUE_CAPACITY] =
			queue->frames[(queue->read_index + move - 1) %
				RAOP_RTP_QUEUE_CAPACITY];
	}
	frame = &queue->frames[(queue->read_index + insert_at) %
		RAOP_RTP_QUEUE_CAPACITY];
	memset(frame, 0, sizeof(*frame));
	frame->packet = *parsed;
	memcpy(frame->payload, data + parsed->header_length,
		parsed->payload_length);
	queue->write_index = (queue->read_index + queue->count + 1) %
		RAOP_RTP_QUEUE_CAPACITY;
	++queue->count;
	if (recovered) {
		++queue->stats.retransmitted_packets;
		queue->stats.payload_bytes += parsed->payload_length;
	}
	return 1;
}

int raop_rtp_queue_push(raop_rtp_queue *queue, const uint8_t *data,
	size_t length)
{
	raop_rtp_packet parsed;
	if (queue == NULL || data == NULL)
		return 0;
	if (!raop_rtp_parse(data, length, &parsed)) {
		++queue->stats.malformed_packets;
		return 0;
	}
	account_packet(&queue->stats, &parsed);
	return insert_packet(queue, data, &parsed, 0);
}

int raop_rtp_queue_pop(raop_rtp_queue *queue, raop_rtp_frame *frame)
{
	if (queue == NULL || frame == NULL || queue->count == 0)
		return 0;
	*frame = queue->frames[queue->read_index];
	queue->last_popped_sequence = frame->packet.sequence;
	queue->have_popped_sequence = 1;
	queue->read_index = (queue->read_index + 1) % RAOP_RTP_QUEUE_CAPACITY;
	--queue->count;
	return 1;
}

int raop_rtp_queue_recover(raop_rtp_queue *queue, const uint8_t *data,
	size_t length)
{
	raop_rtp_packet parsed;
	if (queue == NULL || data == NULL)
		return 0;
	if (!raop_rtp_parse(data, length, &parsed)) {
		++queue->stats.malformed_packets;
		return 0;
	}
	return insert_packet(queue, data, &parsed, 1);
}
