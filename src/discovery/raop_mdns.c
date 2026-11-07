#include "raop_mdns.h"

#include <stdio.h>
#include <string.h>

/*
 * DNS-SD record layout informed by azarucha/sys-airplay's MIT-licensed mDNS
 * responder. See THIRD_PARTY_NOTICES.md for the upstream license notice.
 */

typedef struct {
	uint8_t *bytes;
	size_t capacity;
	size_t length;
	int ok;
} writer_t;

static void put_u8(writer_t *w, uint8_t value)
{
	if (w->length >= w->capacity) {
		w->ok = 0;
		return;
	}
	w->bytes[w->length++] = value;
}

static void put_bytes(writer_t *w, const void *data, size_t length)
{
	if (length > w->capacity - w->length) {
		w->ok = 0;
		return;
	}
	memcpy(w->bytes + w->length, data, length);
	w->length += length;
}

static void put_u16(writer_t *w, uint16_t value)
{
	put_u8(w, (uint8_t)(value >> 8));
	put_u8(w, (uint8_t)value);
}

static void put_u32(writer_t *w, uint32_t value)
{
	put_u16(w, (uint16_t)(value >> 16));
	put_u16(w, (uint16_t)value);
}

static int put_dns_name(writer_t *w, const char *name)
{
	const char *label = name;
	const char *dot;

	if (name == NULL || name[0] == '\0')
		return 0;

	while (*label != '\0') {
		size_t label_length;
		dot = strchr(label, '.');
		label_length = dot ? (size_t)(dot - label) : strlen(label);
		if (label_length == 0 || label_length > 63)
			return 0;
		put_u8(w, (uint8_t)label_length);
		put_bytes(w, label, label_length);
		if (!w->ok)
			return 0;
		if (dot == NULL) {
			label += label_length;
			break;
		}
		label = dot + 1;
	}
	if (*label != '\0')
		return 0;
	put_u8(w, 0);
	return w->ok;
}

static size_t begin_record(writer_t *w, const char *owner,
	uint16_t type, uint16_t class_value, uint32_t ttl)
{
	size_t rdlength_offset;
	if (!put_dns_name(w, owner))
		return 0;
	put_u16(w, type);
	put_u16(w, class_value);
	put_u32(w, ttl);
	rdlength_offset = w->length;
	put_u16(w, 0);
	return w->ok ? rdlength_offset + 1 : 0;
}

static int end_record(writer_t *w, size_t marker)
{
	size_t field;
	size_t data_length;
	if (!w->ok || marker == 0)
		return 0;
	field = marker - 1;
	data_length = w->length - marker - 1;
	if (data_length > 65535)
		return 0;
	w->bytes[field] = (uint8_t)(data_length >> 8);
	w->bytes[field + 1] = (uint8_t)data_length;
	return 1;
}

static int make_names(char *service, size_t service_capacity,
	char *instance, size_t instance_capacity,
	char *host, size_t host_capacity,
	const char *instance_label, const char *host_label)
{
	int a = snprintf(service, service_capacity, "_raop._tcp.local");
	int b = snprintf(instance, instance_capacity, "%s._raop._tcp.local", instance_label);
	int c = snprintf(host, host_capacity, "%s.local", host_label);
	return a > 0 && (size_t)a < service_capacity &&
		b > 0 && (size_t)b < instance_capacity &&
		c > 0 && (size_t)c < host_capacity;
}

size_t raop_mdns_build_response(uint8_t *out, size_t capacity,
	const char *instance_label, const char *host_label,
	uint16_t port, const uint8_t ipv4[4],
	const char *const *txt, size_t txt_count, uint32_t ttl)
{
	char service[80], instance[160], host[80];
	writer_t w;
	size_t record;
	size_t i;

	if (out == NULL || ipv4 == NULL || instance_label == NULL ||
		host_label == NULL || port == 0 || txt_count == 0 || txt == NULL ||
		!make_names(service, sizeof(service), instance, sizeof(instance),
			host, sizeof(host), instance_label, host_label))
		return 0;

	w.bytes = out;
	w.capacity = capacity;
	w.length = 0;
	w.ok = 1;
	put_u16(&w, 0);       /* transaction ID */
	put_u16(&w, 0x8400);  /* response, authoritative */
	put_u16(&w, 0);       /* questions */
	put_u16(&w, 5);       /* answers */
	put_u16(&w, 0);       /* authority records */
	put_u16(&w, 0);       /* additional records */

	/* PTR: service type -> this receiver instance. */
	record = begin_record(&w, service, 12, 1, ttl);
	if (record == 0 || !put_dns_name(&w, instance) || !end_record(&w, record))
		return 0;

	/* SRV: receiver instance -> RTSP control endpoint. */
	record = begin_record(&w, instance, 33, 0x8001, ttl);
	put_u16(&w, 0);       /* priority */
	put_u16(&w, 0);       /* weight */
	put_u16(&w, port);
	if (record == 0 || !put_dns_name(&w, host) || !end_record(&w, record))
		return 0;

	/* TXT: RAOP capabilities supplied by the future receiver implementation. */
	record = begin_record(&w, instance, 16, 0x8001, ttl);
	for (i = 0; i < txt_count; ++i) {
		size_t length;
		if (txt[i] == NULL)
			return 0;
		length = strlen(txt[i]);
		if (length == 0 || length > 255)
			return 0;
		put_u8(&w, (uint8_t)length);
		put_bytes(&w, txt[i], length);
	}
	if (record == 0 || !end_record(&w, record))
		return 0;

	/* A: host label -> local PS3 IPv4 address. */
	record = begin_record(&w, host, 1, 0x8001, ttl);
	put_bytes(&w, ipv4, 4);
	if (record == 0 || !end_record(&w, record))
		return 0;

	/* DNS-SD service enumeration. */
	record = begin_record(&w, "_services._dns-sd._udp.local", 12, 1, ttl);
	if (record == 0 || !put_dns_name(&w, service) || !end_record(&w, record))
		return 0;

	return w.ok ? w.length : 0;
}
