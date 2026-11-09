#include <sys/prx.h>
#include <sys/sys_time.h>
#include <cell/audio.h>

#include <math.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include "resident_diagnostics.h"

#define APP_LOG_PATH "/dev_hdd0/tmp/ps3-airplay-resident-harness.log"
#define MODULE_PATH "/dev_hdd0/game/RPRX90001/USRDIR/ps3-airplay-receiver.fake.sprx"
#ifndef TEST_CYCLES
#define TEST_CYCLES 50U
#endif
#define EVENT_WAIT_POLLS 400U
#define EVENT_POLL_US 25000U
#define MODULE_LIST_CAPACITY 256U
#define AUDIO_PROBE_BLOCKS 940U
#define AUDIO_PROBE_SHORT_BLOCKS 94U
#define AUDIO_PROBE_AMPLITUDE 0.12f
#define AUDIO_PROBE_FREQUENCY_HZ 660.0f

static FILE *app_log;
static uint32_t foreground_audio_port;
static float foreground_audio_phase;
static int foreground_audio_initialized;
static int foreground_audio_opened;
static int foreground_audio_started;
static uint64_t harness_start_us;

static void event(const char *name, int value)
{
	uint64_t uptime_ms = (sys_time_get_system_time() - harness_start_us) / 1000U;
	if (app_log != NULL) {
		(void)fprintf(app_log, "uptime_ms=%llu event=%s value=%d\n",
			(unsigned long long)uptime_ms, name, value);
		(void)fflush(app_log);
	}
	(void)printf("uptime_ms=%llu event=%s value=%d\n",
		(unsigned long long)uptime_ms, name, value);
}

static void drain_resident_log(const airps3_resident_diagnostics *diagnostics,
	uint32_t *read_index)
{
	uint32_t write_index = __atomic_load_n(&diagnostics->log_write_index,
		__ATOMIC_ACQUIRE);
	if (write_index > AIRPS3_DIAG_LOG_CAPACITY)
		write_index = AIRPS3_DIAG_LOG_CAPACITY;
	while (*read_index < write_index) {
		const airps3_resident_log_record *record =
			&diagnostics->log[*read_index];
		uint32_t expected = *read_index + 1U;
		if (__atomic_load_n(&record->committed_sequence,
			__ATOMIC_ACQUIRE) != expected)
			break;
		if (app_log != NULL) {
			(void)fprintf(app_log,
				"resident_seq=%u resident_us=%llu event=%s result=%d value=%u bytes=%u\n",
				expected, (unsigned long long)record->monotonic_us,
				record->event, record->result, record->value,
				record->byte_count);
			(void)fflush(app_log);
		}
		(void)printf(
			"resident_seq=%u resident_us=%llu event=%s result=%d value=%u bytes=%u\n",
			expected, (unsigned long long)record->monotonic_us,
			record->event, record->result, record->value,
			record->byte_count);
		++*read_index;
	}
}

static int wait_for_diagnostic_event(
	const airps3_resident_diagnostics *diagnostics, uint32_t event_mask,
	uint32_t *log_read_index)
{
	unsigned int poll;
	for (poll = 0; poll < EVENT_WAIT_POLLS; ++poll) {
		drain_resident_log(diagnostics, log_read_index);
		if ((__atomic_load_n(&diagnostics->events, __ATOMIC_ACQUIRE) &
			event_mask) != 0)
			return 1;
		(void)usleep(EVENT_POLL_US);
	}
	return 0;
}

static int wait_for_audio_pcm(const airps3_resident_diagnostics *diagnostics,
	uint32_t *log_read_index)
{
	unsigned int poll;
	for (poll = 0; poll < EVENT_WAIT_POLLS; ++poll) {
		drain_resident_log(diagnostics, log_read_index);
		uint32_t events = __atomic_load_n(&diagnostics->events,
			__ATOMIC_ACQUIRE);
		if ((events & AIRPS3_DIAG_EVENT_AUDIO_PCM_SUBMITTED) != 0U)
			return 1;
		if ((events & AIRPS3_DIAG_EVENT_AUDIO_FAILED) != 0U)
			return 0;
		(void)usleep(EVENT_POLL_US);
	}
	return 0;
}

static int get_module_count(unsigned int *count)
{
	sys_prx_id_t ids[MODULE_LIST_CAPACITY];
	sys_prx_get_module_list_t modules;
	int result;
	memset(&modules, 0, sizeof(modules));
	modules.size = sizeof(modules);
	modules.max = MODULE_LIST_CAPACITY;
	modules.idlist = (sysPrxUserPprxId)(uintptr_t)ids;
	result = sys_prx_get_module_list(0, &modules);
	if (result != 0)
		return result;
	if (modules.count >= MODULE_LIST_CAPACITY)
		return -1;
	*count = modules.count;
	return 0;
}

static int start_audio_probe(void)
{
	CellAudioPortParam params = {
		CELL_AUDIO_PORT_2CH, CELL_AUDIO_BLOCK_8, 0, 1.0f
	};
	int result = cellAudioInit();
	event("AUDIO_INIT_RESULT", result);
	if (result != 0)
		return 0;
	foreground_audio_initialized = 1;
	result = cellAudioPortOpen(&params, &foreground_audio_port);
	event("AUDIO_PORT_OPEN_RESULT", result);
	if (result != 0)
		return 0;
	foreground_audio_opened = 1;
	result = cellAudioPortStart(foreground_audio_port);
	event("AUDIO_PORT_START_RESULT", result);
	if (result != 0)
		return 0;
	foreground_audio_started = 1;
	return 1;
}

static int submit_foreground_audio_probe(unsigned int blocks)
{
	float samples[CELL_AUDIO_BLOCK_SAMPLES * 2U];
	const float phase_step = 6.2831853071795864769f *
		AUDIO_PROBE_FREQUENCY_HZ / 48000.0f;
	unsigned int block;
	unsigned int frame;
	for (block = 0; block < blocks; ++block) {
		for (frame = 0; frame < CELL_AUDIO_BLOCK_SAMPLES; ++frame) {
			float sample = sinf(foreground_audio_phase) * AUDIO_PROBE_AMPLITUDE;
			samples[frame * 2U] = sample;
			samples[frame * 2U + 1U] = sample;
			foreground_audio_phase += phase_step;
			if (foreground_audio_phase >= 6.2831853071795864769f)
				foreground_audio_phase -= 6.2831853071795864769f;
		}
		int result = cellAudioAdd2chData(foreground_audio_port, samples,
			CELL_AUDIO_BLOCK_SAMPLES, 1.0f);
		if (result != 0) {
			event("AUDIO_SUBMIT_FAILED", result);
			return 0;
		}
		(void)usleep(5333U);
	}
	event("AUDIO_BLOCKS_SUBMITTED", (int)block);
	return 1;
}

static void stop_audio_probe(void)
{
	int result;
	if (foreground_audio_started) {
		result = cellAudioPortStop(foreground_audio_port);
		event("AUDIO_PORT_STOP_RESULT", result);
	}
	if (foreground_audio_opened) {
		result = cellAudioPortClose(foreground_audio_port);
		event("AUDIO_PORT_CLOSE_RESULT", result);
	}
	if (foreground_audio_initialized) {
		result = cellAudioQuit();
		event("AUDIO_QUIT_RESULT", result);
	}
}

static int stop_and_unload(sys_prx_id_t module_id,
	const airps3_resident_diagnostics *diagnostics,
	unsigned int baseline_module_count, unsigned int cycle,
	uint32_t *log_read_index)
{
	unsigned int module_count;
	int module_result = 0;
	int result;
	int events_ok = 1;
	result = sys_prx_stop_module(module_id, 0, NULL, &module_result, 0, NULL);
	event("STOP_RESULT", result);
	drain_resident_log(diagnostics, log_read_index);
	if (result != 0)
		return 0;
	if (!wait_for_diagnostic_event(diagnostics,
		AIRPS3_DIAG_EVENT_WORKER_STOPPED, log_read_index)) {
		event("WORKER_STOP_EVENT_MISSING", (int)cycle);
		events_ok = 0;
	} else {
		event("WORKER_STOPPED", (int)cycle);
	}
	if (!wait_for_diagnostic_event(diagnostics,
		AIRPS3_DIAG_EVENT_MODULE_STOP, log_read_index)) {
		event("MODULE_STOP_EVENT_MISSING", (int)cycle);
		events_ok = 0;
	}
	result = sys_prx_unload_module(module_id, 0, NULL);
	event("UNLOAD_RESULT", result);
	if (result != 0)
		return 0;
	drain_resident_log(diagnostics, log_read_index);
	event("RESIDENT_LOG_DROPPED", (int)__atomic_load_n(
		&diagnostics->log_dropped, __ATOMIC_ACQUIRE));
	result = get_module_count(&module_count);
	if (result != 0) {
		event("MODULE_LIST_FAILED_AFTER_CYCLE", result);
		return 0;
	}
	event("MODULE_COUNT_AFTER_CYCLE", (int)module_count);
	if (module_count != baseline_module_count) {
		event("MODULE_COUNT_LEAK", (int)module_count);
		return 0;
	}
	if (!events_ok)
		return 0;
	return 1;
}

static int run_cycle(unsigned int cycle, unsigned int baseline_module_count)
{
	sys_prx_id_t module_id;
	unsigned int module_count;
	int module_result = 0;
	int result;
	uint32_t diagnostic_events;
	uint32_t log_read_index = 0;
	unsigned int mode = cycle % 4U;
	airps3_resident_diagnostics diagnostics;
	char name[32];
	memset(&diagnostics, 0, sizeof(diagnostics));
	diagnostics.magic = AIRPS3_RESIDENT_DIAGNOSTICS_MAGIC;
	diagnostics.flags = mode == 0U ? AIRPS3_DIAG_FLAG_SUPPRESS_TEST_TONE : 0U;
	(void)snprintf(name, sizeof(name), "CYCLE_%u_BEGIN", cycle);
	event(name, (int)cycle);
	if (mode == 0U)
		event("CYCLE_MODE_NO_RAOP_PCM", (int)cycle);
	else if (mode == 1U)
		event("CYCLE_MODE_STOP_DURING_PCM", (int)cycle);
	else if (mode == 2U)
		event("CYCLE_MODE_STOP_AFTER_PCM_DRAIN", (int)cycle);
	else
		event("CYCLE_MODE_DUAL_TONE_SUBMISSION", (int)cycle);

	module_id = sys_prx_load_module(MODULE_PATH, 0, NULL);
	if (module_id < 0) {
		event("LOAD_FAILED", module_id);
		return 0;
	}
	event("LOAD_OK", module_id);
	result = sys_prx_start_module(module_id, sizeof(diagnostics), &diagnostics,
		&module_result, 0, NULL);
	if (result != 0) {
		event("START_FAILED", result);
		result = sys_prx_unload_module(module_id, 0, NULL);
		event("UNLOAD_AFTER_START_FAILURE", result);
		return 0;
	}
	event("START_OK", module_result);
	drain_resident_log(&diagnostics, &log_read_index);
	event("MODULE_START_STAGE", (int)diagnostics.stage);
	event("MODULE_START_ERROR", diagnostics.error);
	if (module_result != SYS_PRX_RESIDENT) {
		event("START_NOT_RESIDENT", module_result);
		/* The loader removes a module that returns NO_RESIDENT from its
		 * module_start callback. Do not issue a second unload against that ID. */
		if (module_result == SYS_PRX_NO_RESIDENT) {
			result = get_module_count(&module_count);
			event("MODULE_LIST_AFTER_NONRESIDENT_START", result);
			if (result == 0) {
				event("MODULE_COUNT_AFTER_NONRESIDENT_START",
					(int)module_count);
				if (module_count == baseline_module_count)
					event("START_ROLLBACK_OK", (int)cycle);
				else
					event("MODULE_COUNT_LEAK_AFTER_START_FAILURE",
						(int)module_count);
			}
		} else {
			result = sys_prx_unload_module(module_id, 0, NULL);
			event("UNLOAD_AFTER_UNEXPECTED_START_RESULT", result);
		}
		return 0;
	}
	diagnostic_events = __atomic_load_n(&diagnostics.events, __ATOMIC_ACQUIRE);
	event("MODULE_DIAGNOSTIC_EVENTS", (int)diagnostic_events);
	if ((diagnostic_events & AIRPS3_DIAG_EVENT_LOG_UNAVAILABLE) != 0U)
		event("MODULE_LOG_UNAVAILABLE", (int)cycle);
	if ((diagnostic_events & AIRPS3_DIAG_EVENT_AUDIO_READY) != 0U)
		event("RESIDENT_AUDIO_READY", (int)cycle);
	else if ((diagnostic_events & AIRPS3_DIAG_EVENT_AUDIO_FAILED) != 0U)
		event("RESIDENT_AUDIO_FAILED", (int)cycle);
	else
		event("RESIDENT_AUDIO_STATUS_PENDING", (int)cycle);
	if ((diagnostic_events & AIRPS3_DIAG_EVENT_AUDIO_INIT_BORROWED) != 0U)
		event("RESIDENT_AUDIO_REUSED_SYSTEM_INIT", (int)cycle);
	if ((diagnostic_events & AIRPS3_DIAG_EVENT_TEST_TONE_QUEUED) != 0U)
		event("RESIDENT_TEST_TONE_QUEUED", 440);
	if ((diagnostic_events & AIRPS3_DIAG_EVENT_STARTED) == 0U) {
		event("MODULE_START_EVENT_MISSING", (int)cycle);
		(void)stop_and_unload(module_id, &diagnostics,
			baseline_module_count, cycle, &log_read_index);
		return 0;
	}
	if (!wait_for_diagnostic_event(&diagnostics,
		AIRPS3_DIAG_EVENT_RECEIVER_READY, &log_read_index)) {
		event("RECEIVER_READY_TIMEOUT", (int)cycle);
		(void)stop_and_unload(module_id, &diagnostics,
			baseline_module_count, cycle, &log_read_index);
		return 0;
	}
	event("RECEIVER_READY", (int)cycle);
	if (mode != 0U) {
		if (!wait_for_audio_pcm(&diagnostics, &log_read_index)) {
			event("RESIDENT_PCM_SUBMISSION_MISSING", (int)cycle);
			(void)stop_and_unload(module_id, &diagnostics,
				baseline_module_count, cycle, &log_read_index);
			return 0;
		}
		event("RESIDENT_PCM_SUBMITTED_TO_MIXER", (int)cycle);
	}
	if (mode == 2U) {
		if (!wait_for_diagnostic_event(&diagnostics,
			AIRPS3_DIAG_EVENT_AUDIO_DRAINED, &log_read_index)) {
			event("RESIDENT_PCM_DRAIN_TIMEOUT", (int)cycle);
			(void)stop_and_unload(module_id, &diagnostics,
				baseline_module_count, cycle, &log_read_index);
			return 0;
		}
		event("RESIDENT_PCM_DRAINED", (int)cycle);
	} else if (mode == 3U) {
		event("TWO_ACTIVE_AUDIO_PORTS", (int)cycle);
		event("FOREGROUND_TONE_HZ", (int)AUDIO_PROBE_FREQUENCY_HZ);
		if (!submit_foreground_audio_probe(AUDIO_PROBE_BLOCKS)) {
			(void)stop_and_unload(module_id, &diagnostics,
				baseline_module_count, cycle, &log_read_index);
			return 0;
		}
	} else if (mode == 0U) {
		event("FOREGROUND_TONE_HZ", (int)AUDIO_PROBE_FREQUENCY_HZ);
		if (!submit_foreground_audio_probe(AUDIO_PROBE_SHORT_BLOCKS)) {
			(void)stop_and_unload(module_id, &diagnostics,
				baseline_module_count, cycle, &log_read_index);
			return 0;
		}
	}

	if (!stop_and_unload(module_id, &diagnostics,
		baseline_module_count, cycle, &log_read_index))
		return 0;
	event("CYCLE_PASS", (int)cycle);
	return 1;
}

int main(int argc, const char **argv)
{
	unsigned int cycle;
	unsigned int baseline_module_count;
	int result;
	(void)argc;
	(void)argv;
	harness_start_us = sys_time_get_system_time();
	app_log = fopen(APP_LOG_PATH, "wb");
	event("HARNESS_RUN_ID", (int)(harness_start_us & 0x7fffffffU));
	event(app_log != NULL ? "HARNESS_START" : "HARNESS_LOG_OPEN_FAILED", 0);
	event("HARNESS_CYCLE_COUNT", (int)TEST_CYCLES);
	event("RESIDENT_TEST_TONE_HZ", 440);
	event("FOREGROUND_TONE_HZ", (int)AUDIO_PROBE_FREQUENCY_HZ);
	(void)printf("AIR PS3 resident audio stress harness; synthetic RAOP is not sent.\n");
	result = get_module_count(&baseline_module_count);
	if (result != 0) {
		event("MODULE_LIST_FAILED_AT_START", result);
		if (app_log != NULL)
			(void)fclose(app_log);
		return 1;
	}
	event("MODULE_COUNT_BASELINE", (int)baseline_module_count);
	if (!start_audio_probe()) {
		event("FOREGROUND_AUDIO_START_FAILED", 1);
		stop_audio_probe();
		if (app_log != NULL)
			(void)fclose(app_log);
		return 1;
	}
	for (cycle = 1; cycle <= TEST_CYCLES; ++cycle) {
		if (!run_cycle(cycle, baseline_module_count)) {
			event("HARNESS_FAIL", (int)cycle);
			stop_audio_probe();
			if (app_log != NULL)
				(void)fclose(app_log);
			return 1;
		}
	}
	event("HARNESS_PASS", (int)TEST_CYCLES);
	stop_audio_probe();
	if (app_log != NULL)
		(void)fclose(app_log);
	return 0;
}
