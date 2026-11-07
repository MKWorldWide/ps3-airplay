#include "../../src/protocol/raop_rtp.h"

#include <assert.h>
#include <stdio.h>
#include <string.h>

static size_t make_packet(uint8_t *packet, uint16_t sequence,
	const uint8_t *payload, size_t payload_length)
{
	memset(packet, 0, 12 + payload_length);
	packet[0] = 0x80;
	packet[1] = 96;
	packet[2] = (uint8_t)(sequence >> 8);
	packet[3] = (uint8_t)sequence;
	packet[4] = 0x01;
	packet[5] = 0x02;
	packet[6] = 0x03;
	packet[7] = 0x04;
	packet[8] = 0xaa;
	packet[9] = 0xbb;
	packet[10] = 0xcc;
	packet[11] = 0xdd;
	memcpy(packet + 12, payload, payload_length);
	return 12 + payload_length;
}

int main(void)
{
	raop_rtp_queue queue = {0};
	raop_rtp_queue wrap_queue = {0};
	raop_rtp_frame frame;
	raop_rtp_packet parsed;
	uint8_t packet[12 + RAOP_RTP_PAYLOAD_MAX];
	const uint8_t payload[] = {0x10, 0x20, 0x30, 0x40};
	size_t length;
	unsigned int i;

	length = make_packet(packet, 100, payload, sizeof(payload));
	assert(raop_rtp_parse(packet, length, &parsed));
	assert(parsed.sequence == 100 && parsed.payload_type == 96);
	assert(parsed.timestamp == 0x01020304 && parsed.ssrc == 0xaabbccdd);
	assert(parsed.payload_length == sizeof(payload));
	assert(raop_rtp_queue_push(&queue, packet, length));
	length = make_packet(packet, 102, payload, sizeof(payload));
	assert(raop_rtp_queue_push(&queue, packet, length));
	length = make_packet(packet, 101, payload, sizeof(payload));
	assert(raop_rtp_queue_push(&queue, packet, length));
	assert(queue.stats.packets == 3);
	assert(queue.stats.payload_bytes == sizeof(payload) * 3);
	assert(queue.stats.missing_packets == 1);
	assert(queue.stats.out_of_order_packets == 1);
	assert(raop_rtp_queue_pop(&queue, &frame));
	assert(frame.packet.sequence == 100);
	assert(memcmp(frame.payload, payload, sizeof(payload)) == 0);
	assert(raop_rtp_queue_pop(&queue, &frame));
	assert(frame.packet.sequence == 101);
	assert(raop_rtp_queue_pop(&queue, &frame));
	assert(frame.packet.sequence == 102);
	assert(!raop_rtp_queue_pop(&queue, &frame));
	length = make_packet(packet, 65534, payload, sizeof(payload));
	assert(raop_rtp_queue_push(&wrap_queue, packet, length));
	length = make_packet(packet, 0, payload, sizeof(payload));
	assert(raop_rtp_queue_push(&wrap_queue, packet, length));
	length = make_packet(packet, 65535, payload, sizeof(payload));
	assert(raop_rtp_queue_recover(&wrap_queue, packet, length));
	assert(!raop_rtp_queue_recover(&wrap_queue, packet, length));
	assert(wrap_queue.stats.duplicate_packets == 1);
	assert(raop_rtp_queue_pop(&wrap_queue, &frame));
	assert(frame.packet.sequence == 65534);
	length = make_packet(packet, 65534, payload, sizeof(payload));
	assert(!raop_rtp_queue_recover(&wrap_queue, packet, length));
	assert(wrap_queue.stats.late_packets == 1);
	assert(raop_rtp_queue_pop(&wrap_queue, &frame));
	assert(frame.packet.sequence == 65535);
	assert(raop_rtp_queue_pop(&wrap_queue, &frame));
	assert(frame.packet.sequence == 0);

	for (i = 0; i < RAOP_RTP_QUEUE_CAPACITY + 1; ++i) {
		length = make_packet(packet, (uint16_t)(200 + i), payload,
			sizeof(payload));
		(void)raop_rtp_queue_push(&queue, packet, length);
	}
	assert(queue.count == RAOP_RTP_QUEUE_CAPACITY);
	assert(queue.stats.queue_drops == 1);
	length = make_packet(packet, 10000, payload, sizeof(payload));
	assert(!raop_rtp_queue_recover(&queue, packet, length));
	assert(queue.stats.queue_drops == 2);
	packet[0] = 0x40;
	assert(!raop_rtp_queue_push(&queue, packet, 16));
	assert(queue.stats.malformed_packets == 1);
	while (raop_rtp_queue_pop(&queue, &frame)) {}
	assert(!raop_rtp_queue_pop(&queue, &frame));
	puts("RAOP bounded RTP queue: PASS");
	return 0;
}
