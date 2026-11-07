#ifndef PS3_AIRPLAY_RTSP_DIAGNOSTIC_H
#define PS3_AIRPLAY_RTSP_DIAGNOSTIC_H

#include <stddef.h>

#define RTSP_DIAG_REQUEST_MAX 4096
#define RTSP_DIAG_METHOD_MAX 16
#define RTSP_DIAG_CSEQ_MAX 16

typedef struct {
	char method[RTSP_DIAG_METHOD_MAX];
	char cseq[RTSP_DIAG_CSEQ_MAX];
	int status;
} rtsp_diag_result;

/* Handle one complete RTSP header block. No request body is retained/logged. */
int rtsp_diag_handle(const char *request, size_t request_length,
	char *response, size_t response_capacity, rtsp_diag_result *result);

#endif
