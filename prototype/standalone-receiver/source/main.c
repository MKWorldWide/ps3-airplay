#include <sys/types.h>
#include <sys/time.h>

#include "raop_alac.h"
#include "raop_crypto.h"
#include "raop_mdns_service.h"
#include "pcm_ring.h"
#include "diagnostic_tone.h"
#include "raop_receiver.h"
#include "diagnostic_screen.h"

#include <audio/audio.h>
#include <io/pad.h>
#include <lv2/system.h>
#include <lv2/systime.h>
#include <net/net.h>
#include <net/netctl.h>
#include <sys/event_queue.h>
#include <sys/socket.h>
#include <sysmodule/sysmodule.h>
#include <sysutil/sysutil.h>
#include <unistd.h>

#include <arpa/inet.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <time.h>

#if defined(MBEDTLS_PLATFORM_MS_TIME_ALT)
#include <mbedtls/platform_time.h>
mbedtls_ms_time_t mbedtls_ms_time(void)
{
	return (mbedtls_ms_time_t)(sysGetSystemTime() / 1000U);
}
#endif

#define RECEIVER_LOG_PATH "/dev_hdd0/tmp/ps3-airplay.log"
#define BACKGROUND_RGB_PATH "/dev_hdd0/game/AIRPLAY02/USRDIR/background.rgb"
#define RTSP_PORT 5000
#define RTP_BASE_PORT 6000
#define AUDIO_PORT_LEVEL 0.25f

static volatile int exit_requested;
static raop_receiver receiver;
static raop_mdns_service mdns_service;
static raop_crypto_context crypto_context;
static receiver_log_file diagnostic_log;
static raop_alac_decoder *decoder;
static raop_pcm_ring pcm_ring;
static raop_diagnostic_tone diagnostic_tone;
static diagnostic_screen screen;
static int diagnostic_tone_logged;
static uint8_t decoded_pcm[RAOP_ALAC_PCM_MAX];
static uint64_t decoded_packet_count;
static int audio_prebuffer_ready;
static uint32_t active_sample_rate = 48000;

static void on_system_event(u64 status, u64 param, void *userdata)
{
	(void)param;
	(void)userdata;
	if (status == SYSUTIL_EXIT_GAME)
		exit_requested = 1;
}

static int log_event(const char *event, int value, size_t bytes)
{
	time_t now = time(NULL);
	struct tm *utc = gmtime(&now);
	char timestamp[21];
	if (utc == NULL || strftime(timestamp, sizeof(timestamp),
		"%Y-%m-%dT%H:%M:%SZ", utc) == 0)
		memcpy(timestamp, "1970-01-01T00:00:00Z", sizeof(timestamp));
	return receiver_log_file_event(&diagnostic_log, timestamp, event, value, bytes);
}

static int local_ipv4(uint8_t address[4])
{
	struct sockaddr_in multicast = {0};
	struct sockaddr_in local = {0};
	socklen_t length = sizeof(local);
	int fd = socket(AF_INET, SOCK_DGRAM, 0);
	int result = 0;
	if (fd < 0)
		return 0;
	multicast.sin_family = AF_INET;
	multicast.sin_addr.s_addr = htonl(0xe00000fbU);
	multicast.sin_port = htons(5353);
	if (connect(fd, (struct sockaddr *)&multicast, sizeof(multicast)) == 0 &&
		getsockname(fd, (struct sockaddr *)&local, &length) == 0 &&
		local.sin_addr.s_addr != htonl(INADDR_ANY)) {
		memcpy(address, &local.sin_addr.s_addr, 4);
		result = 1;
	}
	(void)close(fd);
	return result;
}

static void fill_audio_silence(const audioPortConfig *config)
{
	float *audio_data = (float *)(uintptr_t)config->audioDataStart;
	memset(audio_data, 0, (size_t)config->numBlocks *
		(size_t)config->channelCount * AUDIO_BLOCK_SAMPLES * sizeof(float));
}

static void render_next_audio_block(const audioPortConfig *config)
{
	volatile u64 *read_index = (volatile u64 *)(uintptr_t)config->readIndex;
	u32 next_block = (u32)((*read_index + 1U) % config->numBlocks);
	float *audio_data = (float *)(uintptr_t)config->audioDataStart;
	float *block = audio_data + next_block * config->channelCount *
		AUDIO_BLOCK_SAMPLES;
	if (!raop_diagnostic_tone_complete(&diagnostic_tone)) {
		(void)raop_diagnostic_tone_render(&diagnostic_tone, block,
			AUDIO_BLOCK_SAMPLES);
		if (!diagnostic_tone_logged &&
			raop_diagnostic_tone_complete(&diagnostic_tone)) {
			diagnostic_tone_logged = 1;
			(void)log_event("AUDIO_TEST_TONE_COMPLETE",
				(int)RAOP_DIAGNOSTIC_TONE_FREQUENCY,
				RAOP_DIAGNOSTIC_TONE_FRAMES);
		}
		return;
	}
	(void)raop_pcm_ring_render_float(&pcm_ring, active_sample_rate,
		RAOP_DIAGNOSTIC_TONE_RATE, block, AUDIO_BLOCK_SAMPLES);
}

static void close_audio(int started, int notify_set, int port_open,
	int queue_created, int initialized, u32 port, sys_event_queue_t queue,
	sys_ipc_key_t key)
{
	if (started)
		(void)audioPortStop(port);
	if (notify_set)
		(void)audioRemoveNotifyEventQueue(key);
	if (port_open)
		(void)audioPortClose(port);
	if (queue_created)
		(void)sysEventQueueDestroy(queue, 0);
	if (initialized)
		(void)audioQuit();
}

int main(void)
{
	int result = 1;
	int net_started = 0, netctl_module_loaded = 0, netctl_started = 0;
	int crypto_started = 0, receiver_opened = 0, mdns_opened = 0;
	int audio_initialized = 0, audio_queue_created = 0, audio_notify_set = 0;
	int audio_port_opened = 0, audio_port_started = 0, callback_registered = 0;
	int pad_initialized = 0;
	u32 audio_port = 0;
	sys_event_queue_t audio_queue;
	sys_ipc_key_t audio_key;
	audioPortParam audio_params = {0};
	audioPortConfig audio_config = {0};
	union net_ctl_info net_info;
	uint8_t ip[4];
	char instance[64], host[64];
	u64 next_log_us = 0;
	int screen_result;
	uint64_t reported_underflows = 0;
	uint64_t reported_drops = 0;

	mdns_service.socket_fd = -1;
	raop_pcm_ring_init(&pcm_ring, 2);
	if (!receiver_log_file_open(&diagnostic_log, RECEIVER_LOG_PATH)) {
		printf("Cannot open capped log at %s\n", RECEIVER_LOG_PATH);
		goto cleanup;
	}
	(void)log_event("APP_START", 0, 0);
	screen_result = diagnostic_screen_open(&screen, BACKGROUND_RGB_PATH);
	if (screen_result == DIAGNOSTIC_SCREEN_OK)
		(void)log_event("SCREEN_READY", 1, 0);
	else
		(void)log_event("SCREEN_INIT_FAILED", screen_result, 0);
	if (ioPadInit(7) == 0) {
		pad_initialized = 1;
		(void)log_event("CONTROLLER_READY", 1, 0);
	} else {
		(void)log_event("CONTROLLER_INIT_FAILED", -1, 0);
	}
	if (netInitialize() != 0) {
		(void)log_event("NET_INIT_FAILED", -1, 0);
		goto cleanup;
	}
	net_started = 1;
	if (sysModuleLoad(SYSMODULE_NETCTL) < 0) {
		(void)log_event("NETCTL_LOAD_FAILED", -1, 0);
		goto cleanup;
	}
	netctl_module_loaded = 1;
	if (netCtlInit() != 0) {
		(void)log_event("NETCTL_INIT_FAILED", -1, 0);
		goto cleanup;
	}
	netctl_started = 1;
	if (netCtlGetInfo(NET_CTL_INFO_ETHER_ADDR, &net_info) != 0) {
		(void)log_event("NETCTL_INFO_FAILED", -1, 0);
		goto cleanup;
	}
	if (!raop_crypto_context_init(&crypto_context, raop_crypto_ps3_rng, NULL)) {
		(void)log_event("CRYPTO_INIT_FAILED", -1, 0);
		goto cleanup;
	}
	crypto_started = 1;
	if (!raop_receiver_open(&receiver, RTSP_PORT, RTP_BASE_PORT,
		&diagnostic_log)) {
		(void)log_event("RECEIVER_OPEN_FAILED", -1, 0);
		goto cleanup;
	}
	receiver_opened = 1;
	if (!raop_receiver_configure_crypto(&receiver, &crypto_context,
		raop_crypto_ps3_rng, NULL, net_info.ether_addr.data)) {
		(void)log_event("RECEIVER_CRYPTO_FAILED", -1, 0);
		goto cleanup;
	}
	if (!local_ipv4(ip)) {
		(void)log_event("LOCAL_IP_FAILED", -1, 0);
		goto cleanup;
	}
	(void)snprintf(instance, sizeof(instance),
		"%02X%02X%02X%02X%02X%02X@PS3-AirPlay-Diagnostic",
		net_info.ether_addr.data[0], net_info.ether_addr.data[1],
		net_info.ether_addr.data[2], net_info.ether_addr.data[3],
		net_info.ether_addr.data[4], net_info.ether_addr.data[5]);
	(void)snprintf(host, sizeof(host), "ps3-airplay-%02x%02x%02x%02x",
		ip[0], ip[1], ip[2], ip[3]);
	mdns_opened = raop_mdns_service_open(&mdns_service, ip, instance, host,
		raop_receiver_rtsp_port(&receiver), (uint64_t)(sysGetSystemTime() / 1000U));
	(void)log_event(mdns_opened ? "MDNS_READY" : "MDNS_FAILED", mdns_opened, 0);
	if (sysUtilRegisterCallback(SYSUTIL_EVENT_SLOT0, on_system_event, NULL) != 0) {
		(void)log_event("CALLBACK_REGISTER_FAILED", -1, 0);
		goto cleanup;
	}
	callback_registered = 1;
	if (audioInit() != 0) {
		(void)log_event("AUDIO_INIT_FAILED", -1, 0);
		goto cleanup;
	}
	audio_initialized = 1;
	audio_params.numChannels = AUDIO_PORT_2CH;
	audio_params.numBlocks = AUDIO_BLOCK_8;
	audio_params.attrib = AUDIO_PORT_INITLEVEL;
	audio_params.level = AUDIO_PORT_LEVEL;
	if (audioPortOpen(&audio_params, &audio_port) != 0) {
		(void)log_event("AUDIO_OPEN_FAILED", -1, 0);
		goto cleanup;
	}
	audio_port_opened = 1;
	if (audioGetPortConfig(audio_port, &audio_config) != 0) {
		(void)log_event("AUDIO_CONFIG_FAILED", -1, 0);
		goto cleanup;
	}
	if (audioCreateNotifyEventQueue(&audio_queue, &audio_key) != 0) {
		(void)log_event("AUDIO_QUEUE_CREATE_FAILED", -1, 0);
		goto cleanup;
	}
	audio_queue_created = 1;
	if (audioSetNotifyEventQueue(audio_key) != 0) {
		(void)log_event("AUDIO_QUEUE_SET_FAILED", -1, 0);
		goto cleanup;
	}
	audio_notify_set = 1;
	if (sysEventQueueDrain(audio_queue) != 0) {
		(void)log_event("AUDIO_QUEUE_DRAIN_FAILED", -1, 0);
		goto cleanup;
	}
	fill_audio_silence(&audio_config);
	if (audioPortStart(audio_port) != 0) {
		(void)log_event("AUDIO_START_FAILED", -1, 0);
		goto cleanup;
	}
	audio_port_started = 1;
	(void)log_event("AUDIO_READY", (int)audio_config.channelCount, 0);
	raop_diagnostic_tone_init(&diagnostic_tone);
	(void)log_event("AUDIO_TEST_TONE_START",
		(int)RAOP_DIAGNOSTIC_TONE_FREQUENCY,
		RAOP_DIAGNOSTIC_TONE_FRAMES);
	printf("RAOP diagnostic receiver ready. Log: %s\n", RECEIVER_LOG_PATH);

	while (!exit_requested) {
		int poll_result = raop_receiver_poll(&receiver, 1);
		if (poll_result < 0) {
			(void)log_event("RECEIVER_ERROR", poll_result, 0);
			break;
		}
		if (mdns_opened && raop_mdns_service_poll(&mdns_service,
			(uint64_t)(sysGetSystemTime() / 1000U)) < 0) {
			(void)log_event("MDNS_POLL_FAILED", -1, 0);
			raop_mdns_service_close(&mdns_service);
			mdns_opened = 0;
		}
		if (pad_initialized) {
			padInfo info;
			padData data;
			int pad;
			if (ioPadGetInfo(&info) == 0) {
				for (pad = 0; pad < MAX_PADS; ++pad) {
					if (info.status[pad] && ioPadGetData((u32)pad, &data) == 0 &&
						data.BTN_START) {
						(void)log_event("USER_EXIT_START", pad, 0);
						exit_requested = 1;
						break;
					}
				}
			}
		}
		if (receiver.session.state == RAOP_SESSION_RECORDING && decoder == NULL) {
			if (receiver.session.offer.has_alac &&
				receiver.session.offer.has_fmtp)
				decoder = raop_alac_create(&receiver.session.offer);
			active_sample_rate = decoder == NULL ? 48000 :
				receiver.session.offer.fmtp[10];
			raop_pcm_ring_init(&pcm_ring,
				decoder == NULL ? 2 : (uint8_t)receiver.session.offer.fmtp[6]);
			(void)log_event(decoder != NULL ? "ALAC_READY" : "ALAC_UNSUPPORTED",
				(int)receiver.session.offer.fmtp[2],
				(int)receiver.session.offer.fmtp[6]);
			audio_prebuffer_ready = 0;
			decoded_packet_count = 0;
		}
		if (receiver.session.state != RAOP_SESSION_RECORDING && decoder != NULL) {
			raop_alac_destroy(decoder);
			decoder = NULL;
			active_sample_rate = 48000;
			memset(decoded_pcm, 0, sizeof(decoded_pcm));
			raop_pcm_ring_init(&pcm_ring, 2);
			memset(&receiver.audio_queue, 0, sizeof(receiver.audio_queue));
			audio_prebuffer_ready = 0;
		}
		if (decoder != NULL && !audio_prebuffer_ready &&
			receiver.audio_queue.count >= RAOP_RTP_PREBUFFER_PACKETS) {
			audio_prebuffer_ready = 1;
			(void)log_event("AUDIO_PREBUFFER_READY",
				(int)receiver.audio_queue.count, 0);
		}
		{
			raop_rtp_frame frame;
			unsigned int drained = 0;
			while (audio_prebuffer_ready && drained < 32 &&
				pcm_ring.count <= RAOP_PCM_RING_CAPACITY - RAOP_ALAC_FRAME_MAX &&
				raop_receiver_pop_audio(&receiver, &frame)) {
				++drained;
				if (decoder != NULL) {
					size_t pcm_length = 0;
					uint32_t samples = 0;
					if (raop_alac_decode(decoder, frame.payload,
						frame.packet.payload_length, decoded_pcm,
						sizeof(decoded_pcm), &pcm_length, &samples)) {
						(void)raop_pcm_ring_push_le16(&pcm_ring, decoded_pcm,
							pcm_length);
						++decoded_packet_count;
						if (decoded_packet_count == 1 ||
							(decoded_packet_count % 128U) == 0)
							(void)log_event("ALAC_FRAME", (int)samples,
								pcm_length);
					} else {
						(void)log_event("ALAC_DECODE_FAILED",
							frame.packet.sequence, frame.packet.payload_length);
					}
					memset(decoded_pcm, 0, sizeof(decoded_pcm));
				}
			memset(frame.payload, 0, sizeof(frame.payload));
			}
		}
		{
			sys_event_t event;
			if (sysEventQueueReceive(audio_queue, &event, 1000) == 0)
				render_next_audio_block(&audio_config);
		}
		sysUtilCheckCallback();
		if ((u64)sysGetSystemTime() >= next_log_us) {
			uint64_t current = pcm_ring.output_underflows;
			if (current != reported_underflows)
				(void)log_event("PCM_UNDERFLOW", (int)(current - reported_underflows),
					(size_t)pcm_ring.count);
			reported_underflows = current;
			current = pcm_ring.frames_dropped;
			if (current != reported_drops)
				(void)log_event("PCM_DROP", (int)(current - reported_drops),
					(size_t)pcm_ring.count);
			reported_drops = current;
			next_log_us = sysGetSystemTime() + 1000000U;
		}
	}
	result = 0;

cleanup:
	if (pad_initialized)
		(void)ioPadEnd();
	diagnostic_screen_close(&screen);
	if (decoder != NULL)
		raop_alac_destroy(decoder);
	memset(decoded_pcm, 0, sizeof(decoded_pcm));
	if (audio_port_started || audio_notify_set || audio_port_opened ||
		audio_queue_created || audio_initialized)
		close_audio(audio_port_started, audio_notify_set, audio_port_opened,
			audio_queue_created, audio_initialized, audio_port, audio_queue,
			audio_key);
	if (callback_registered)
		(void)sysUtilUnregisterCallback(SYSUTIL_EVENT_SLOT0);
	if (mdns_opened)
		raop_mdns_service_close(&mdns_service);
	if (receiver_opened)
		raop_receiver_close(&receiver);
	if (crypto_started)
		raop_crypto_context_free(&crypto_context);
	(void)log_event("APP_STOP", result, 0);
	receiver_log_file_close(&diagnostic_log);
	if (netctl_started)
		netCtlTerm();
	if (netctl_module_loaded)
		(void)sysModuleUnload(SYSMODULE_NETCTL);
	if (net_started)
		netDeinitialize();
	return result;
}
