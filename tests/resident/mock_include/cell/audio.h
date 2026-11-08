#ifndef RESIDENT_HOST_MOCK_CELL_AUDIO_H
#define RESIDENT_HOST_MOCK_CELL_AUDIO_H

#include <stdint.h>

#define CELL_AUDIO_PORT_2CH 2
#define CELL_AUDIO_BLOCK_8 8
#define CELL_AUDIO_BLOCK_SAMPLES 256
#define CELL_AUDIO_ERROR_ALREADY_INIT 0x80310701

typedef struct {
	uint64_t nChannel;
	uint64_t nBlock;
	uint64_t attr;
	float level;
} CellAudioPortParam;

int cellAudioInit(void);
int cellAudioQuit(void);
int cellAudioPortOpen(CellAudioPortParam *param, uint32_t *port);
int cellAudioPortClose(uint32_t port);
int cellAudioPortStart(uint32_t port);
int cellAudioPortStop(uint32_t port);
int cellAudioAdd2chData(uint32_t port, float *samples,
	unsigned int count, float volume);

#endif
