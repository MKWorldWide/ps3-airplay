#ifndef PS3_AIRPLAY_AUDIO_LIMITS_H
#define PS3_AIRPLAY_AUDIO_LIMITS_H
/* Includes uncompressed 352-frame stereo ALAC (1416 bytes) and framing.
 * Larger compressed frames remain bounded by this shared encoded limit. */
#define RAOP_ENCODED_AUDIO_MAX 1536U
#endif
