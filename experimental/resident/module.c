#include "raop_receiver.h"
#include "raop_alac.h"
#include "raop_mdns_service.h"
#include "resident_diagnostics.h"
#include "pcm_ring.h"
#include "pcm_spsc.h"

#include <cell/audio.h>
#include <cell/libnet.h>
#include <cell/libnetctl.h>
#include <cell/sysmodule.h>
#include <sys/prx.h>
#include <sys/thread.h>
#include <sys/prx_module.h>
#include <sys/sys_time.h>
#include <sys/timer.h>

#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <stdint.h>
#include <math.h>
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
#define RESIDENT_AUDIO_STACK 0x10000
#define AUDIO_PREBUFFER_FRAMES 2048U
#define AUDIO_REFILL_FRAMES 1024U
#define AUDIO_OUTPUT_FRAMES CELL_AUDIO_BLOCK_SAMPLES
#define AUDIO_TEST_TONE_FRAMES RAOP_PCM_SPSC_CAPACITY
#define AUDIO_TEST_TONE_CHUNK_FRAMES 1024U
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
static sys_ppu_thread_t audio_thread;
static volatile int stop_requested;
static volatile int worker_exited;
static volatile int receiver_log_lock;
static int worker_started;
static int audio_thread_started;
static int audio_module_loaded;
static int audio_initialized_by_module;
static int audio_port_opened;
static int audio_port_started;
static uint32_t audio_port;
static volatile int audio_stop_requested;
static volatile int audio_worker_exited;
static volatile uint32_t pcm_generation;
static volatile uint32_t pcm_input_rate;
static volatile uint32_t pcm_input_channels;
static raop_pcm_spsc decoded_pcm_queue;
/* These single-worker-owned buffers exceed the 64 KiB worker stack. */
static raop_pcm_ring audio_pcm_ring;
static int16_t audio_staging[AUDIO_REFILL_FRAMES * 2U];
static float audio_output[AUDIO_OUTPUT_FRAMES * 2U];
static int net_module_loaded;
static int net_initialized;
static int netctl_module_loaded;
static int netctl_initialized;
static int crypto_initialized;
static uint8_t console_mac[6];
static int have_console_mac;
static uint8_t decoder_pcm[RAOP_ALAC_PCM_MAX];
static airps3_resident_diagnostics *active_diagnostics;

static void publish_diagnostic_event(uint32_t event)
{
	if (active_diagnostics != NULL)
		(void)__atomic_fetch_or(&active_diagnostics->events, event,
			__ATOMIC_RELEASE);
}

static void publish_start_diagnostic(size_t args, void *argp,
	uint32_t stage, int result)
{
	airps3_resident_diagnostics *diagnostics =
		(airps3_resident_diagnostics *)argp;
	if (diagnostics != NULL && args == sizeof(*diagnostics) &&
		diagnostics->magic == AIRPS3_RESIDENT_DIAGNOSTICS_MAGIC) {
		active_diagnostics = diagnostics;
		diagnostics->stage = stage;
		diagnostics->error = result;
		airps3_diag_log(diagnostics, sys_time_get_system_time(),
			"MODULE_START_STAGE", result, stage, 0);
	}
}

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
	if (active_diagnostics != NULL)
		airps3_diag_log(active_diagnostics, sys_time_get_system_time(), event,
			(int32_t)value, 0U,
			bytes > UINT32_MAX ? UINT32_MAX : (uint32_t)bytes);
	for (attempt = 0; attempt < 1000; ++attempt) {
		if (__atomic_exchange_n(&receiver_log_lock, 1, __ATOMIC_ACQUIRE) == 0) {
			acquired = 1;
			break;
		}
		(void)sysThreadYield();
	}
	if (!acquired)
		return 0;
	if (receiver_log.stream == NULL && active_diagnostics != NULL) {
		__atomic_store_n(&receiver_log_lock, 0, __ATOMIC_RELEASE);
		return 1;
	}
	timestamp_utc(timestamp);
	logged = receiver_log_file_event(&receiver_log, timestamp, event, value,
		bytes);
	if (!logged && active_diagnostics != NULL)
		airps3_diag_log(active_diagnostics, sys_time_get_system_time(),
			"LOG_WRITE_FAILED", -1, 0, 0);
	__atomic_store_n(&receiver_log_lock, 0, __ATOMIC_RELEASE);
	return logged;
}

static void runtime_trace(const char *event, int value)
{
	int attempt;
	if (active_diagnostics == NULL)
		return;
	for (attempt = 0; attempt < 1000; ++attempt) {
		if (__atomic_exchange_n(&receiver_log_lock, 1, __ATOMIC_ACQUIRE) == 0) {
			(void)printf("AIRPS3_TRACE %s value=%d\n", event, value);
			(void)fflush(stdout);
			__atomic_store_n(&receiver_log_lock, 0, __ATOMIC_RELEASE);
			return;
		}
		(void)sysThreadYield();
	}
}

static void audio_worker(void *argument)
{
	uint32_t seen_generation = UINT32_MAX;
	uint32_t input_rate = 44100;
	uint32_t input_channels = 2;
	int playback_active = 0;
	int pcm_reported = 0;
	(void)argument;
	raop_pcm_ring_init(&audio_pcm_ring, (uint8_t)input_channels);
	while (!__atomic_load_n(&audio_stop_requested, __ATOMIC_ACQUIRE)) {
		uint32_t current_generation = __atomic_load_n(&pcm_generation,
			__ATOMIC_ACQUIRE);
		if (current_generation != seen_generation) {
			seen_generation = current_generation;
			input_rate = __atomic_load_n(&pcm_input_rate, __ATOMIC_ACQUIRE);
			input_channels = __atomic_load_n(&pcm_input_channels,
				__ATOMIC_ACQUIRE);
			raop_pcm_ring_init(&audio_pcm_ring, (uint8_t)input_channels);
			playback_active = 0;
			(void)log_event("AUDIO_SESSION_RESET", (int)input_rate,
				input_channels);
		}
		while (audio_pcm_ring.count < AUDIO_PREBUFFER_FRAMES &&
			raop_pcm_spsc_count(&decoded_pcm_queue) != 0) {
			size_t popped = raop_pcm_spsc_pop(&decoded_pcm_queue, audio_staging,
				AUDIO_REFILL_FRAMES, seen_generation);
			if (popped == 0)
				break;
			(void)raop_pcm_ring_push_native(&audio_pcm_ring, audio_staging,
				popped);
		}
		if (!playback_active && audio_pcm_ring.count >= AUDIO_PREBUFFER_FRAMES) {
			playback_active = 1;
			(void)log_event("AUDIO_PLAYBACK_STARTED", (int)audio_pcm_ring.count,
				(int)input_rate);
		}
		if (playback_active) {
			(void)raop_pcm_ring_render_float(&audio_pcm_ring, input_rate, 48000,
				audio_output, AUDIO_OUTPUT_FRAMES);
			if (audio_pcm_ring.count == 0 &&
				raop_pcm_spsc_count(&decoded_pcm_queue) == 0) {
				playback_active = 0;
				(void)log_event("AUDIO_BUFFER_UNDERRUN", 1, 0);
				(void)log_event("AUDIO_PCM_DRAINED", 0, 0);
				publish_diagnostic_event(AIRPS3_DIAG_EVENT_AUDIO_DRAINED);
			}
		} else {
			raop_pcm_ring_render_silence(audio_output, AUDIO_OUTPUT_FRAMES);
		}
		{
			int result = cellAudioAdd2chData(audio_port, audio_output,
				AUDIO_OUTPUT_FRAMES, 1.0f);
			if (result != 0) {
				(void)log_event("AUDIO_SUBMIT_FAILED", result, 0);
				publish_diagnostic_event(AIRPS3_DIAG_EVENT_AUDIO_FAILED);
				break;
			}
			if (playback_active && !pcm_reported) {
				pcm_reported = 1;
				(void)log_event("AUDIO_PCM_SUBMITTED", AUDIO_OUTPUT_FRAMES,
					sizeof(audio_output));
				publish_diagnostic_event(
					AIRPS3_DIAG_EVENT_AUDIO_PCM_SUBMITTED);
			}
		}
		(void)sys_timer_usleep(5333U);
	}
	__atomic_store_n(&audio_worker_exited, 1, __ATOMIC_RELEASE);
	runtime_trace("AUDIO_THREAD_EXITING", 0);
	sysThreadExit(0);
	runtime_trace("AUDIO_THREAD_EXIT_RETURNED", 0);
}

static int release_audio(void)
{
	int result;
	if (audio_port_started) {
		result = cellAudioPortStop(audio_port);
		if (result != 0) {
			(void)log_event("AUDIO_PORT_STOP_FAILED", result, 0);
			return 0;
		}
		audio_port_started = 0;
	}
	if (audio_port_opened) {
		result = cellAudioPortClose(audio_port);
		if (result != 0) {
			(void)log_event("AUDIO_PORT_CLOSE_FAILED", result, 0);
			return 0;
		}
		audio_port_opened = 0;
	}
	if (audio_initialized_by_module) {
		result = cellAudioQuit();
		if (result != 0) {
			(void)log_event("AUDIO_QUIT_FAILED", result, 0);
			return 0;
		}
		audio_initialized_by_module = 0;
	}
	if (audio_module_loaded) {
		result = cellSysmoduleUnloadModule(CELL_SYSMODULE_AUDIO);
		if (result != 0) {
			(void)log_event("AUDIO_MODULE_UNLOAD_FAILED", result, 0);
			return 0;
		}
		audio_module_loaded = 0;
	}
	return 1;
}

static int start_audio_backend(void)
{
	CellAudioPortParam parameters = {
		CELL_AUDIO_PORT_2CH, CELL_AUDIO_BLOCK_8, 0, 1.0f
	};
	int result = cellSysmoduleLoadModule(CELL_SYSMODULE_AUDIO);
	if (result != 0) {
		(void)log_event("AUDIO_MODULE_LOAD_FAILED", result, 0);
		publish_diagnostic_event(AIRPS3_DIAG_EVENT_AUDIO_FAILED);
		return 0;
	}
	audio_module_loaded = 1;
	result = cellAudioInit();
	if (result == 0)
		audio_initialized_by_module = 1;
	else if (result != (int)CELL_AUDIO_ERROR_ALREADY_INIT) {
		(void)log_event("AUDIO_INIT_FAILED", result, 0);
		publish_diagnostic_event(AIRPS3_DIAG_EVENT_AUDIO_FAILED);
		(void)release_audio();
		return 0;
	} else {
		publish_diagnostic_event(AIRPS3_DIAG_EVENT_AUDIO_INIT_BORROWED);
	}
	result = cellAudioPortOpen(&parameters, &audio_port);
	if (result != 0) {
		(void)log_event("AUDIO_PORT_OPEN_FAILED", result, 0);
		publish_diagnostic_event(AIRPS3_DIAG_EVENT_AUDIO_FAILED);
		(void)release_audio();
		return 0;
	}
	audio_port_opened = 1;
	result = cellAudioPortStart(audio_port);
	if (result != 0) {
		(void)log_event("AUDIO_PORT_START_FAILED", result, 0);
		publish_diagnostic_event(AIRPS3_DIAG_EVENT_AUDIO_FAILED);
		(void)release_audio();
		return 0;
	}
	audio_port_started = 1;
	raop_pcm_spsc_init(&decoded_pcm_queue);
	__atomic_store_n(&pcm_input_rate, 44100, __ATOMIC_RELAXED);
	__atomic_store_n(&pcm_input_channels, 2, __ATOMIC_RELAXED);
	__atomic_store_n(&pcm_generation, 0, __ATOMIC_RELEASE);
	__atomic_store_n(&audio_stop_requested, 0, __ATOMIC_RELEASE);
	__atomic_store_n(&audio_worker_exited, 0, __ATOMIC_RELEASE);
	if (active_diagnostics != NULL &&
		(active_diagnostics->flags & AIRPS3_DIAG_FLAG_SUPPRESS_TEST_TONE) == 0) {
		uint8_t tone[AUDIO_TEST_TONE_CHUNK_FRAMES * 4U];
		uint32_t offset;
		const float step = 6.2831853071795864769f * 440.0f / 44100.0f;
		float phase = 0.0f;
		uint32_t generation = __atomic_load_n(&pcm_generation,
			__ATOMIC_ACQUIRE);
		for (offset = 0; offset < AUDIO_TEST_TONE_FRAMES;
			offset += AUDIO_TEST_TONE_CHUNK_FRAMES) {
			uint32_t frame;
			for (frame = 0; frame < AUDIO_TEST_TONE_CHUNK_FRAMES; ++frame) {
				int16_t sample = (int16_t)(sinf(phase) * 3932.0f);
				uint16_t bits = (uint16_t)sample;
				tone[frame * 4U] = (uint8_t)bits;
				tone[frame * 4U + 1U] = (uint8_t)(bits >> 8);
				tone[frame * 4U + 2U] = (uint8_t)bits;
				tone[frame * 4U + 3U] = (uint8_t)(bits >> 8);
				phase += step;
				if (phase >= 6.2831853071795864769f)
					phase -= 6.2831853071795864769f;
			}
			if (raop_pcm_spsc_push_le16(&decoded_pcm_queue, tone,
					 sizeof(tone), 2, generation) != AUDIO_TEST_TONE_CHUNK_FRAMES) {
				(void)log_event("AUDIO_TEST_TONE_QUEUE_FAILED", (int)offset, 0);
				publish_diagnostic_event(AIRPS3_DIAG_EVENT_AUDIO_FAILED);
				return 0;
			}
		}
		publish_diagnostic_event(AIRPS3_DIAG_EVENT_TEST_TONE_QUEUED);
		(void)log_event("AUDIO_TEST_TONE_QUEUED", 440,
			AUDIO_TEST_TONE_FRAMES * 4U);
	}
	__atomic_store_n(&audio_thread_started, 1, __ATOMIC_RELEASE);
	result = sysThreadCreate(&audio_thread, audio_worker, NULL, 3000,
		RESIDENT_AUDIO_STACK, THREAD_JOINABLE, "ps3-airplay-audio");
	if (result != 0) {
		__atomic_store_n(&audio_thread_started, 0, __ATOMIC_RELEASE);
		(void)log_event("AUDIO_WORKER_CREATE_FAILED", result, 0);
		publish_diagnostic_event(AIRPS3_DIAG_EVENT_AUDIO_FAILED);
		(void)release_audio();
		return 0;
	}
	(void)log_event("AUDIO_READY", (int)audio_port, 0);
	publish_diagnostic_event(AIRPS3_DIAG_EVENT_AUDIO_READY);
	return 1;
}

static void receiver_worker(void *argument)
{
	uint8_t address[4];
	char instance[64];
	char host[64];
	raop_alac_decoder *decoder = NULL;
	uint64_t decoded_frames = 0;
	int previous_recording = 0;
	int result;
	(void)argument;
	mdns_service.socket_fd = -1;

	if (!raop_receiver_open(&receiver, RESIDENT_RTSP_PORT,
		RESIDENT_RTP_BASE_PORT, &receiver_log)) {
		(void)log_event("RECEIVER_OPEN_FAILED", -1, 0);
		(void)log_event("WORKER_STOPPED", -1, 0);
		publish_diagnostic_event(AIRPS3_DIAG_EVENT_WORKER_STOPPED);
		__atomic_store_n(&worker_exited, 1, __ATOMIC_RELEASE);
		sysThreadExit(0);
	}
	publish_diagnostic_event(AIRPS3_DIAG_EVENT_RECEIVER_READY);
	if (!log_event("RECEIVER_READY", raop_receiver_rtsp_port(&receiver), 0)) {
		raop_receiver_close(&receiver);
		(void)log_event("WORKER_STOPPED", -1, 0);
		publish_diagnostic_event(AIRPS3_DIAG_EVENT_WORKER_STOPPED);
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
			if (decoder != NULL) {
				__atomic_store_n(&pcm_input_rate,
					(uint32_t)receiver.session.offer.fmtp[10], __ATOMIC_RELAXED);
				__atomic_store_n(&pcm_input_channels,
					(uint32_t)receiver.session.offer.fmtp[6], __ATOMIC_RELAXED);
				(void)__atomic_add_fetch(&pcm_generation, 1U, __ATOMIC_RELEASE);
				previous_recording = 1;
			}
		}
		if (receiver.session.state != RAOP_SESSION_RECORDING && decoder != NULL) {
			raop_alac_destroy(decoder);
			decoder = NULL;
			decoded_frames = 0;
			if (previous_recording) {
				(void)__atomic_add_fetch(&pcm_generation, 1U, __ATOMIC_RELEASE);
				previous_recording = 0;
			}
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
						if (__atomic_load_n(&audio_thread_started,
								__ATOMIC_ACQUIRE)) {
							uint32_t generation = __atomic_load_n(&pcm_generation,
								__ATOMIC_ACQUIRE);
							uint8_t channels = (uint8_t)__atomic_load_n(
								&pcm_input_channels, __ATOMIC_ACQUIRE);
							size_t accepted = raop_pcm_spsc_push_le16(
								&decoded_pcm_queue, decoder_pcm, pcm_length,
								channels, generation);
							if (accepted != samples)
								(void)log_event("PCM_QUEUE_DROP",
									(int)(samples - accepted), (int)pcm_length);
						}
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
	publish_diagnostic_event(AIRPS3_DIAG_EVENT_WORKER_STOPPED);
	__atomic_store_n(&worker_exited, 1, __ATOMIC_RELEASE);
	sysThreadExit(0);
}

static int release_network(void)
{
	int result;
	if (netctl_initialized) {
		cellNetCtlTerm();
		netctl_initialized = 0;
	}
	if (netctl_module_loaded) {
		result = cellSysmoduleUnloadModule(CELL_SYSMODULE_NETCTL);
		if (result != 0) {
			(void)log_event("NETCTL_UNLOAD_FAILED", result, 0);
			return 0;
		}
		netctl_module_loaded = 0;
	}
	if (net_initialized) {
		result = sys_net_finalize_network();
		if (result != 0) {
			(void)log_event("NET_FINALIZE_FAILED", result, 0);
			return 0;
		}
		net_initialized = 0;
	}
	if (net_module_loaded) {
		result = cellSysmoduleUnloadModule(CELL_SYSMODULE_NET);
		if (result != 0) {
			(void)log_event("NET_UNLOAD_FAILED", result, 0);
			return 0;
		}
		net_module_loaded = 0;
	}
	return 1;
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
	int log_opened;
	publish_start_diagnostic(args, argp, 1U, 0);

	/* The process CRT initializes heap and constructors; this PRX owns its
	 * lifecycle and does not use the TLS-backed sys_ppu_thread_create shim. */
	__libc_init_array();
	publish_start_diagnostic(args, argp, 2U, 0);
	stop_requested = 0;
	worker_exited = 0;
	worker_started = 0;
	__atomic_store_n(&audio_thread_started, 0, __ATOMIC_RELEASE);
	audio_module_loaded = 0;
	audio_initialized_by_module = 0;
	audio_port_opened = 0;
	audio_port_started = 0;
	net_module_loaded = 0;
	net_initialized = 0;
	netctl_module_loaded = 0;
	netctl_initialized = 0;
	crypto_initialized = 0;
	have_console_mac = 0;

	log_opened = receiver_log_file_open(&receiver_log, RECEIVER_LOG_PATH);
	(void)printf("AIR PS3 resident: log open %s\n",
		log_opened ? "ok" : "failed");
	(void)fflush(stdout);
	if (!log_opened) {
		if (active_diagnostics == NULL) {
			publish_start_diagnostic(args, argp, 11U, -1);
			goto fail_runtime;
		}
		publish_diagnostic_event(AIRPS3_DIAG_EVENT_LOG_UNAVAILABLE);
		airps3_diag_log(active_diagnostics, sys_time_get_system_time(),
			"MODULE_LOG_UNAVAILABLE", -1, 0, 0);
	}
	publish_start_diagnostic(args, argp, 3U, 0);
	if (!log_event("MODULE_START", 0, 0)) {
		(void)printf("AIR PS3 resident: MODULE_START log write failed\n");
		(void)fflush(stdout);
		goto fail;
	}
	publish_start_diagnostic(args, argp, 4U, 0);
	(void)printf("AIR PS3 resident: MODULE_START logged\n");
	(void)fflush(stdout);

	result = cellSysmoduleLoadModule(CELL_SYSMODULE_NET);
	if (result != 0) {
		(void)log_event("NET_MODULE_FAILED", result, 0);
		goto fail;
	}
	net_module_loaded = 1;
	publish_start_diagnostic(args, argp, 5U, 0);
	result = sys_net_initialize_network();
	if (result != 0) {
		(void)log_event("NET_INIT_FAILED", result, 0);
		goto fail;
	}
	net_initialized = 1;
	publish_start_diagnostic(args, argp, 6U, 0);
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
	(void)start_audio_backend();
	publish_start_diagnostic(args, argp, 7U, 0);
	publish_diagnostic_event(AIRPS3_DIAG_EVENT_STARTED);
	return SYS_PRX_RESIDENT;

fail:
	publish_start_diagnostic(args, argp, 9U, result);
	(void)log_event("MODULE_START_FAILED", result, 0);
	(void)printf("AIR PS3 resident: start failed result=%d\n", result);
	(void)fflush(stdout);
	release_crypto();
	if (!release_network()) {
		(void)log_event("MODULE_START_CLEANUP_PENDING", result, 0);
		/* Remain resident so module_stop can retry retained network resources. */
		return SYS_PRX_RESIDENT;
	}
	receiver_log_file_close(&receiver_log);
fail_runtime:
	publish_start_diagnostic(args, argp, 10U, result);
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

	runtime_trace("STOP_BEGIN", 0);
	__atomic_store_n(&stop_requested, 1, __ATOMIC_RELEASE);
	__atomic_store_n(&audio_stop_requested, 1, __ATOMIC_RELEASE);
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
	runtime_trace("STOP_RECEIVER_JOINED", 0);
	if (__atomic_load_n(&audio_thread_started, __ATOMIC_ACQUIRE)) {
		wait_started_us = sys_time_get_system_time();
		wait_polls = 0;
		while (!__atomic_load_n(&audio_worker_exited, __ATOMIC_ACQUIRE) &&
			wait_polls < WORKER_STOP_MAX_POLLS) {
			if (sys_time_get_system_time() - wait_started_us >=
				WORKER_STOP_TIMEOUT_US)
				break;
			++wait_polls;
			(void)sysThreadYield();
		}
		if (!__atomic_load_n(&audio_worker_exited, __ATOMIC_ACQUIRE))
			return SYS_PRX_STOP_FAILED;
		runtime_trace("STOP_AUDIO_EXIT_FLAG", 1);
		join_result = sysThreadJoin(audio_thread, &thread_result);
		runtime_trace("STOP_AUDIO_JOIN_RESULT", join_result);
		if (join_result != 0)
			return SYS_PRX_STOP_FAILED;
		__atomic_store_n(&audio_thread_started, 0, __ATOMIC_RELEASE);
	}
	runtime_trace("STOP_AUDIO_RELEASE_BEGIN", 0);
	if (!release_audio())
		return SYS_PRX_STOP_FAILED;
	runtime_trace("STOP_AUDIO_RELEASE_DONE", 0);
	release_crypto();
	if (!release_network())
		return SYS_PRX_STOP_FAILED;
	(void)log_event("MODULE_STOP", (int)thread_result, 0);
	publish_diagnostic_event(AIRPS3_DIAG_EVENT_MODULE_STOP);
	receiver_log_file_close(&receiver_log);
	active_diagnostics = NULL;
	__libc_fini_array();
	return SYS_PRX_STOP_OK;
}
