#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include <ppu-lv2.h>
#include <ppu-types.h>
#include <sys/memory.h>
#include <sys/process.h>
#include <sysutil/video.h>
#include <io/pad.h>
#include <cell/gcm.h>
#include <rsx/rsx.h>

SYS_PROCESS_PARAM(1001, 0x100000);

#define APP_LOG_PATH "/dev_hdd0/tmp/ps3-airplay-native-launch.log"
#define COMMAND_BUFFER_SIZE (1024u * 1024u)
#define HOST_BUFFER_SIZE (32u * 1024u * 1024u)
#define DISPLAY_BUFFERS 2
#define LABEL_INDEX 255
#define MAX_WAIT_POLLS 1500
#define FRAME_LIMIT 1200
#define COLOR_FRAMES 120

typedef struct {
	uint32_t *pixels;
	uint32_t offset;
	uint16_t width;
	uint16_t height;
	uint8_t id;
} display_buffer;

static FILE *g_log;
static uint32_t g_label_value;
static uint32_t g_pitch;
static uint32_t g_depth_offset;
static void *g_depth_buffer;

static void log_event(const char *event)
{
	if (g_log != NULL) {
		(void)fprintf(g_log, "%s\n", event);
		(void)fflush(g_log);
	}
	(void)printf("%s\n", event);
}

static int wait_rsx_idle(CellGcmContextData *ctx)
{
	volatile uint32_t *label = cellGcmGetLabelAddress(LABEL_INDEX);
	if (label == NULL)
		return 0;

	g_label_value++;
	cellGcmSetWriteBackEndLabel(ctx, LABEL_INDEX, g_label_value);
	cellGcmFlush(ctx);
	for (uint32_t i = 0; i < MAX_WAIT_POLLS; i++) {
		if (*label == g_label_value)
			return 1;
		usleep(1000);
	}
	return 0;
}

static int wait_flip(void)
{
	for (uint32_t i = 0; i < MAX_WAIT_POLLS; i++) {
		if (cellGcmGetFlipStatus() == 0) {
			cellGcmResetFlipStatus();
			return 1;
		}
		usleep(1000);
	}
	return 0;
}

static int make_buffer(display_buffer *buffer, uint16_t width,
		uint16_t height, uint8_t id)
{
	const uint32_t bytes = (uint32_t)width * (uint32_t)height * sizeof(uint32_t);
	buffer->pixels = (uint32_t *)rsxMemalign(64, bytes);
	if (buffer->pixels == NULL)
		return 0;
	if (cellGcmAddressToOffset(buffer->pixels, &buffer->offset) != 0)
		return 0;
	if (cellGcmSetDisplayBuffer(id, buffer->offset, g_pitch,
			width, height) != 0)
		return 0;
	buffer->width = width;
	buffer->height = height;
	buffer->id = id;
	return 1;
}

static void set_render_target(CellGcmContextData *ctx,
		const display_buffer *buffer)
{
	CellGcmSurface surface = {0};
	surface.colorFormat = GCM_SURFACE_X8R8G8B8;
	surface.colorTarget = GCM_SURFACE_TARGET_0;
	surface.colorLocation[0] = GCM_LOCATION_RSX;
	surface.colorOffset[0] = buffer->offset;
	surface.colorPitch[0] = g_pitch;
	surface.colorLocation[1] = GCM_LOCATION_RSX;
	surface.colorLocation[2] = GCM_LOCATION_RSX;
	surface.colorLocation[3] = GCM_LOCATION_RSX;
	surface.colorPitch[1] = 64;
	surface.colorPitch[2] = 64;
	surface.colorPitch[3] = 64;
	surface.depthFormat = GCM_SURFACE_ZETA_Z16;
	surface.depthLocation = GCM_LOCATION_RSX;
	surface.depthOffset = g_depth_offset;
	surface.depthPitch = g_pitch;
	surface.type = GCM_SURFACE_TYPE_LINEAR;
	surface.antiAlias = GCM_SURFACE_CENTER_1;
	surface.width = buffer->width;
	surface.height = buffer->height;
	surface.x = 0;
	surface.y = 0;
	cellGcmSetSurface(ctx, &surface);
}

static int setup_video(sys_addr_t host_address, display_buffer *buffers,
		uint16_t *width, uint16_t *height, CellGcmContextData **out_ctx)
{
	videoState state;
	videoConfiguration config;
	videoResolution resolution;
	CellGcmContextData *ctx;

	if (cellGcmInit(COMMAND_BUFFER_SIZE, HOST_BUFFER_SIZE,
			(void *)(uintptr_t)host_address) != 0)
		return 0;
	ctx = CELL_GCM_CURRENT;
	*out_ctx = ctx;
	if (videoGetState(0, 0, &state) != 0 || state.state != 0)
		return 0;
	if (videoGetResolution(state.displayMode.resolution, &resolution) != 0)
		return 0;
	memset(&config, 0, sizeof(config));
	config.resolution = state.displayMode.resolution;
	config.format = VIDEO_BUFFER_FORMAT_XRGB;
	config.pitch = resolution.width * sizeof(uint32_t);
	config.aspect = state.displayMode.aspect;
	if (!wait_rsx_idle(ctx))
		return 0;
	if (videoConfigure(0, &config, NULL, 0) != 0)
		return 0;

	g_pitch = resolution.width * sizeof(uint32_t);
	g_depth_buffer = rsxMemalign(64,
			(uint32_t)resolution.height * g_pitch * 2u);
	if (g_depth_buffer == NULL ||
			cellGcmAddressToOffset(g_depth_buffer, &g_depth_offset) != 0)
		return 0;
	cellGcmSetFlipMode(GCM_FLIP_VSYNC);
	cellGcmResetFlipStatus();
	*width = resolution.width;
	*height = resolution.height;
	for (uint8_t i = 0; i < DISPLAY_BUFFERS; i++) {
		if (!make_buffer(&buffers[i], *width, *height, i))
			return 0;
	}
	return 1;
}

static int queue_flip(CellGcmContextData *ctx, uint8_t id)
{
	if (cellGcmSetFlip(ctx, id) != 0)
		return 0;
	cellGcmFlush(ctx);
	return 1;
}

int main(int argc, const char **argv)
{
	(void)argc;
	(void)argv;

	static const uint32_t colors[] = {
		0xff00a8ffu, 0xffff2020u, 0xff20e060u,
		0xffffd020u, 0xffd020ffu, 0xff20e8e8u
	};
	sys_addr_t host_address = 0;
	display_buffer buffers[DISPLAY_BUFFERS] = {{0}};
	padInfo pad_info;
	uint16_t width = 0;
	uint16_t height = 0;
	uint8_t current_buffer = 0;
	uint16_t previous_start[4] = {0};
	int exit_by_user = 0;
	int result = 1;
	CellGcmContextData *ctx = NULL;

	g_log = fopen(APP_LOG_PATH, "wb");
	log_event("APP_START_NATIVE_PS3DK_0_1_0");
	log_event(g_log != NULL ? "LOG_OPEN_OK" : "LOG_OPEN_FAILED");
	if (ioPadInit(7) != 0) {
		log_event("CONTROLLER_INIT_FAILED");
		goto cleanup;
	}
	log_event("CONTROLLER_READY");

	if (sys_memory_allocate(HOST_BUFFER_SIZE, SYS_MEMORY_PAGE_SIZE_1M,
			&host_address) != 0) {
		log_event("HOST_BUFFER_ALLOC_FAILED");
		goto cleanup;
	}
	log_event("HOST_BUFFER_READY");
	if (!setup_video(host_address, buffers, &width, &height, &ctx)) {
		log_event("DISPLAY_INIT_FAILED");
		goto cleanup;
	}
	ctx = CELL_GCM_CURRENT;
	log_event("DISPLAY_INIT_READY");
	(void)printf("Native launch control active: %ux%u; press START to exit\n",
			width, height);

	if (!queue_flip(ctx, (uint8_t)(DISPLAY_BUFFERS - 1)) || !wait_flip()) {
		log_event("INITIAL_FLIP_FAILED");
		goto cleanup;
	}
	log_event("DISPLAY_CYCLE_START");

	for (uint32_t frame = 0; frame < FRAME_LIMIT; frame++) {
		if ((frame % 30u) == 0u) {
			(void)ioPadGetInfo(&pad_info);
			for (uint32_t pad = 0; pad < 4; pad++) {
				if (pad_info.status[pad]) {
					padData data = {0};
					if (ioPadGetData(pad, &data) == 0) {
						const uint16_t start = data.BTN_START;
						if (start && !previous_start[pad]) {
							exit_by_user = 1;
							break;
						}
						previous_start[pad] = start;
					}
				}
			}
			if (exit_by_user)
				break;
		}

		set_render_target(ctx, &buffers[current_buffer]);
		cellGcmSetClearColor(ctx, colors[(frame / COLOR_FRAMES) %
				(uint32_t)(sizeof(colors) / sizeof(colors[0]))]);
		cellGcmSetClearSurface(ctx,
				GCM_CLEAR_R | GCM_CLEAR_G | GCM_CLEAR_B | GCM_CLEAR_A);
		if (!wait_flip() || !queue_flip(ctx, buffers[current_buffer].id)) {
			log_event("DISPLAY_FRAME_TIMEOUT");
			goto cleanup;
		}
		current_buffer = (uint8_t)((current_buffer + 1u) % DISPLAY_BUFFERS);
		usleep(16000);
	}

	log_event(exit_by_user ? "USER_EXIT_START" : "AUTO_EXIT_TIMEOUT");
	result = 0;

cleanup:
	if (ctx != NULL && !wait_rsx_idle(ctx)) {
		log_event("RSX_DRAIN_TIMEOUT");
		result = 1;
	}
	/* GCM owns the host-buffer mapping until process exit. */
	(void)ioPadEnd();
	log_event("APP_STOP");
	if (g_log != NULL) {
		(void)fclose(g_log);
		g_log = NULL;
	}
	return result;
}
