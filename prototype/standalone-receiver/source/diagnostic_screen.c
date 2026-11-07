#include "diagnostic_screen.h"

#include <malloc.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <rsx/gcm_sys.h>
#include <rsx/rsx.h>
#include <sysutil/video.h>

#define SCREEN_BUFFER_COUNT 2
#define SCREEN_COMMAND_BYTES 0x100000U
#define SCREEN_IO_BYTES (32U * 1024U * 1024U)
#define GCM_LABEL_INDEX 255U
#define IMAGE_WIDTH 1280U
#define IMAGE_HEIGHT 720U
#define IMAGE_RGB_BYTES (IMAGE_WIDTH * IMAGE_HEIGHT * 3U)

static int load_rgb_image(const char *path, uint8_t *rgb, size_t expected)
{
	FILE *file = fopen(path, "rb");
	size_t bytes;
	int extra;
	if (file == NULL)
		return 0;
	bytes = fread(rgb, 1, expected, file);
	extra = fgetc(file);
	(void)fclose(file);
	return bytes == expected && extra == EOF;
}

static void copy_image(uint32_t *buffer, uint32_t pitch, uint16_t width,
	uint16_t height, const uint8_t *rgb)
{
	uint32_t x, y;
	for (y = 0; y < height; ++y) {
		uint32_t source_y = (y * IMAGE_HEIGHT) / height;
		uint32_t *row = (uint32_t *)((uint8_t *)buffer + y * pitch);
		for (x = 0; x < width; ++x) {
			uint32_t source_x = (x * IMAGE_WIDTH) / width;
			size_t source = ((size_t)source_y * IMAGE_WIDTH + source_x) * 3U;
			row[x] = ((uint32_t)rgb[source] << 16) |
				((uint32_t)rgb[source + 1] << 8) | rgb[source + 2];
		}
	}
}

static void wait_rsx_idle(gcmContextData *context)
{
	uint32_t label = 1;
	rsxSetWriteBackendLabel(context, GCM_LABEL_INDEX, label);
	rsxSetWaitLabel(context, GCM_LABEL_INDEX, label);
	++label;
	rsxSetWriteBackendLabel(context, GCM_LABEL_INDEX, label);
	rsxFlushBuffer(context);
	while (*(volatile uint32_t *)gcmGetLabelAddress(GCM_LABEL_INDEX) != label)
		usleep(30);
}

static int setup_video_mode(gcmContextData *context, videoState *state,
	videoResolution *resolution)
{
	videoConfiguration configuration;
	if (videoGetState(0, 0, state) != 0 || state->state != 0 ||
		videoGetResolution(state->displayMode.resolution, resolution) != 0)
		return DIAGNOSTIC_SCREEN_VIDEO_STATE_FAILED;
	memset(&configuration, 0, sizeof(configuration));
	configuration.resolution = state->displayMode.resolution;
	configuration.format = VIDEO_BUFFER_FORMAT_XRGB;
	configuration.pitch = rsxAlign(64,
		(uint32_t)resolution->width * sizeof(uint32_t));
	configuration.aspect = state->displayMode.aspect;
	wait_rsx_idle(context);
	if (videoConfigure(0, &configuration, NULL, 0) != 0 ||
		videoGetState(0, 0, state) != 0)
		return DIAGNOSTIC_SCREEN_VIDEO_CONFIG_FAILED;
	gcmSetFlipMode(GCM_FLIP_VSYNC);
	gcmResetFlipStatus();
	(void)context;
	return DIAGNOSTIC_SCREEN_OK;
}

int diagnostic_screen_open(diagnostic_screen *screen, const char *rgb_path)
{
	gcmContextData *context = NULL;
	videoState state;
	videoResolution resolution;
	uint8_t *rgb = NULL;
	uint32_t i;
	int error = DIAGNOSTIC_SCREEN_OK;
	if (screen == NULL || rgb_path == NULL)
		return DIAGNOSTIC_SCREEN_BAD_ARGUMENT;
	memset(screen, 0, sizeof(*screen));
	screen->host_memory = memalign(1024U * 1024U, SCREEN_IO_BYTES);
	rgb = (uint8_t *)malloc(IMAGE_RGB_BYTES);
	if (screen->host_memory == NULL || rgb == NULL) {
		error = DIAGNOSTIC_SCREEN_MEMORY_FAILED;
		goto error;
	}
	if (!load_rgb_image(rgb_path, rgb, IMAGE_RGB_BYTES)) {
		error = DIAGNOSTIC_SCREEN_IMAGE_FAILED;
		goto error;
	}
	if (rsxInit(&context, SCREEN_COMMAND_BYTES, SCREEN_IO_BYTES,
		screen->host_memory) != 0 || context == NULL) {
		error = DIAGNOSTIC_SCREEN_RSX_INIT_FAILED;
		goto error;
	}
	error = setup_video_mode(context, &state, &resolution);
	if (error != DIAGNOSTIC_SCREEN_OK)
		goto error;
	screen->width = resolution.width;
	screen->height = resolution.height;
	screen->pitch = rsxAlign(64, (uint32_t)screen->width * sizeof(uint32_t));
	screen->context = context;
	for (i = 0; i < SCREEN_BUFFER_COUNT; ++i) {
		size_t bytes = (size_t)screen->pitch * screen->height;
		screen->buffers[i] = rsxMemalign(64, bytes);
		if (screen->buffers[i] == NULL) {
			error = DIAGNOSTIC_SCREEN_BUFFER_ALLOC_FAILED;
			goto error;
		}
		if (rsxAddressToOffset(screen->buffers[i], &screen->offsets[i]) != 0) {
			error = DIAGNOSTIC_SCREEN_BUFFER_MAP_FAILED;
			goto error;
		}
		if (gcmSetDisplayBuffer(i, screen->offsets[i], screen->pitch,
			screen->width, screen->height) != 0) {
			error = DIAGNOSTIC_SCREEN_BUFFER_REGISTER_FAILED;
			goto error;
		}
		copy_image((uint32_t *)screen->buffers[i], screen->pitch,
			screen->width, screen->height, rgb);
	}
	free(rgb);
	/* Prime scanout with the spare buffer, matching PSL1GHT videoTest. */
	if (gcmSetFlip(context, SCREEN_BUFFER_COUNT - 1) != 0) {
		error = DIAGNOSTIC_SCREEN_FLIP_FAILED;
		goto error;
	}
	rsxFlushBuffer(context);
	gcmSetWaitFlip(context);
	while (gcmGetFlipStatus() != 0)
		usleep(200);
	gcmResetFlipStatus();
	if (gcmSetFlip(context, 0) != 0) {
		error = DIAGNOSTIC_SCREEN_FLIP_FAILED;
		goto error;
	}
	rsxFlushBuffer(context);
	gcmSetWaitFlip(context);
	while (gcmGetFlipStatus() != 0)
		usleep(200);
	gcmResetFlipStatus();
	screen->initialized = 1;
	return DIAGNOSTIC_SCREEN_OK;

error:
	free(rgb);
	if (screen->context != NULL) {
		diagnostic_screen_close(screen);
	} else {
		if (context != NULL)
			rsxFinish(context, 1);
		free(screen->host_memory);
	}
	memset(screen, 0, sizeof(*screen));
	return error;
}

void diagnostic_screen_close(diagnostic_screen *screen)
{
	uint32_t i;
	if (screen == NULL)
		return;
	if (screen->context != NULL) {
		if (screen->initialized)
			gcmSetWaitFlip((gcmContextData *)screen->context);
		for (i = 0; i < SCREEN_BUFFER_COUNT; ++i) {
			if (screen->buffers[i] != NULL)
				rsxFree(screen->buffers[i]);
		}
		rsxFinish((gcmContextData *)screen->context, 1);
	}
	free(screen->host_memory);
	memset(screen, 0, sizeof(*screen));
}
