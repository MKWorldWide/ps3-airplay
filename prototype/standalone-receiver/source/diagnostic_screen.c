#include "diagnostic_screen.h"

#include <malloc.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <lv2/systime.h>

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

static int wait_rsx_idle(gcmContextData *context)
{
 static uint32_t label = 1;
 uint64_t deadline = sysGetSystemTime() + 500000U;
 uint32_t target = ++label;
 rsxSetWriteBackendLabel(context, GCM_LABEL_INDEX, target);
 rsxFlushBuffer(context);
 while (*(volatile uint32_t *)gcmGetLabelAddress(GCM_LABEL_INDEX) != target) {
  if ((uint64_t)sysGetSystemTime() >= deadline) return 0;
  usleep(1000);
 }
 return 1;
}

static int wait_initial_flip(void)
{
 uint64_t deadline = sysGetSystemTime() + 500000U;
 while (gcmGetFlipStatus() != 0) {
  if ((uint64_t)sysGetSystemTime() >= deadline) return 0;
  usleep(200);
 }
 return 1;
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
	if (!wait_rsx_idle(context))
		return DIAGNOSTIC_SCREEN_TIMEOUT;
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
	/* Prime scanout with the spare buffer, matching PSL1GHT videoTest. */
	if (gcmSetFlip(context, SCREEN_BUFFER_COUNT - 1) != 0) {
		error = DIAGNOSTIC_SCREEN_FLIP_FAILED;
		goto error;
	}
	rsxFlushBuffer(context);
	gcmSetWaitFlip(context);
	if (!wait_initial_flip()) {
		error = DIAGNOSTIC_SCREEN_TIMEOUT;
		goto error;
	}
	gcmResetFlipStatus();
	if (gcmSetFlip(context, 0) != 0) {
		error = DIAGNOSTIC_SCREEN_FLIP_FAILED;
		goto error;
	}
	rsxFlushBuffer(context);
	gcmSetWaitFlip(context);
	if (!wait_initial_flip()) {
		error = DIAGNOSTIC_SCREEN_TIMEOUT;
		goto error;
	}
	gcmResetFlipStatus();
	screen->image_rgb = rgb;
	screen->initialized = 1;
	screen->next_buffer = 1;
	return DIAGNOSTIC_SCREEN_OK;

error:
	free(rgb);
	if (screen->context != NULL) {
		diagnostic_screen_close(screen);
	} else {
		if (context == NULL || wait_rsx_idle(context))
			free(screen->host_memory);
	}
	memset(screen, 0, sizeof(*screen));
	return error;
}

/* The image is static. A single scanout refresh is sufficient after VSH
 * returns control; continuously flipping identical buffers can flicker. */
void diagnostic_screen_invalidate(diagnostic_screen *screen)
{
 if (screen == NULL || !screen->initialized) return;
 screen->redraw_mask = 3U;
 /* The compositor may have held a flip while its menu was active. */
 if (screen->flip_pending) screen->flip_started_us = sysGetSystemTime();
}

int diagnostic_screen_present(diagnostic_screen *screen, int system_menu_open)
{
 uint64_t now;
 uint32_t active_buffer;
 uint32_t buffer;
 if (screen == NULL || !screen->initialized) return 0;
 now = sysGetSystemTime();
 if (screen->flip_pending) {
  if (gcmGetFlipStatus() != 0)
   return !system_menu_open && now - screen->flip_started_us > 500000U ?
    DIAGNOSTIC_SCREEN_TIMEOUT : 0;
  screen->flip_pending = 0;
  if (system_menu_open) return 0;

  /* The just-completed target is active. Refresh the other buffer without
   * another flip unless invalidation dirtied the active image as well. */
  active_buffer = screen->next_buffer ^ 1U;
  buffer = screen->next_buffer;
  if (screen->redraw_mask & (1U << buffer)) {
   copy_image((uint32_t *)screen->buffers[buffer], screen->pitch,
    screen->width, screen->height, screen->image_rgb);
   screen->redraw_mask &= ~(1U << buffer);
  }
  if (!(screen->redraw_mask & (1U << active_buffer))) return 0;
 } else if (system_menu_open || screen->redraw_mask == 0) {
  return 0;
 }

 /* next_buffer is inactive whenever no flip is pending. */
 buffer = screen->next_buffer;
 if (screen->redraw_mask & (1U << buffer)) {
  copy_image((uint32_t *)screen->buffers[buffer], screen->pitch,
   screen->width, screen->height, screen->image_rgb);
  screen->redraw_mask &= ~(1U << buffer);
 }
 gcmResetFlipStatus();
 if (gcmSetFlip((gcmContextData *)screen->context, buffer) != 0)
  return DIAGNOSTIC_SCREEN_FLIP_FAILED;
 rsxFlushBuffer((gcmContextData *)screen->context);
 /* Queue the standard GPU-side wait without blocking the audio/input loop. */
 gcmSetWaitFlip((gcmContextData *)screen->context);
 screen->next_buffer ^= 1U;
 screen->flip_started_us = now;
 screen->flip_pending = 1;
 return 1;
}

int diagnostic_screen_close(diagnostic_screen *screen)
{
 uint32_t i;
 if (screen == NULL) return 0;
 /* On timeout, leave allocations for process teardown rather than freeing
  * memory that the RSX may still reference. No unbounded rsxFinish/waitFlip. */
 if (screen->context != NULL &&
     !wait_rsx_idle((gcmContextData *)screen->context)) {
  memset(screen, 0, sizeof(*screen));
  return DIAGNOSTIC_SCREEN_TIMEOUT;
 }
 for (i = 0; i < SCREEN_BUFFER_COUNT; ++i)
  if (screen->buffers[i] != NULL) rsxFree(screen->buffers[i]);
 free(screen->image_rgb);
 free(screen->host_memory);
 memset(screen, 0, sizeof(*screen));
 return 0;
}
