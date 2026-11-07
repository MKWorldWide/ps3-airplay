#ifndef PS3_AIRPLAY_RAOP_MDNS_SERVICE_H
#define PS3_AIRPLAY_RAOP_MDNS_SERVICE_H

#include <stdint.h>
#include <stddef.h>

typedef enum {
	RAOP_MDNS_FAILURE_NONE = 0,
	RAOP_MDNS_FAILURE_ARGUMENT = 1,
	RAOP_MDNS_FAILURE_SOCKET = 2,
	RAOP_MDNS_FAILURE_TTL = 3,
	RAOP_MDNS_FAILURE_BIND = 4,
	RAOP_MDNS_FAILURE_JOIN = 5,
	RAOP_MDNS_FAILURE_INTERFACE = 6,
	RAOP_MDNS_FAILURE_ANNOUNCE = 7
} raop_mdns_failure_stage;

typedef struct {
	int socket_fd;
	uint8_t ipv4[4];
	char instance_label[64];
	char host_label[64];
	uint16_t port;
	uint64_t next_announce_ms;
	raop_mdns_failure_stage failure_stage;
	int failure_error;
} raop_mdns_service;

/* Return the single source of truth for the service's advertised capabilities. */
size_t raop_mdns_service_txt_records(const char *const **records);

/* Start periodic diagnostic _raop._tcp mDNS announcements on IPv4 multicast. */
int raop_mdns_service_open(raop_mdns_service *service,
	const uint8_t ipv4[4], const char *instance_label,
	const char *host_label, uint16_t port, uint64_t now_ms);
int raop_mdns_service_poll(raop_mdns_service *service, uint64_t now_ms);
/* Sends a TTL-zero goodbye before closing. */
void raop_mdns_service_close(raop_mdns_service *service);

#endif
