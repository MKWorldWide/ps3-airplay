#include "../../src/discovery/raop_mdns.h"
#include "../../src/discovery/raop_mdns_service.h"

#include <assert.h>
#include <stdio.h>
#include <string.h>

static uint16_t read_u16(const uint8_t *p)
{
	return (uint16_t)(((uint16_t)p[0] << 8) | p[1]);
}

static uint32_t read_u32(const uint8_t *p)
{
	return ((uint32_t)read_u16(p) << 16) | read_u16(p + 2);
}

static int read_name(const uint8_t *packet, size_t length, size_t *offset,
	char *out, size_t out_capacity)
{
	size_t pos = *offset;
	size_t used = 0;
	int first = 1;
	while (pos < length) {
		uint8_t label_length = packet[pos++];
		if (label_length == 0) {
			*offset = pos;
			if (used >= out_capacity)
				return 0;
			out[used] = '\0';
			return 1;
		}
		if ((label_length & 0xc0) != 0 || pos + label_length > length)
			return 0;
		if (!first) {
			if (used + 1 >= out_capacity)
				return 0;
			out[used++] = '.';
		}
		if (used + label_length >= out_capacity)
			return 0;
		memcpy(out + used, packet + pos, label_length);
		used += label_length;
		pos += label_length;
		first = 0;
	}
	return 0;
}

int main(void)
{
	static const char *const expected_txt[] = {
		"txtvers=1", "ch=2", "cn=1", "et=0,1", "sr=44100", "ss=16",
		"tp=UDP", "pw=false", "vn=65537", "vs=0.1.2", "am=PS3", "sf=0x4"
	};
	const char *const *txt = NULL;
	size_t txt_count = raop_mdns_service_txt_records(&txt);
	const uint8_t ipv4[4] = {192, 0, 2, 42};
	uint8_t packet[1024];
	size_t packet_length;
	size_t offset = 12;
	unsigned int record_index;
	static const uint16_t expected_types[] = {12, 33, 16, 1, 12};
	size_t txt_index;
	assert(txt_count == sizeof(expected_txt) / sizeof(expected_txt[0]));
	for (txt_index = 0; txt_index < txt_count; ++txt_index)
		assert(strcmp(txt[txt_index], expected_txt[txt_index]) == 0);
	packet_length = raop_mdns_build_response(packet, sizeof(packet),
		"AABBCCDDEEFF@PS3-AirPlay", "PS3-AirPlay", 5000, ipv4,
		txt, txt_count, 120);

	assert(packet_length > 12);
	assert(read_u16(packet + 2) == 0x8400);
	assert(read_u16(packet + 4) == 0);
	assert(read_u16(packet + 6) == 5);
	assert(read_u16(packet + 8) == 0 && read_u16(packet + 10) == 0);

	for (record_index = 0; record_index < 5; ++record_index) {
		char owner[256];
		assert(read_name(packet, packet_length, &offset, owner, sizeof(owner)));
		assert(offset + 10 <= packet_length);
		uint16_t type = read_u16(packet + offset);
		uint16_t klass = read_u16(packet + offset + 2);
		uint32_t ttl = read_u32(packet + offset + 4);
		uint16_t rdlength = read_u16(packet + offset + 8);
		offset += 10;
		assert(type == expected_types[record_index]);
		assert(ttl == 120);
		assert(offset + rdlength <= packet_length);

		if (record_index == 0) {
			char target[256];
			assert(strcmp(owner, "_raop._tcp.local") == 0);
			assert(read_name(packet, packet_length, &offset, target, sizeof(target)));
			assert(strcmp(target,
				"AABBCCDDEEFF@PS3-AirPlay._raop._tcp.local") == 0);
			assert((klass & 0x7fff) == 1);
		} else if (record_index == 1) {
			assert(strcmp(owner,
				"AABBCCDDEEFF@PS3-AirPlay._raop._tcp.local") == 0);
			assert(rdlength >= 7);
			assert(read_u16(packet + offset) == 0);
			assert(read_u16(packet + offset + 2) == 0);
			assert(read_u16(packet + offset + 4) == 5000);
			assert((klass & 0x8000) != 0);
			offset += rdlength;
		} else if (record_index == 2) {
			size_t txt_end = offset + rdlength;
			size_t txt_items = 0;
			assert(strcmp(owner,
				"AABBCCDDEEFF@PS3-AirPlay._raop._tcp.local") == 0);
			while (offset < txt_end) {
				uint8_t item_length = packet[offset++];
				assert(item_length > 0 && offset + item_length <= txt_end);
				assert(txt_items < txt_count);
				assert(item_length == strlen(txt[txt_items]));
				assert(memcmp(packet + offset, txt[txt_items], item_length) == 0);
				offset += item_length;
				++txt_items;
			}
			assert(txt_items == txt_count);
			assert((klass & 0x8000) != 0);
		} else if (record_index == 3) {
			assert(strcmp(owner, "PS3-AirPlay.local") == 0);
			assert(rdlength == 4);
			assert(memcmp(packet + offset, ipv4, sizeof(ipv4)) == 0);
			assert((klass & 0x8000) != 0);
			offset += rdlength;
		} else {
			char target[256];
			assert(strcmp(owner, "_services._dns-sd._udp.local") == 0);
			assert(read_name(packet, packet_length, &offset, target, sizeof(target)));
			assert(strcmp(target, "_raop._tcp.local") == 0);
			assert((klass & 0x7fff) == 1);
		}
	}
	assert(offset == packet_length);
	assert(raop_mdns_build_response(packet, 8, "bad", "host", 5000,
		ipv4, txt, txt_count, 120) == 0);
	assert(raop_mdns_build_response(packet, sizeof(packet), "bad..label",
		"host", 5000, ipv4, txt, txt_count, 120) == 0);
	assert(raop_mdns_build_response(packet, sizeof(packet), "instance",
		"host", 0, ipv4, txt, txt_count, 120) == 0);
	assert(raop_mdns_build_response(packet, sizeof(packet), "instance",
		"host", 5000, ipv4, txt, 0, 120) == 0);

	printf("RAOP mDNS packet validated: %lu bytes, five records\n",
		(unsigned long)packet_length);
	return 0;
}
