# ALAC decoder fixtures

`alac-ffmpeg-352.packet` and `alac-ffmpeg-352.pcm` were extracted from a
CAF file generated with FFmpeg 63.1.102's ALAC encoder from a 440 Hz, 44.1 kHz,
stereo, signed 16-bit sine source. The 352-byte encoded packet decodes to 353
samples (the final packet is padded by the CAF muxer); the PCM fixture is the
CAF-decoded signed 16-bit little-endian output. The decoder test configures a
4096-sample ALAC frame limit as specified by the CAF magic cookie and compares
the exact valid decoded PCM bytes.

## Synthetic incompressible boundary fixture

`alac-noise-352.packet` is a valid 1,416-byte ALAC packet representing 352
stereo samples. `alac-noise-352.pcm` is the exact 1,408-byte input/reference.
This is fabricated deterministic noise, not captured user audio. Generate 704
signed 16-bit values with Python `random.Random(6161).randrange(-32768,32768)`,
pack each little-endian (`struct.pack('<h', value)`), encode using FFmpeg:

```sh
ffmpeg -f s16le -ar 44100 -ac 2 -i noise.pcm -c:a alac noise.m4a
ffmpeg -i noise.m4a -map 0:a -c copy -f data noise.packet
```

The forensic payload test shows production's 1,400-byte RTP/ALAC caps reject
this valid packet, while an isolated 1,536-byte-cap experiment accepts it and
produces byte-exact reference PCM. Raising these caps is not a deployed fix.
