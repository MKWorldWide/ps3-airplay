# Instrument controller input after v0.2.0 hardware failure

## Timestamp

2026-11-07 19:49 America/New_York

## Scope of the pass

Document v0.2.0's install/launch/stream/exit evidence and add privacy-safe controller state diagnostics for the next patch candidate, v0.2.1 / SFO 00.41.

## Why this pass was needed

During a live v0.2.0 song test, the user reported that neither controller START nor the PS button worked. The existing log indicated successful polling and pad presence but recorded no button states.

## Files changed

- `prototype/standalone-receiver/source/main.c`
- `prototype/standalone-receiver/Makefile`
- `scripts/build-lima-standalone-receiver.sh`
- `scripts/package-lima-standalone-receiver.sh`
- `docs/ROADMAP.md`
- `docs/TEST_MATRIX.md`
- `docs/VERSIONING.md`
- `docs/memory.md`
- `docs/lessons-learned.md`
- `docs/reports/2026-11-07_v0200_candidate.md`
- `docs/scratchpad.md`
- this decision log

## Architecture / logic changes

The controller poller now emits bounded state-transition events for pad connect/disconnect, selected button-mask changes, and pad data/API errors. The system callback continues to emit numeric status and additionally names menu-open, menu-close, and exit-game events. The port loop is limited to PSL1GHT's seven defined ports, avoiding shifts by unsupported pad indices.

## Runtime behavior changes

No intended control semantics change: START still requests orderly exit, and the PS button remains system-managed. The upcoming package identity advances to app `0.2.1`, SFO `00.41`. Package build, install, and retest are pending.

## Validation performed

- Preserved four private v0.2.0 snapshots plus pre-exit and cleanup logs outside Git.
- Confirmed installed SFO was `00.40`; the installed EBOOT's first 450,944 bytes match the validated package SELF exactly, followed by 48 zero bytes.
- Fresh v0.2.0 log showed RTSP RECORD, 23,040 successful RTP decryptions, ALAC frame decode, PCM submission, occupancy 12,602/16,384, and no sampled ALAC failure or PCM underflow/drop event in the retained post-wrap segment.
- Confirmed user-reported START/PS failure. webMAN exit returned HTTP 200; log recorded `SYSUTIL_EXIT_GAME`, resource cleanup, and `APP_STOP`; status then showed XMB.
- The complete Lima ASan/UBSan host suite and PPU component compile passed.
- The standalone application compiled and linked with the new poller and callback instrumentation using `-Wall -Wextra -Werror`; package build is pending source commit.

## Immediate results

The v0.2.0 artifact was installed and launched. The run proved the receiver reached the PCM submission path while a song was streamed, but controller support failed by user report. The new diagnostic fields target the missing evidence.

## Known limitations / follow-ups

- Compile the updated standalone application, run the Lima suite, package v0.2.1 / SFO 00.41, and install/launch it for a focused START/PS test.
- User-visible display and subjective audio quality were not confirmed in v0.2.0.
- RTP/PCM measures are interval data from a wrapped capped log, not full-session totals.

## Risks introduced

The added state-transition logging is rate-limited by changes and errors but could still create events for every distinct button transition. It records only an eight-bit control mask and pad port index, not device identifiers. No boot plugin or firmware settings are changed.

## Decision record

### Decision: Add diagnostics before changing button semantics

- **Context:** Existing poller calls the PSL1GHT START bitfield, but the failed hardware report lacks evidence that button data reaches the process.
- **Options considered:** Guess at controller/PS callback behavior and change controls; log bounded transitions and system callback names first.
- **Why chosen:** The diagnostic distinguishes poll failure, absent pad, no button event, and callback delivery with minimal runtime behavior change.
- **Tradeoffs accepted:** The next build may still reproduce the input issue; it is designed to localize it rather than claim a fix.

## Next recommended step

Commit and push this pass after the privacy review, then build and validate AIRPLAY02 0.2.1 / SFO 00.41 from that exact commit, deploy by the authorized FTP/webMAN route, and test START and PS menu callbacks while streaming briefly.
