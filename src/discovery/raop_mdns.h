#ifndef PS3_AIRPLAY_RAOP_MDNS_H
#define PS3_AIRPLAY_RAOP_MDNS_H

#include <stddef.h>
#include <stdint.h>

/*
 * Build an authoritative DNS-SD response for a single RAOP service.
 * IPv4 is supplied as four network-order bytes. TXT entries are raw
 * key/value strings without their DNS length prefix.
 *
 * This function only constructs a packet. It does not open sockets, join
 * multicast groups, or announce a receiver to the LAN.
 * Returns the packet length, or 0 if an input or output buffer is invalid.
 */
size_t raop_mdns_build_response(uint8_t *out, size_t capacity,
	const char *instance_label, const char *host_label,
	uint16_t port, const uint8_t ipv4[4],
	const char *const *txt, size_t txt_count, uint32_t ttl);

#endif
