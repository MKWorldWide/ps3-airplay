#include "raop_receiver.h"
#include "raop_alac.h"
#include "raop_mdns_service.h"

#include <cell/libnet.h>
#include <cell/libnetctl.h>
#include <cell/sysmodule.h>
#include <sys/prx.h>
#include <sys/thread.h>
#include <sys/prx_module.h>
#include <sys/sys_time.h>

#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <mbedtls/platform_util.h>
#if !defined(__powerpc64__)
#include <unistd.h>
#endif

#ifndef RECEIVER_LOG_PATH
#define RECEIVER_LOG_PATH "/dev_hdd0/tmp/ps3-airplay.log"
#endif
#define RECEIVER_WORKER_STACK 0x10000
#define WORKER_STOP_TIMEOUT_US 5000000ULL
#define WORKER_STOP_MAX_POLLS 1000000U
#ifndef RESIDENT_RTSP_PORT
#define RESIDENT_RTSP_PORT 5000
#endif
#ifndef RESIDENT_RTP_BASE_PORT
#define RESIDENT_RTP_BASE_PORT 6000
#endif

extern void __libc_init_array(void);
extern void __libc_fini_array(void);

static receiver_log_file receiver_log;
static raop_receiver receiver;
static raop_mdns_service mdns_service;
static raop_crypto_context crypto_context;
static sys_ppu_thread_t worker_thread;
static volatile int stop_requested;
static volatile int worker_exited;
static volatile int receiver_log_lock;
static int worker_started;
static int net_module_loaded;
static int net_initialized;
static int netctl_module_loaded;
static int netctl_initialized;
static int crypto_initialized;
static uint8_t console_mac[6];
static int have_console_mac;
static uint8_t decoder_pcm[RAOP_ALAC_PCM_MAX];

static uint64_t monotonic_milliseconds(void)
{
	struct timespec now;
	if (clock_gettime(CLOCK_MONOTONIC, &now) != 0)
		return 0;
	return (uint64_t)now.tv_sec * 1000U + (uint64_t)now.tv_nsec / 1000000U;
}

static int local_ipv4(uint8_t address[4])
{
	struct sockaddr_in multicast;
	struct sockaddr_in local;
	socklen_t length = (socklen_t)sizeof(local);
	int fd = socket(AF_INET, SOCK_DGRAM, 0);
	int result = 0;
	if (fd < 0)
		return 0;
	memset(&multicast, 0, sizeof(multicast));
	multicast.sin_family = AF_INET;
	multicast.sin_addr.s_addr = htonl(0xe00000fbU);
	multicast.sin_port = htons(5353);
	if (connect(fd, (struct sockaddr *)&multicast, sizeof(multicast)) == 0 &&
		getsockname(fd, (struct sockaddr *)&local, &length) == 0 &&
		local.sin_addr.s_addr != htonl(INADDR_ANY)) {
		memcpy(address, &local.sin_addr.s_addr, 4);
		result = 1;
	}
#if defined(__powerpc64__)
	(void)socketclose(fd);
#else
	(void)close(fd);
#endif
	return result;
}

static void timestamp_utc(char timestamp[21])
{
	time_t now = time(NULL);
	struct tm *utc = gmtime(&now);
	if (utc == NULL || strftime(timestamp, 21, "%Y-%m-%dT%H:%M:%SZ", utc) == 0)
		memcpy(timestamp, "1970-01-01T00:00:00Z", 21);
}

static int log_event(const char *event, int value, size_t bytes)
{
	char timestamp[21];
	int acquired = 0;
	int attempt;
	int logged;
	for (attempt = 0; attempt < 1000; ++attempt) {
		if (__atomic_exchange_n(&receiver_log_lock, 1, __ATOMIC_ACQUIRE) == 0) {
			acquired = 1;
			break;
		}
		(void)sysThreadYield();
	}
	if (!acquired)
		return 0;
	timestamp_utc(timestamp);
	logged = receiver_log_file_event(&receiver_log, timestamp, event, value,
		bytes);
	__atomic_store_n(&receiver_log_lock, 0, __ATOMIC_RELEASE);
	return logged;
}

static void receiver_worker(void *argument)
{
	uint8_t address[4];
	char instance[64];
	char host[64];
	raop_alac_decoder *decoder = NULL;
	uint64_t decoded_frames = 0;
	int result;
	(void)argument;
	mdns_service.socket_fd = -1;

	if (!raop_receiver_open(&receiver, RESIDENT_RTSP_PORT,
		RESIDENT_RTP_BASE_PORT, &receiver_log)) {
		(void)log_event("RECEIVER_OPEN_FAILED", -1, 0);
		__atomic_store_n(&worker_exited, 1, __ATOMIC_RELEASE);
		sysThreadExit(0);
	}
	if (!log_event("RECEIVER_READY", raop_receiver_rtsp_port(&receiver), 0)) {
		raop_receiver_close(&receiver);
		__atomic_store_n(&worker_exited, 1, __ATOMIC_RELEASE);
		sysThreadExit(0);
	}
	if (local_ipv4(address)) {
		if (crypto_initialized && have_console_mac &&
			raop_receiver_configure_crypto(&receiver, &crypto_context,
				raop_crypto_ps3_rng, NULL, console_mac)) {
			(void)log_event("CRYPTO_READY", 1, 0);
		} else {
			(void)log_event("CRYPTO_UNAVAILABLE", -1, 0);
		}
		if (have_console_mac) {
			(void)snprintf(instance, sizeof(instance),
				"%02X%02X%02X%02X%02X%02X@PS3-AirPlay-Diagnostic",
				console_mac[0], console_mac[1], console_mac[2], console_mac[3],
				console_mac[4], console_mac[5]);
		} else {
			(void)snprintf(instance, sizeof(instance),
				"%02X%02X%02X%02X0001@PS3-AirPlay-Diagnostic",
				address[0], address[1], address[2], address[3]);
		}
		(void)snprintf(host, sizeof(host), "ps3-airplay-%02x%02x%02x%02x",
			address[0], address[1], address[2], address[3]);
		if (raop_mdns_service_open(&mdns_service, address, instance, host,
			raop_receiver_rtsp_port(&receiver), monotonic_milliseconds()))
			(void)log_event("MDNS_ADVERTISING", 1, 0);
		else
			(void)log_event("MDNS_OPEN_FAILED", -1, 0);
	} else {
		(void)log_event("MDNS_IP_FAILED", -1, 0);
	}

	while (!__atomic_load_n(&stop_requested, __ATOMIC_ACQUIRE)) {
		result = raop_receiver_poll(&receiver, 100);
		if (result < 0) {
			(void)log_event("RECEIVER_ERROR", result, 0);
			break;
		}
		{
			raop_rtp_frame frame;
			unsigned int drained = 0;
			size_t bytes = 0;
		if (receiver.session.state == RAOP_SESSION_RECORDING && decoder == NULL) {
			decoder = raop_alac_create(&receiver.session.offer);
			(void)log_event(decoder != NULL ? "ALAC_READY" : "ALAC_UNSUPPORTED",
				(int)receiver.session.offer.fmtp[2],
				(int)receiver.session.offer.fmtp[6]);
		}
		if (receiver.session.state != RAOP_SESSION_RECORDING && decoder != NULL) {
			raop_alac_destroy(decoder);
			decoder = NULL;
			decoded_frames = 0;
			memset(decoder_pcm, 0, sizeof(decoder_pcm));
			memset(&receiver.audio_queue, 0, sizeof(receiver.audio_queue));
		}
			while (drained < RAOP_RTP_QUEUE_CAPACITY &&
				raop_receiver_pop_audio(&receiver, &frame)) {
				++drained;
				bytes += frame.packet.payload_length;
				if (decoder != NULL) {
					size_t pcm_length = 0;
					uint32_t samples = 0;
					if (raop_alac_decode(decoder, frame.payload,
						frame.packet.payload_length, decoder_pcm,
						sizeof(decoder_pcm), &pcm_length, &samples)) {
						++decoded_frames;
						if (decoded_frames == 1 || (decoded_frames % 128U) == 0)
							(void)log_event("ALAC_FRAME",
								(int)samples, (int)pcm_length);
					} else {
						(void)log_event("ALAC_DECODE_FAILED",
							frame.packet.sequence,
							(int)frame.packet.payload_length);
					}
					memset(decoder_pcm, 0, sizeof(decoder_pcm));
				}
				mbedtls_platform_zeroize(frame.payload, sizeof(frame.payload));
			}
			if (drained != 0) {
				(void)log_event(receiver.session.has_audio_key ?
					"RTP_DECRYPTED_BATCH" : "RTP_PAYLOAD_BATCH",
					(int)drained, bytes);
			}
		}
		if (mdns_service.socket_fd >= 0 &&
			raop_mdns_service_poll(&mdns_service, monotonic_milliseconds()) < 0) {
			(void)log_event("MDNS_SEND_FAILED", -1, 0);
			raop_mdns_service_close(&mdns_service);
		}
	}

	if (decoder != NULL)
		raop_alac_destroy(decoder);
	memset(decoder_pcm, 0, sizeof(decoder_pcm));
	raop_mdns_service_close(&mdns_service);
	raop_receiver_close(&receiver);
	(void)log_event("WORKER_STOPPED", 0, 0);
	__atomic_store_n(&worker_exited, 1, __ATOMIC_RELEASE);
	sysThreadExit(0);
}

static void release_network(void)
{
	if (netctl_initialized) {
		cellNetCtlTerm();
		netctl_initialized = 0;
	}
	if (netctl_module_loaded) {
		(void)cellSysmoduleUnloadModule(CELL_SYSMODULE_NETCTL);
		netctl_module_loaded = 0;
	}
	if (net_initialized) {
		(void)sys_net_finalize_network();
		net_initialized = 0;
	}
	if (net_module_loaded) {
		(void)cellSysmoduleUnloadModule(CELL_SYSMODULE_NET);
		net_module_loaded = 0;
	}
}

static void release_crypto(void)
{
	if (crypto_initialized) {
		raop_crypto_context_free(&crypto_context);
		crypto_initialized = 0;
	}
}

int module_start(size_t args, void *argp)
{
	int result = -1;
	(void)args;
	(void)argp;

	/* The process CRT initializes heap and constructors; this PRX owns its
	 * lifecycle and does not use the TLS-backed sys_ppu_thread_create shim. */
	__libc_init_array();
	stop_requested = 0;
	worker_exited = 0;
	worker_started = 0;
	net_module_loaded = 0;
	net_initialized = 0;
	netctl_module_loaded = 0;
	netctl_initialized = 0;
	crypto_initialized = 0;
	have_console_mac = 0;

	if (!receiver_log_file_open(&receiver_log, RECEIVER_LOG_PATH))
		goto fail_runtime;
	if (!log_event("MODULE_START", 0, 0))
		goto fail;

	result = cellSysmoduleLoadModule(CELL_SYSMODULE_NET);
	if (result != 0) {
		(void)log_event("NET_MODULE_FAILED", result, 0);
		goto fail;
	}
	net_module_loaded = 1;
	result = sys_net_initialize_network();
	if (result != 0) {
		(void)log_event("NET_INIT_FAILED", result, 0);
		goto fail;
	}
	net_initialized = 1;
	if (!log_event("NET_READY", 0, 0)) {
		result = -1;
		goto fail;
	}
	result = cellSysmoduleLoadModule(CELL_SYSMODULE_NETCTL);
	if (result == 0) {
		union CellNetCtlInfo net_info;
		netctl_module_loaded = 1;
		result = cellNetCtlInit();
		if (result == 0) {
			netctl_initialized = 1;
			memset(&net_info, 0, sizeof(net_info));
			result = cellNetCtlGetInfo(CELL_NET_CTL_INFO_ETHER_ADDR,
				&net_info);
			if (result == 0) {
				memcpy(console_mac, net_info.ether_addr.data,
					sizeof(console_mac));
				have_console_mac = 1;
				(void)log_event("MAC_READY", 1, 0);
			} else {
				(void)log_event("MAC_READ_FAILED", result, 0);
			}
		} else {
			(void)log_event("NETCTL_INIT_FAILED", result, 0);
		}
	} else {
		(void)log_event("NETCTL_LOAD_FAILED", result, 0);
	}
	if (raop_crypto_context_init(&crypto_context, raop_crypto_ps3_rng, NULL)) {
		crypto_initialized = 1;
		(void)log_event("CRYPTO_KEY_READY", 1, 0);
	} else {
		(void)log_event("CRYPTO_KEY_FAILED", -1, 0);
	}

	result = sysThreadCreate(&worker_thread, receiver_worker, NULL,
		3000, RECEIVER_WORKER_STACK, THREAD_JOINABLE,
		"ps3-airplay");
	if (result != 0) {
		(void)log_event("WORKER_CREATE_FAILED", result, 0);
		goto fail;
	}
	worker_started = 1;
	return SYS_PRX_RESIDENT;

fail:
	release_network();
	release_crypto();
	(void)log_event("MODULE_START_FAILED", result, 0);
	receiver_log_file_close(&receiver_log);
fail_runtime:
	__libc_fini_array();
	return SYS_PRX_NO_RESIDENT;
}

int module_stop(size_t args, void *argp)
{
	uint64_t thread_result = 0;
	uint64_t wait_started_us;
	uint32_t wait_polls = 0;
	int join_result;
	(void)args;
	(void)argp;

	__atomic_store_n(&stop_requested, 1, __ATOMIC_RELEASE);
	if (worker_started) {
		wait_started_us = sys_time_get_system_time();
		while (!__atomic_load_n(&worker_exited, __ATOMIC_ACQUIRE) &&
			wait_polls < WORKER_STOP_MAX_POLLS) {
			if (sys_time_get_system_time() - wait_started_us >=
				WORKER_STOP_TIMEOUT_US)
				break;
			++wait_polls;
			(void)sysThreadYield();
		}
		if (!__atomic_load_n(&worker_exited, __ATOMIC_ACQUIRE))
			return SYS_PRX_STOP_FAILED;
		/* The worker has completed receiver cleanup and is at sysThreadExit;
		 * join now only to reap its already-terminating joinable thread. */
		join_result = sysThreadJoin(worker_thread, &thread_result);
		if (join_result != 0) {
			/* The listener may still be writing directly to the same log.
			 * Keep the module and resources resident; avoid concurrent stdio. */
			return SYS_PRX_STOP_FAILED;
		}
		worker_started = 0;
	}
	(void)log_event("MODULE_STOP", (int)thread_result, 0);
	release_crypto();
	release_network();
	receiver_log_file_close(&receiver_log);
	__libc_fini_array();
	return SYS_PRX_STOP_OK;
}
