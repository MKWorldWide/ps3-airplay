# Active work

- The PS3 is currently running installed AIRPLAY02 v0.1.33; the latest read-only log snapshot ends at `SCREEN_FRAMES`. Do not replace the title until it exits.
- The user hears Spotify AirPlay but reports crackle, worse while scrolling XMB; resumed image flickers/dims with a frozen loading icon. Relaunch restores a solid background.
- Latest v0.1.33 log: encrypted 44.1 kHz stream, 1,664 RTP packets, 88 sequence gaps, zero ALAC failures, five resend-recovery events. The 51,200-frame underflow counter includes intentional silence during startup prebuffer and is not proof of a playback dropout.
- v0.1.35 adds valid active-stream underflow accounting, session-scoped counters, and v0.1.34's bounded resend recovery. Lima host/sanitizer suite, PPU build, ELF/SFO and finalized package checks pass. The 3,236,016-byte PKG is staged and byte-verified. Current v0.1.33 is still streaming; install v0.1.35 only after it exits cleanly.
- User-supplied MacBook recording is 22.5s mono/48kHz, quiet (peak -23.9 dBFS, RMS -40.7 dBFS); no hard digital silence or gross discontinuity detected. It cannot resolve subtle crackle by itself. Wrong-input AVFoundation capture was discarded; do not publish the audio.
- Keep raw logs and packages ignored under `work/`; do not alter boot plugins or the resident module.
