# v0.1.35 audio and display follow-up

**Capture window:** 2026-11-07 21:15:09–21:15:43 UTC
**Installed title:** AIRPLAY02, SFO APP_VER 00.35
**Test identifier:** HW-035-LOG-FOLLOWUP

## Evidence

Read-only FTP retrieved the capped PS3 log and installed PARAM.SFO. SFO validation confirms AIRPLAY02 / 00.35. The log is 8,911 bytes (SHA-256 `70910fed1c54f1421d6e3f46d4b7991100e6c2857d9109b02a3bee53d024845d`). The startup record wrapped, so the captured log itself does not identify its package version; installed SFO provides the current version boundary. The installed EBOOT was not compared with a local package in this pass.

The retained stream section shows encrypted 44.1 kHz audio, five 512-packet receive checkpoints (2,560 received packets), 195 missing-sequence observations, 214 late-packet observations, three resend-recovery checkpoints reporting zero recovered packets, zero ALAC failures, and 86,106 active-stream PCM output-underflow frames. The underflows equal about 1.79 seconds at 48 kHz. Each missing ALAC packet represents 352 source samples; the gap duration is of the same order as the reported starvation. This points to unrecovered RTP loss as the leading cause, not as the sole proven cause.

`SCREEN_FRAMES` advanced by 565–566 between ten-second heartbeats, about 56.5 flips/second. There was no `SCREEN_PRESENT_FAILED` event. Repeated flipping is verified; the capture cannot prove it caused the user's visual flicker.

The log records RTSP teardown/disconnect followed by idle heartbeats. It does not contain `APP_STOP`, so this was not a clean application exit. No console state was changed during capture. The staged package directory was separately confirmed empty after the user's cleanup request.

## Assessment

The current source already linearly resamples 44.1 kHz PCM to the 48 kHz GameOS audio port. The measured packet gaps and zero recovery are a stronger lead than a speculative resampler change. The v0.1.36 resend fix has not yet been tested on hardware because the installed app remains v0.1.35.

The display source submitted flips continuously at approximately display refresh rate despite a static image. The next candidate stops that loop, refreshes once after the system menu closes, then updates the inactive framebuffer. This reduces unnecessary scanout activity but needs visible hardware verification.

## Next test

Build v0.1.37 / SFO 00.37 from committed source. Run the existing sanitizer suite, validate the PPU ELF and package, then use RPCS3 synthetic audio to verify late-resend recovery and PCM flow. On PS3, compare RTP recovery and PCM underflow totals, verify the static image before and after two menu resumes, and confirm START exits cleanly. Treat user hearing and visible screen observation as separate acceptance results.
