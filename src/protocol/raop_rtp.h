#ifndef PS3_AIRPLAY_RAOP_RTP_H
#define PS3_AIRPLAY_RAOP_RTP_H

#include <stddef.h>
#include <stdint.h>

#define RAOP_RTP_HEADER_MAX 264
#define RAOP_RTP_PAYLOAD_MAX 1400
#define RAOP_RTP_QUEUE_CAPACITY 256
#define RAOP_RTP_PREBUFFER_PACKETS 128

typedef struct {
	uint8_t payload_type;
	uint16_t sequence;
	uint32_t timestamp;
	uint32_t ssrc;
	size_t header_length;
	size_t payload_length;
} raop_rtp_packet;

typedef struct {
	uint64_t packets;
	uint64_t payload_bytes;
	uint64_t missing_packets;
	uint64_t out_of_order_packets;
	uint64_t malformed_packets;
	uint64_t queue_drops;
	uint64_t retransmitted_packets;
	uint64_t duplicate_packets;
	uint64_t late_packets;
	uint16_t last_sequence;
	uint16_t last_missing_sequence;
	uint16_t last_missing_count;
	int have_sequence;
} raop_rtp_stats;

typedef struct {
	raop_rtp_packet packet;
	uint8_t payload[RAOP_RTP_PAYLOAD_MAX];
} raop_rtp_frame;

typedef struct {
	raop_rtp_frame frames[RAOP_RTP_QUEUE_CAPACITY];
	size_t read_index;
	size_t write_index;
	size_t count;
	uint16_t last_popped_sequence;
	int have_popped_sequence;
	raop_rtp_stats stats;
} raop_rtp_queue;

/* Parse a single RTP v2 packet; reject malformed, padded, or oversized frames. */
int raop_rtp_parse(const uint8_t *data, size_t length,
	raop_rtp_packet *packet);

/* Validate, account and enqueue payload bytes into the fixed-size ring. */
int raop_rtp_queue_push(raop_rtp_queue *queue, const uint8_t *data,
	size_t length);
/* Insert a validated retransmitted RTP packet by sequence into queued audio. */
int raop_rtp_queue_recover(raop_rtp_queue *queue, const uint8_t *data,
	size_t length);
int raop_rtp_queue_pop(raop_rtp_queue *queue, raop_rtp_frame *frame);
/* Invalidate queued frames and sequence epoch while retaining cumulative stats. */
void raop_rtp_queue_flush(raop_rtp_queue *queue);

#endif
