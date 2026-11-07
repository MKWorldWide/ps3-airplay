#if defined(__APPLE__)
#define _DARWIN_C_SOURCE
#endif
#define _POSIX_C_SOURCE 200809L

#if defined(RAOP_USE_PSL1GHT)
#include <sys/types.h>
#endif

#include "raop_mdns_service.h"

#include "raop_mdns.h"

#include <stdio.h>
#include <string.h>
#include <errno.h>

#if defined(__powerpc64__) && !defined(RAOP_USE_PSL1GHT)
#include <cell/libnet.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <sys/time.h>
#include <netinet/in.h>
#elif defined(RAOP_USE_PSL1GHT)
#include <sys/time.h>
#include <net/select.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#else
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#endif

#define RAOP_MDNS_ANNOUNCE_INTERVAL_MS 15000U
#define RAOP_MDNS_PACKET_MAX 1200

static const char *const raop_txt_records[] = {
	"txtvers=1", "ch=2", "cn=1", "et=0,1", "sr=44100", "ss=16",
	"tp=UDP", "pw=false", "vn=65537", "vs=0.1.2", "am=PS3", "sf=0x4"
};

size_t raop_mdns_service_txt_records(const char *const **records)
{
	if (records != NULL)
		*records = raop_txt_records;
	return sizeof(raop_txt_records) / sizeof(raop_txt_records[0]);
}

static int contains_bytes(const uint8_t *data, size_t length,
	const char *needle, size_t needle_length)
{
	size_t i;
	if (needle_length > length)
		return 0;
	for (i = 0; i + needle_length <= length; ++i)
		if (memcmp(data + i, needle, needle_length) == 0)
			return 1;
	return 0;
}

static void close_socket(int fd)
{
#if defined(__powerpc64__) && !defined(RAOP_USE_PSL1GHT)
	(void)socketclose(fd);
#else
	(void)close(fd);
#endif
}

static int send_record(raop_mdns_service *service, uint32_t ttl)
{
	const char *const *txt;
	size_t txt_count = raop_mdns_service_txt_records(&txt);
	struct sockaddr_in group;
	uint8_t packet[RAOP_MDNS_PACKET_MAX];
	size_t length;
	ssize_t sent;
	length = raop_mdns_build_response(packet, sizeof(packet),
		service->instance_label, service->host_label, service->port,
		service->ipv4, txt, txt_count, ttl);
	if (length == 0)
		return 0;
	memset(&group, 0, sizeof(group));
	group.sin_family = AF_INET;
	group.sin_addr.s_addr = htonl(0xe00000fbU);
	group.sin_port = htons(5353);
	sent = sendto(service->socket_fd, packet, length, 0,
		(struct sockaddr *)&group, sizeof(group));
	return sent == (ssize_t)length;
}

int raop_mdns_service_open(raop_mdns_service *service,
	const uint8_t ipv4[4], const char *instance_label,
	const char *host_label, uint16_t port, uint64_t now_ms)
{
	int ttl = 255;
	int reuse = 1;
	int fd;
	struct sockaddr_in local;
	struct ip_mreq membership;
	struct in_addr outgoing_interface;
	if (service == NULL || ipv4 == NULL || instance_label == NULL ||
		host_label == NULL || port == 0 || strlen(instance_label) >= 64 ||
		strlen(host_label) >= 64)
		return 0;
	memset(service, 0, sizeof(*service));
	service->socket_fd = -1;
	fd = socket(AF_INET, SOCK_DGRAM, 0);
	if (fd < 0)
		return 0;
	(void)setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse));
	if (setsockopt(fd, IPPROTO_IP, IP_MULTICAST_TTL, &ttl, sizeof(ttl)) < 0) {
		close_socket(fd);
		return 0;
	}
	memset(&local, 0, sizeof(local));
	local.sin_family = AF_INET;
	local.sin_addr.s_addr = htonl(INADDR_ANY);
	local.sin_port = htons(5353);
	if (bind(fd, (struct sockaddr *)&local, sizeof(local)) < 0) {
		close_socket(fd);
		return 0;
	}
	memset(&membership, 0, sizeof(membership));
	membership.imr_multiaddr.s_addr = htonl(0xe00000fbU);
	memcpy(&membership.imr_interface.s_addr, ipv4, 4);
	if (setsockopt(fd, IPPROTO_IP, IP_ADD_MEMBERSHIP, &membership,
		sizeof(membership)) < 0) {
		close_socket(fd);
		return 0;
	}
	outgoing_interface.s_addr = membership.imr_interface.s_addr;
	if (setsockopt(fd, IPPROTO_IP, IP_MULTICAST_IF, &outgoing_interface,
		sizeof(outgoing_interface)) < 0) {
		close_socket(fd);
		return 0;
	}
	service->socket_fd = fd;
	memcpy(service->ipv4, ipv4, 4);
	strcpy(service->instance_label, instance_label);
	strcpy(service->host_label, host_label);
	service->port = port;
	service->next_announce_ms = now_ms;
	if (!send_record(service, 120)) {
		raop_mdns_service_close(service);
		return 0;
	}
	service->next_announce_ms = now_ms + RAOP_MDNS_ANNOUNCE_INTERVAL_MS;
	return 1;
}

int raop_mdns_service_poll(raop_mdns_service *service, uint64_t now_ms)
{
	fd_set read_set;
	struct timeval timeout = {0, 0};
	uint8_t query[9000];
	int ready;
	if (service == NULL || service->socket_fd < 0)
		return -1;
	FD_ZERO(&read_set);
	FD_SET(service->socket_fd, &read_set);
	ready = select(service->socket_fd + 1, &read_set, NULL, NULL, &timeout);
	if (ready < 0 && errno != EINTR)
		return -1;
	if (ready > 0 && FD_ISSET(service->socket_fd, &read_set)) {
		ssize_t amount = recv(service->socket_fd, query, sizeof(query), 0);
		if (amount >= 12 && (query[2] & 0x80) == 0 &&
			(contains_bytes(query, (size_t)amount, "_raop", 5) ||
			 contains_bytes(query, (size_t)amount, "_services._dns-sd", 16))) {
			if (!send_record(service, 120))
				return -1;
			service->next_announce_ms = now_ms + RAOP_MDNS_ANNOUNCE_INTERVAL_MS;
			return 1;
		}
	}
	if (now_ms < service->next_announce_ms)
		return 0;
	if (!send_record(service, 120))
		return -1;
	service->next_announce_ms = now_ms + RAOP_MDNS_ANNOUNCE_INTERVAL_MS;
	return 1;
}

void raop_mdns_service_close(raop_mdns_service *service)
{
	if (service == NULL)
		return;
	if (service->socket_fd >= 0) {
		struct ip_mreq membership;
		(void)send_record(service, 0);
		memset(&membership, 0, sizeof(membership));
		membership.imr_multiaddr.s_addr = htonl(0xe00000fbU);
		memcpy(&membership.imr_interface.s_addr, service->ipv4, 4);
		(void)setsockopt(service->socket_fd, IPPROTO_IP, IP_DROP_MEMBERSHIP,
			&membership, sizeof(membership));
		close_socket(service->socket_fd);
	}
	memset(service, 0, sizeof(*service));
	service->socket_fd = -1;
}
