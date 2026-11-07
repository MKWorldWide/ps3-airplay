# ALAC decoder fixtures

`alac-ffmpeg-352.packet` and `alac-ffmpeg-352.pcm` were extracted from a
CAF file generated with FFmpeg 63.1.102's ALAC encoder from a 440 Hz, 44.1 kHz,
stereo, signed 16-bit sine source. The 352-byte encoded packet decodes to 353
samples (the final packet is padded by the CAF muxer); the PCM fixture is the
CAF-decoded signed 16-bit little-endian output. The decoder test configures a
4096-sample ALAC frame limit as specified by the CAF magic cookie and compares
the exact valid decoded PCM bytes.
