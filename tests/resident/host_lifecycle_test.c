#define _POSIX_C_SOURCE 200809L

#include "mock_crypto_rng.h"
#include "../../src/observability/receiver_log.h"
#include "../../src/protocol/raop_receiver.h"
#include "../../src/discovery/raop_mdns_service.h"
#include "../../src/codec/raop_alac.h"
#include "../../src/crypto/raop_crypto.h"
#include <cell/libnetctl.h>
#include <cell/sysmodule.h>
#include <sys/prx.h>
#include <sys/thread.h>
#include <sys/sys_time.h>

#include <assert.h>
#include <arpa/inet.h>
#include <errno.h>
#include <pthread.h>
#include <sched.h>
#include <stdatomic.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

int module_start(size_t args, void *argp);
int module_stop(size_t args, void *argp);

#ifndef RECEIVER_LOG_PATH
#define RECEIVER_LOG_PATH "build/resident-host/receiver.log"
#endif

typedef struct {
	void (*entry)(void *);
	void *arg;
} thread_start;

static int fail_thread_create;
static int fail_thread_join;
static int fail_net_init;
static int fail_netctl_load;
static int fail_netctl_init;
static int fail_receiver_open;
static int fail_netctl_unload;
static int fail_net_finalize;
static int fail_net_unload;
static int netctl_unload_calls;
static int net_finalize_calls;
static int net_unload_calls;
static int net_load_calls;
static int netctl_load_calls;
static int net_init_calls;
static int netctl_init_calls;
static int netctl_term_calls;
static int host_socket_calls;
static int host_connect_calls;
static int host_getsockname_calls;
static int host_close_calls;
static unsigned int mock_poll_delay_ms = 1;
static int mock_poll_error_once;
static volatile int mock_poll_entered;

int resident_host_socket(int domain, int type, int protocol)
{
	assert(domain == AF_INET);
	assert(type == SOCK_DGRAM);
	(void)protocol;
	++host_socket_calls;
	return 42;
}

int resident_host_connect(int fd, const struct sockaddr *address,
	socklen_t length)
{
	const struct sockaddr_in *target = (const struct sockaddr_in *)address;
	assert(fd == 42);
	assert(length >= sizeof(*target));
	assert(target->sin_family == AF_INET);
	assert(ntohl(target->sin_addr.s_addr) == 0xe00000fbU);
	assert(ntohs(target->sin_port) == 5353);
	++host_connect_calls;
	return 0;
}

int resident_host_getsockname(int fd, struct sockaddr *address,
	socklen_t *length)
{
	struct sockaddr_in local;
	assert(fd == 42);
	assert(address != NULL && length != NULL && *length >= sizeof(local));
	memset(&local, 0, sizeof(local));
	local.sin_family = AF_INET;
	local.sin_addr.s_addr = htonl(0x7f000001U);
	memcpy(address, &local, sizeof(local));
	*length = sizeof(local);
	++host_getsockname_calls;
	return 0;
}

int resident_host_close(int fd)
{
	assert(fd == 42);
	++host_close_calls;
	return 0;
}

static void *thread_trampoline(void *opaque)
{
	thread_start start = *(thread_start *)opaque;
	free(opaque);
	start.entry(start.arg);
	return NULL;
}

int sysThreadCreate(sys_ppu_thread_t *thread, void (*entry)(void *), void *arg,
	int priority, size_t stack_size, uint64_t flags, const char *name)
{
	thread_start *start;
	(void)priority;
	(void)stack_size;
	(void)flags;
	(void)name;
	if (fail_thread_create) {
		fail_thread_create = 0;
		return -7001;
	}
	start = malloc(sizeof(*start));
	if (start == NULL)
		return -ENOMEM;
	start->entry = entry;
	start->arg = arg;
	if (pthread_create(thread, NULL, thread_trampoline, start) != 0) {
		free(start);
		return -EAGAIN;
	}
	return 0;
}

int sysThreadJoin(sys_ppu_thread_t thread, uint64_t *result)
{
	void *thread_result = NULL;
	if (fail_thread_join) {
		--fail_thread_join;
		return -7007;
	}
	int status = pthread_join(thread, &thread_result);
	if (status == 0 && result != NULL)
		*result = (uint64_t)(uintptr_t)thread_result;
	return status;
}

int sysThreadYield(void)
{
	return sched_yield();
}

void sysThreadExit(uint64_t result)
{
	pthread_exit((void *)(uintptr_t)result);
}

uint64_t sys_time_get_system_time(void)
{
	struct timespec now;
	assert(clock_gettime(CLOCK_MONOTONIC, &now) == 0);
	return (uint64_t)now.tv_sec * 1000000U +
		(uint64_t)now.tv_nsec / 1000U;
}

int cellSysmoduleLoadModule(int module)
{
	if (module == CELL_SYSMODULE_NET)
		++net_load_calls;
	else if (module == CELL_SYSMODULE_NETCTL) {
		++netctl_load_calls;
		if (fail_netctl_load) {
			--fail_netctl_load;
			return -7009;
		}
	} else
		return -EINVAL;
	return 0;
}

int cellSysmoduleUnloadModule(int module)
{
	if (module == CELL_SYSMODULE_NETCTL) {
		++netctl_unload_calls;
		if (fail_netctl_unload) {
			--fail_netctl_unload;
			return -7002;
		}
		return 0;
	}
	if (module == CELL_SYSMODULE_NET) {
		++net_unload_calls;
		if (fail_net_unload) {
			--fail_net_unload;
			return -7003;
		}
		return 0;
	}
	return -EINVAL;
}

int sys_net_initialize_network(void)
{
	++net_init_calls;
	if (fail_net_init) {
		--fail_net_init;
		return -7005;
	}
	return 0;
}

int sys_net_finalize_network(void)
{
	++net_finalize_calls;
	if (fail_net_finalize) {
		--fail_net_finalize;
		return -7004;
	}
	return 0;
}

int cellNetCtlInit(void)
{
	++netctl_init_calls;
	if (fail_netctl_init) {
		--fail_netctl_init;
		return -7008;
	}
	return 0;
}

void cellNetCtlTerm(void)
{
	++netctl_term_calls;
}

int cellNetCtlGetInfo(int code, union CellNetCtlInfo *info)
{
	if (code != CELL_NET_CTL_INFO_ETHER_ADDR || info == NULL)
		return -EINVAL;
	memset(info->ether_addr.data, 0x5a, sizeof(info->ether_addr.data));
	return 0;
}

int raop_crypto_context_init(raop_crypto_context *context,
	mbedtls_f_rng_t *rng, void *rng_context)
{
	(void)rng;
	(void)rng_context;
	context->initialized = 1;
	return 1;
}

void raop_crypto_context_free(raop_crypto_context *context)
{
	context->initialized = 0;
}

int raop_crypto_ps3_rng(void *context, unsigned char *output, size_t length)
{
	(void)context;
	memset(output, 0, length);
	return 0;
}

int raop_receiver_open(raop_receiver *receiver, uint16_t rtsp_port,
	uint16_t rtp_base_port, receiver_log_file *log)
{
	(void)rtp_base_port;
	memset(receiver, 0, sizeof(*receiver));
	if (fail_receiver_open) {
		--fail_receiver_open;
		return 0;
	}
	receiver->tcp_fd = 1;
	receiver->audio_fd = 2;
	receiver->control_fd = 3;
	receiver->timing_fd = 4;
	receiver->rtsp_port = rtsp_port;
	receiver->log = log;
	return 1;
}

int raop_receiver_configure_crypto(raop_receiver *receiver,
	raop_crypto_context *crypto, mbedtls_f_rng_t *rng, void *rng_context,
	const uint8_t mac[6])
{
	(void)rng;
	(void)rng_context;
	(void)mac;
	if (receiver == NULL || crypto == NULL || !crypto->initialized)
		return 0;
	receiver->crypto = crypto;
	receiver->crypto_configured = 1;
	return 1;
}

int raop_receiver_poll(raop_receiver *receiver, unsigned int timeout_ms)
{
	struct timespec delay;
	(void)receiver;
	(void)timeout_ms;
	__atomic_store_n(&mock_poll_entered, 1, __ATOMIC_RELEASE);
	if (mock_poll_error_once) {
		--mock_poll_error_once;
		return -7006;
	}
	delay.tv_sec = mock_poll_delay_ms / 1000U;
	delay.tv_nsec = (long)(mock_poll_delay_ms % 1000U) * 1000000L;
	(void)nanosleep(&delay, NULL);
	return 0;
}

uint16_t raop_receiver_rtsp_port(const raop_receiver *receiver)
{
	return receiver->rtsp_port;
}

int raop_receiver_pop_audio(raop_receiver *receiver, raop_rtp_frame *frame)
{
	(void)receiver;
	(void)frame;
	return 0;
}

void raop_receiver_close(raop_receiver *receiver)
{
	receiver->tcp_fd = receiver->audio_fd = receiver->control_fd =
		receiver->timing_fd = -1;
}

int raop_mdns_service_open(raop_mdns_service *service,
	const uint8_t ipv4[4], const char *instance_label,
	const char *host_label, uint16_t port, uint64_t now_ms)
{
	(void)ipv4;
	(void)instance_label;
	(void)host_label;
	(void)now_ms;
	service->socket_fd = -1;
	service->port = port;
	return 1;
}

int raop_mdns_service_poll(raop_mdns_service *service, uint64_t now_ms)
{
	(void)service;
	(void)now_ms;
	return 0;
}

void raop_mdns_service_close(raop_mdns_service *service)
{
	service->socket_fd = -1;
}

raop_alac_decoder *raop_alac_create(const raop_sdp_offer *offer)
{
	(void)offer;
	return NULL;
}

int raop_alac_decode(raop_alac_decoder *decoder, const uint8_t *packet,
	size_t packet_length, uint8_t *pcm, size_t pcm_capacity,
	size_t *pcm_length, uint32_t *sample_count)
{
	(void)decoder;
	(void)packet;
	(void)packet_length;
	(void)pcm;
	(void)pcm_capacity;
	(void)pcm_length;
	(void)sample_count;
	return 0;
}

void raop_alac_destroy(raop_alac_decoder *decoder)
{
	(void)decoder;
}

void mbedtls_platform_zeroize(void *buffer, size_t length)
{
	volatile unsigned char *bytes = buffer;
	while (length-- != 0)
		*bytes++ = 0;
}

int receiver_log_file_open(receiver_log_file *log, const char *path)
{
	memset(log, 0, sizeof(*log));
	if (snprintf(log->path, sizeof(log->path), "%s", path) >=
		(int)sizeof(log->path))
		return 0;
	log->stream = fopen(path, "wb");
	return log->stream != NULL;
}

void receiver_log_file_close(receiver_log_file *log)
{
	if (log->stream != NULL)
		(void)fclose(log->stream);
	log->stream = NULL;
}

int receiver_log_file_event(receiver_log_file *log, const char *timestamp,
	const char *event, int value, size_t byte_count)
{
	int result;
	if (log->stream == NULL)
		return 0;
	result = fprintf(log->stream, "%s event=%s value=%d bytes=%zu\n",
		timestamp, event, value, byte_count);
	return result > 0 && fflush(log->stream) == 0;
}

void __libc_init_array(void)
{
}

void __libc_fini_array(void)
{
}

static void reset_mocks(void)
{
	fail_thread_create = 0;
	fail_thread_join = 0;
	fail_net_init = 0;
	fail_netctl_load = 0;
	fail_netctl_init = 0;
	fail_receiver_open = 0;
	fail_netctl_unload = 0;
	fail_net_finalize = 0;
	fail_net_unload = 0;
	netctl_unload_calls = 0;
	net_finalize_calls = 0;
	net_unload_calls = 0;
	net_load_calls = 0;
	netctl_load_calls = 0;
	net_init_calls = 0;
	netctl_init_calls = 0;
	netctl_term_calls = 0;
	host_socket_calls = 0;
	host_connect_calls = 0;
	host_getsockname_calls = 0;
	host_close_calls = 0;
	mock_poll_delay_ms = 1;
	mock_poll_error_once = 0;
	__atomic_store_n(&mock_poll_entered, 0, __ATOMIC_RELEASE);
}

static void wait_for_worker(void)
{
	struct timespec delay = {0, 25000000L};
	(void)nanosleep(&delay, NULL);
}

static void wait_for_poll_entry(void)
{
	struct timespec delay = {0, 1000000L};
	uint64_t deadline = sys_time_get_system_time() + 2000000U;
	while (!__atomic_load_n(&mock_poll_entered, __ATOMIC_ACQUIRE)) {
		assert(sys_time_get_system_time() < deadline);
		(void)nanosleep(&delay, NULL);
	}
}

static void assert_log_event(const char *wanted)
{
	char line[256];
	int found = 0;
	FILE *log = fopen(RECEIVER_LOG_PATH, "rb");
	assert(log != NULL);
	while (fgets(line, sizeof(line), log) != NULL) {
		if (strstr(line, wanted) != NULL) {
			found = 1;
			break;
		}
	}
	(void)fclose(log);
	assert(found);
}

static void wait_for_log_event(const char *wanted, unsigned int timeout_ms)
{
	struct timespec delay = {0, 20000000L};
	uint64_t deadline = sys_time_get_system_time() +
		(uint64_t)timeout_ms * 1000U;
	for (;;) {
		char line[256];
		FILE *log = fopen(RECEIVER_LOG_PATH, "rb");
		assert(log != NULL);
		while (fgets(line, sizeof(line), log) != NULL) {
			if (strstr(line, wanted) != NULL) {
				(void)fclose(log);
				return;
			}
		}
		(void)fclose(log);
		assert(sys_time_get_system_time() < deadline);
		(void)nanosleep(&delay, NULL);
	}
}

static void test_startup_rollback_on_thread_failure(void)
{
	reset_mocks();
	fail_thread_create = 1;
	assert(module_start(0, NULL) == SYS_PRX_NO_RESIDENT);
	assert(net_load_calls == 1 && net_init_calls == 1);
	assert(netctl_load_calls == 1 && netctl_init_calls == 1);
	assert(netctl_term_calls == 1 && netctl_unload_calls == 1);
	assert(net_finalize_calls == 1 && net_unload_calls == 1);
	assert(host_socket_calls == 0 && host_connect_calls == 0);
	puts("startup rollback after worker-create failure: PASS");
}

static void test_startup_rollback_on_network_init_failure(void)
{
	reset_mocks();
	fail_net_init = 1;
	assert(module_start(0, NULL) == SYS_PRX_NO_RESIDENT);
	assert(net_load_calls == 1 && net_init_calls == 1);
	assert(netctl_load_calls == 0 && netctl_unload_calls == 0);
	assert(net_finalize_calls == 0 && net_unload_calls == 1);
	assert(host_socket_calls == 0 && host_connect_calls == 0);
	puts("startup rollback after network-init failure: PASS");
}

static void test_netctl_init_failure_cleanup(void)
{
	reset_mocks();
	fail_netctl_init = 1;
	assert(module_start(0, NULL) == SYS_PRX_RESIDENT);
	wait_for_log_event("event=NETCTL_INIT_FAILED", 2000);
	wait_for_log_event("event=RECEIVER_READY", 2000);
	assert(module_stop(0, NULL) == SYS_PRX_STOP_OK);
	assert(netctl_init_calls == 1 && netctl_term_calls == 0);
	assert(netctl_unload_calls == 1);
	assert(net_finalize_calls == 1 && net_unload_calls == 1);
	assert_log_event("event=WORKER_STOPPED");
	assert_log_event("event=MODULE_STOP");
	puts("NetCtl-init failure with receiver cleanup: PASS");
}

static void test_netctl_load_failure_cleanup(void)
{
	reset_mocks();
	fail_netctl_load = 1;
	assert(module_start(0, NULL) == SYS_PRX_RESIDENT);
	wait_for_log_event("event=NETCTL_LOAD_FAILED", 2000);
	wait_for_log_event("event=RECEIVER_READY", 2000);
	assert(module_stop(0, NULL) == SYS_PRX_STOP_OK);
	assert(netctl_load_calls == 1 && netctl_init_calls == 0);
	assert(netctl_term_calls == 0 && netctl_unload_calls == 0);
	assert(net_finalize_calls == 1 && net_unload_calls == 1);
	assert_log_event("event=WORKER_STOPPED");
	assert_log_event("event=MODULE_STOP");
	puts("NetCtl-load failure with receiver cleanup: PASS");
}

static void test_receiver_open_failure_cleanup(void)
{
	reset_mocks();
	fail_receiver_open = 1;
	assert(module_start(0, NULL) == SYS_PRX_RESIDENT);
	wait_for_log_event("event=RECEIVER_OPEN_FAILED", 2000);
	wait_for_log_event("event=WORKER_STOPPED", 2000);
	assert(module_stop(0, NULL) == SYS_PRX_STOP_OK);
	assert(netctl_term_calls == 1 && netctl_unload_calls == 1);
	assert(net_finalize_calls == 1 && net_unload_calls == 1);
	assert(host_socket_calls == 0 && host_connect_calls == 0);
	assert_log_event("event=MODULE_STOP");
	puts("receiver-open failure and exited-worker cleanup: PASS");
}

static void test_netctl_unload_retry(void)
{
	reset_mocks();
	assert(module_start(0, NULL) == SYS_PRX_RESIDENT);
	wait_for_worker();
	fail_netctl_unload = 1;
	assert(module_stop(0, NULL) == SYS_PRX_STOP_FAILED);
	assert(netctl_unload_calls == 1);
	assert(net_finalize_calls == 0 && net_unload_calls == 0);
	assert(module_stop(0, NULL) == SYS_PRX_STOP_OK);
	assert(netctl_unload_calls == 2);
	assert(net_finalize_calls == 1 && net_unload_calls == 1);
	assert_log_event("event=RECEIVER_READY");
	assert_log_event("event=WORKER_STOPPED");
	assert_log_event("event=MODULE_STOP");
	assert(host_socket_calls == 1 && host_connect_calls == 1);
	assert(host_getsockname_calls == 1 && host_close_calls == 1);
	puts("worker stop and NetCtl unload retry: PASS");
}

static void test_net_finalize_retry(void)
{
	reset_mocks();
	assert(module_start(0, NULL) == SYS_PRX_RESIDENT);
	wait_for_worker();
	fail_net_finalize = 1;
	assert(module_stop(0, NULL) == SYS_PRX_STOP_FAILED);
	assert(netctl_unload_calls == 1);
	assert(net_finalize_calls == 1 && net_unload_calls == 0);
	assert(module_stop(0, NULL) == SYS_PRX_STOP_OK);
	assert(netctl_unload_calls == 1);
	assert(net_finalize_calls == 2 && net_unload_calls == 1);
	assert_log_event("event=WORKER_STOPPED");
	assert_log_event("event=MODULE_STOP");
	puts("network finalize retry after worker exit: PASS");
}

static void test_net_unload_retry(void)
{
	reset_mocks();
	assert(module_start(0, NULL) == SYS_PRX_RESIDENT);
	wait_for_worker();
	fail_net_unload = 1;
	assert(module_stop(0, NULL) == SYS_PRX_STOP_FAILED);
	assert(netctl_unload_calls == 1);
	assert(net_finalize_calls == 1 && net_unload_calls == 1);
	assert(module_stop(0, NULL) == SYS_PRX_STOP_OK);
	assert(netctl_unload_calls == 1);
	assert(net_finalize_calls == 1 && net_unload_calls == 2);
	assert_log_event("event=WORKER_STOPPED");
	assert_log_event("event=MODULE_STOP");
	puts("network module unload retry after worker exit: PASS");
}

static void test_bounded_worker_stop_retry(void)
{
	uint64_t stop_started_us;
	uint64_t stop_elapsed_us;
	reset_mocks();
	mock_poll_delay_ms = 5500;
	assert(module_start(0, NULL) == SYS_PRX_RESIDENT);
	wait_for_poll_entry();
	stop_started_us = sys_time_get_system_time();
	assert(module_stop(0, NULL) == SYS_PRX_STOP_FAILED);
	stop_elapsed_us = sys_time_get_system_time() - stop_started_us;
	assert(stop_elapsed_us <= 5500000U);
	wait_for_log_event("event=WORKER_STOPPED", 6000);
	assert(module_stop(0, NULL) == SYS_PRX_STOP_OK);
	assert_log_event("event=MODULE_STOP");
	puts("bounded worker-stop timeout and retry: PASS");
}

static void test_worker_poll_error_cleanup(void)
{
	reset_mocks();
	mock_poll_error_once = 1;
	assert(module_start(0, NULL) == SYS_PRX_RESIDENT);
	wait_for_log_event("event=RECEIVER_ERROR", 2000);
	wait_for_log_event("event=WORKER_STOPPED", 2000);
	fail_thread_join = 1;
	assert(module_stop(0, NULL) == SYS_PRX_STOP_FAILED);
	assert(netctl_unload_calls == 0);
	assert(net_finalize_calls == 0 && net_unload_calls == 0);
	assert(module_stop(0, NULL) == SYS_PRX_STOP_OK);
	assert(netctl_unload_calls == 1);
	assert(net_finalize_calls == 1 && net_unload_calls == 1);
	assert_log_event("event=WORKER_STOPPED");
	assert_log_event("event=MODULE_STOP");
	puts("worker poll-error cleanup and retry after join failure: PASS");
}

int main(void)
{
	(void)setvbuf(stdout, NULL, _IONBF, 0);
	test_startup_rollback_on_thread_failure();
	test_startup_rollback_on_network_init_failure();
	test_netctl_load_failure_cleanup();
	test_netctl_init_failure_cleanup();
	test_receiver_open_failure_cleanup();
	test_netctl_unload_retry();
	test_net_finalize_retry();
	test_net_unload_retry();
	test_bounded_worker_stop_retry();
	test_worker_poll_error_cleanup();
	puts("resident host lifecycle simulation: ALL PASS (mocked platform APIs)");
	return 0;
}
