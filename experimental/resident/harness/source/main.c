#include <sys/prx.h>
#include <cell/audio.h>

#include <math.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include "resident_diagnostics.h"

#define APP_LOG_PATH "/dev_hdd0/tmp/ps3-airplay-resident-harness.log"
#define MODULE_PATH "/dev_hdd0/game/RPRX90001/USRDIR/ps3-airplay-receiver.fake.sprx"
#define TEST_CYCLES 3U
#define EVENT_WAIT_POLLS 400U
#define EVENT_POLL_US 25000U
#define MODULE_LIST_CAPACITY 256U
#define AUDIO_PROBE_BLOCKS 940U
#define AUDIO_PROBE_AMPLITUDE 0.12f

static FILE *app_log;
static uint32_t foreground_audio_port;
static float foreground_audio_phase;
static int foreground_audio_initialized;
static int foreground_audio_opened;
static int foreground_audio_started;

static void event(const char *name, int value)
{
	if (app_log != NULL) {
		(void)fprintf(app_log, "event=%s value=%d\n", name, value);
		(void)fflush(app_log);
	}
	(void)printf("event=%s value=%d\n", name, value);
}

static int wait_for_diagnostic_event(
	const airps3_resident_diagnostics *diagnostics, uint32_t event_mask)
{
	unsigned int poll;
	for (poll = 0; poll < EVENT_WAIT_POLLS; ++poll) {
		if ((__atomic_load_n(&diagnostics->events, __ATOMIC_ACQUIRE) &
			event_mask) != 0)
			return 1;
		(void)usleep(EVENT_POLL_US);
	}
	return 0;
}

static int wait_for_audio_pcm(const airps3_resident_diagnostics *diagnostics)
{
	unsigned int poll;
	for (poll = 0; poll < EVENT_WAIT_POLLS; ++poll) {
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
	const float phase_step = 6.2831853071795864769f * 440.0f / 48000.0f;
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
	unsigned int baseline_module_count, unsigned int cycle)
{
	unsigned int module_count;
	int module_result = 0;
	int result;
	int events_ok = 1;
	result = sys_prx_stop_module(module_id, 0, NULL, &module_result, 0, NULL);
	event("STOP_RESULT", result);
	if (result != 0)
		return 0;
	if (!wait_for_diagnostic_event(diagnostics,
		AIRPS3_DIAG_EVENT_WORKER_STOPPED)) {
		event("WORKER_STOP_EVENT_MISSING", (int)cycle);
		events_ok = 0;
	} else {
		event("WORKER_STOPPED", (int)cycle);
	}
	if (!wait_for_diagnostic_event(diagnostics,
		AIRPS3_DIAG_EVENT_MODULE_STOP)) {
		event("MODULE_STOP_EVENT_MISSING", (int)cycle);
		events_ok = 0;
	}
	result = sys_prx_unload_module(module_id, 0, NULL);
	event("UNLOAD_RESULT", result);
	if (result != 0)
		return 0;
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
	airps3_resident_diagnostics diagnostics = {
		AIRPS3_RESIDENT_DIAGNOSTICS_MAGIC, 0U, 0, 0U
	};
	char name[32];
	(void)snprintf(name, sizeof(name), "CYCLE_%u_BEGIN", cycle);
	event(name, (int)cycle);

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
	if ((diagnostic_events & AIRPS3_DIAG_EVENT_STARTED) == 0U) {
		event("MODULE_START_EVENT_MISSING", (int)cycle);
		(void)stop_and_unload(module_id, &diagnostics,
			baseline_module_count, cycle);
		return 0;
	}
	if (!wait_for_diagnostic_event(&diagnostics,
		AIRPS3_DIAG_EVENT_RECEIVER_READY)) {
		event("RECEIVER_READY_TIMEOUT", (int)cycle);
		(void)stop_and_unload(module_id, &diagnostics,
			baseline_module_count, cycle);
		return 0;
	}
	event("RECEIVER_READY", (int)cycle);
	if (!wait_for_audio_pcm(&diagnostics)) {
		event("RESIDENT_PCM_SUBMISSION_MISSING", (int)cycle);
		(void)stop_and_unload(module_id, &diagnostics,
			baseline_module_count, cycle);
		return 0;
	}
	event("RESIDENT_PCM_SUBMITTED_TO_MIXER", (int)cycle);
	event("TWO_ACTIVE_AUDIO_PORTS", (int)cycle);
	if (!submit_foreground_audio_probe(AUDIO_PROBE_BLOCKS)) {
		(void)stop_and_unload(module_id, &diagnostics,
			baseline_module_count, cycle);
		return 0;
	}

	if (!stop_and_unload(module_id, &diagnostics,
		baseline_module_count, cycle))
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
	app_log = fopen(APP_LOG_PATH, "wb");
	event(app_log != NULL ? "HARNESS_START" : "HARNESS_LOG_OPEN_FAILED", 0);
	(void)printf("AIR PS3 resident audio-backend and lifecycle test; no synthetic RAOP stream is sent.\n");
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
