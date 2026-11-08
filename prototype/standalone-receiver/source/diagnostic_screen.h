#ifndef PS3_AIRPLAY_DIAGNOSTIC_SCREEN_H
#define PS3_AIRPLAY_DIAGNOSTIC_SCREEN_H

#include <stdint.h>

typedef struct {
	void *context;
	void *host_memory;
	uint8_t *image_rgb;
	uint32_t redraw_mask;
	void *buffers[2];
	uint32_t offsets[2];
	uint16_t width;
	uint16_t height;
	uint32_t pitch;
	int initialized;
	int flip_pending;
	uint32_t next_buffer;
	uint64_t flip_started_us;
	uint64_t resume_until_us;
} diagnostic_screen;

enum diagnostic_screen_result {
	DIAGNOSTIC_SCREEN_OK = 0,
	DIAGNOSTIC_SCREEN_BAD_ARGUMENT = -1,
	DIAGNOSTIC_SCREEN_MEMORY_FAILED = -2,
	DIAGNOSTIC_SCREEN_IMAGE_FAILED = -3,
	DIAGNOSTIC_SCREEN_RSX_INIT_FAILED = -4,
	DIAGNOSTIC_SCREEN_VIDEO_STATE_FAILED = -5,
	DIAGNOSTIC_SCREEN_VIDEO_CONFIG_FAILED = -6,
	DIAGNOSTIC_SCREEN_BUFFER_ALLOC_FAILED = -7,
	DIAGNOSTIC_SCREEN_BUFFER_MAP_FAILED = -8,
	DIAGNOSTIC_SCREEN_BUFFER_REGISTER_FAILED = -9,
	DIAGNOSTIC_SCREEN_FLIP_FAILED = -10,
	DIAGNOSTIC_SCREEN_TIMEOUT = -11
};

int diagnostic_screen_open(diagnostic_screen *screen, const char *rgb_path);
void diagnostic_screen_invalidate(diagnostic_screen *screen);
int diagnostic_screen_present(diagnostic_screen *screen, int system_menu_open);
int diagnostic_screen_close(diagnostic_screen *screen);

#endif
