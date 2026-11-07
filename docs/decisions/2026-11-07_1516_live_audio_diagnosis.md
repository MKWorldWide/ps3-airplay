# Live AirPlay audio diagnosis

## Timestamp in EDT

2026-11-07 15:16 EDT

## Scope of the pass

Retrieved a live PS3 log, verified the installed application identity against local build artifacts, analyzed the user's supplied microphone recording, and recorded the latest audio/display observations. No source or console files were modified in this diagnostic pass.

## Why this pass was needed

The user hears crackling audio that becomes more noticeable while navigating XMB, and the screen flickers/dims after resuming from XMB. A v0.1.34 recovery build was already staged, but its installation state was uncertain.

## Files changed

- `docs/memory.md`
- `docs/BUILD_PIPELINE.md`
- `docs/VERSIONING.md`
- `docs/lessons-learned.md`
- `docs/scratchpad.md`
- `docs/ROADMAP.md`
- `docs/TEST_MATRIX.md`
- `docs/RISKS.md`
- `docs/reports/2026-11-07_v0133_audio_followup.md`
- `docs/decisions/2026-11-07_1516_live_audio_diagnosis.md`
- User-supplied recording was analyzed locally and remains outside the repository.

## Architecture / logic changes

No runtime architecture changed. FTP retrieval confirmed the installed SFO and EBOOT exactly match AIRPLAY02 v0.1.33, whereas v0.1.34 is staged and its package matches the local candidate. The latest v0.1.33 log reports RTP gaps and PCM underflow while decryption and ALAC decode continue.

## Runtime behavior changes

None. The console remained in the current user test; no install, launch, reboot, or process termination was issued in this pass.

## Validation performed

- FTP retrieved `/dev_hdd0/tmp/ps3-airplay.log` read-only; the 16,275-byte snapshot ended on `SCREEN_FRAMES`.
- Installed `PARAM.SFO` and `EBOOT.BIN` were compared byte-for-byte with the local v0.1.33 and v0.1.34 candidates. Only v0.1.33 matched.
- Staged v0.1.34 PKG was compared byte-for-byte with the local artifact; it matched at 3,236,016 bytes.
- Parsed only allowlisted event names and numeric counters from the private log; raw log and identifiers remain ignored under `work/`.
- FFmpeg decoded the supplied AIFC to mono 48 kHz float samples in memory. Peak/RMS, short-window levels, and adjacent-sample jumps were calculated; no copy of the recording was created in the repository.
- The partial capture taken from the wrong AVFoundation input was excluded; microphone index zero was not assumed to be the MacBook mic.

## Immediate results

- v0.1.33 log shows 1,664 received RTP packets, 88 sequence gaps, four resend requests, five recovery events, zero ALAC failure totals, and a 51,200-frame PCM-underflow interval.
- User-reported crackle and XMB display flicker remain unresolved. The process was still active at capture time.
- The supplied sample has a quiet acoustic level and no gross silence interval or large discontinuity; this cannot rule out subtle crackle.
- The 22.485-second supplied sample has a quiet acoustic level and no gross silence interval or large discontinuity; this cannot rule out subtle crackle.
- v0.1.34 is correctly staged but not installed. Its pending resend-range changes are the next controlled comparison.

## Known limitations / follow-ups

No controlled v0.1.34 hardware run or Spotify-quality A/B has occurred. The wrapped log lacks startup/RTSP negotiation and cannot establish which source quality Spotify sent. Display resume and audio quality must be assessed separately.

## Risks introduced

None. Avoid replacing AIRPLAY02 while its process is active. Do not interpret successful ALAC decode as proof of continuous audible output.

## Decision record

### Decision: Test v0.1.34 before adding another audio change

- **Context:** The v0.1.33 log contains sequence gaps and an output-underflow interval; v0.1.34 specifically changes resend retention and tracking.
- **Options considered:** Adjust Spotify quality immediately; enlarge buffers; modify the decoder; first test the already-built recovery candidate.
- **Why chosen:** It isolates a proven code-path difference without introducing a second unmeasured variable.
- **Tradeoffs accepted:** Crackling may persist if the main cause is system scheduling or output timing; one short A/B remains necessary afterward.

### Decision: Do not install while the title is running

- **Context:** The live log ended on a display heartbeat rather than `APP_STOP`.
- **Options considered:** Force the process closed remotely or wait for its clean exit.
- **Why chosen:** A running title could be disrupted or produce ambiguous install state.
- **Tradeoffs accepted:** Hardware validation waits for the user-visible app to exit.

## Next recommended step

After AIRPLAY02 v0.1.33 exits to XMB, build and install v0.1.35 (which includes v0.1.34 recovery plus corrected underflow accounting), then run a short stream with XMB idle and while navigating. Compare missing/recovered RTP packets and active-playback PCM underflows.
