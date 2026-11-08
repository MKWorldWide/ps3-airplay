# AIR PS3 v0.2.8 reliability, XMB and branding candidate

**Source baseline:** `baseline-v0.2.7` / AIRPLAY02 v0.2.7 / SFO `00.47`

**Candidate:** v0.2.8 / SFO `00.48`; XMB title `AIR PS3`; RAOP friendly suffix `AirPS3`

**Status:** exact-source Linux release gate passed; candidate package retained locally
**Hardware status:** package installation on the development unit and a later 10m41s stream-log summary are recorded below. The source/build pass itself did not make system changes. The primary test unit was not touched in this documentation update.

## Forensic input and confidence

The supplied mission summary reports a main-console event with an RTSP OPTIONS stall of about 5.435 seconds, a similarly long audio-service gap, subsequent 256/256 RTP queue occupancy, queue drops, concealment, and PCM underflow. That summary supports a control/audio starvation correlation. The exact raw trace was not identified among the retained local v0.2.7 logs during this source pass, so these figures are recorded as user-supplied evidence, not independently recomputed measurements.

The cause is not established. Synchronous `fwrite` plus `fflush` on each event is a concrete blocking mechanism in control-path diagnostics and could worsen shared-loop starvation, but timing coincidence alone does not prove disk I/O was the only cause. The candidate isolates formatting/enqueue from filesystem writes and records RTSP parse, handler, send, log queue, write, and flush timings.

## Changes

- Added a fixed 128-record SPSC log queue. On PS3, a lowest-priority PPU writer batches file writes and flushes. The audio/control producer does not perform disk I/O while the worker is active. The 5 MiB per-launch cap and append-only file behavior remain.
- Queue overrun, queue depth, and maximum write/flush time are included in periodic diagnostics. Shutdown asks the writer to drain for at most 500 ms; if the writer remains blocked, the process avoids racing `fclose` against it and the final buffered tail may be lost. A failed log sink cannot report its own failure into that same file.
- Added microsecond RTSP parse, handler, and response-send duration events. Existing millisecond handler timing remains for continuity.
- Added conditional RTP live-edge recovery: only a full 256-packet queue with more than 128 packets of lead over the last consumed sequence triggers it. Stale queued frames and queued audio datagrams are dropped, pending resend state is reset, and the app clears PCM/resampler history and re-enters its existing prebuffer gate. Drops and resyncs remain counted.
- Converted the user-supplied artwork into a lossless PNG of the same decoded RGB pixels with embedded source metadata removed. Added checked direct-resize derivatives: `ICON0.PNG` 320x176 and `PIC1.PNG` 1920x1080. The in-app RGB background is generated from the same source.
- Changed only the XMB title and RAOP instance suffix. `AIRPLAY02`, the NPDRM content identity, the install location, and the audio format path remain unchanged.

## Recovery regression

The host queue test verifies that backlogs through 126 packets (about one second at the 352-frame/44.1 kHz sender rate) do not force resynchronization. A simulated 625-packet backlog (about five seconds) produces two resynchronizations, counts 512 stale frames as drops, and leaves 113 current frames queued. The next popped frame is at the new live edge; concealment does not walk the obsolete sequence range.

These tests model packet-sequence backlog; they do not reproduce PS3 scheduler timing, UDP socket behavior, ALAC output, or audible recovery. Additional burst/reorder/wrap/FLUSH/TEARDOWN/reconnect tests remain part of the release gate and hardware retest.

## XMB composition boundary

The app controls its framebuffer, while GameOS/VSH controls the PS-button system-menu composition. Existing menu/draw callbacks and paced refresh behavior are preserved. This pass does not draw over VSH or add an unverified dimming mode. Whether the supplied image remains visible under the system menu and whether resume avoids the dim/frozen loading display require a physical test; package creation alone cannot establish either result.

## Artwork checksums

The committed artwork manifest records SHA-256 values for the metadata-free source pixel image and both derivatives. The validation script decodes each file, verifies dimensions, and compares derivative pixels with a direct Lanczos resize from the source. Package extraction must compare equal byte-for-byte with both committed PNG assets.

## Build and package results

The first exact-source Lima gate completed protocol/audio and PPU component checks, then stopped compiling the host logger because strict Ubuntu C mode did not expose `clock_gettime`. A host-only `_POSIX_C_SOURCE=200809L` definition was added before system headers; the PS3 `sysGetSystemTime()` path is unchanged. The corrected commit completed the full release gate and was independently rechecked after copying the artifacts back to the Mac.

- Source revision: `9f7af0027b7c3e9470a1583bbfbf676f9525f72d`
- Toolchain: existing Lima Ubuntu 24.04 x86_64 guest; PS3DK v0.27.0, PSL1GHT, and the pinned Mbed TLS dependency.
- CellOS PPU ELF: 2,755,120 bytes; SHA-256 `106ee5b6ac445583924fbc0e5bd136aa45b74f726b1693a2b45c84deb00d6868`; validated as PPC64 CellOS executable with five load segments.
- PKG: `ps3-airplay-receiver-0.2.8.pkg`, 3,906,640 bytes; SHA-256 `e70d957de4a0fe2b0f8a7bd3a87f63cabaab2f12726e6341508a5f9b8c5de383`; debug package with six items.
- SFO: `APP_VER=00.48`, `VERSION=00.48`, `TITLE_ID=AIRPLAY02`, `TITLE=AIR PS3`; all launch-metadata checks passed. Existing content ID remains `UP0001-AIRPLAY02_00-0000000000000000`.
- Extracted SELF/EBOOT: 494,288 bytes; SHA-256 `a45223dd996c0cb14522d8803a070acbdc2d0349b7fb78836d84d2c42bf58103`.
- Extracted `ICON0.PNG`, `PIC1.PNG`, and generated `background.rgb` byte-match the checked-in/generator outputs. SHA-256 values are respectively `ffcabffe7550599c67a1cbdb4a1e7bcd8d8866fd83813a5f78e9886975234183`, `1dca1304d818df4275f13d7efb6401d5c7e4d520c72d92ac1bd4dc496fdf964c`, and `602d7df8352a816a27cb1152a18f5062ff25e709c31bf564787b0da02980efde`.
- Host ASan/UBSan and protocol/audio regressions passed, including deterministic ALAC mutation smoke, 1,416-byte encrypted ALAC exact PCM, FFmpeg PCM comparison, RTP sequence wrap/loss/reorder/retry/flush cases, 600-second sample-ratio conservation, 10,000-packet jitter simulation, RTSP/RTP loopback and bounded slow-reader disconnect, logger tests, and display-resume state-machine mocks.
- Host-only cost probe (Lima x86_64 emulation): logger write/flush mean 12,281 ns, p99 40,999 ns, max 515,987 ns; 256-frame SRC mean 324,561 ns, p99 362,991 ns, max 1,011,976 ns against a nominal 5.333 ms block. These are host probe measurements, not PS3 timing guarantees.
- ELF, SFO, package-envelope, package extraction, and artwork validators all passed. The original failed run is documented above; the full corrected run completed successfully.

## Acceptance boundary

| Check | Result |
|---|---|
| Host RTP queue delay/recovery test | PASS |
| Mac host forensic/regression script | PASS; Linux-only resend-policy reproduction skipped on macOS |
| Exact PS3 PPU and package gate | PASS on pinned Lima source `9f7af00`; local artifact hashes and extracted assets independently verified |
| Installed version identity | PASS: FTP download-back matched package; installed SFO/EBOOT matched package installation payload |
| Physical display / PS-menu resume / START exit | Not established by the supplied v0.2.8 log summary; it has no menu/resume events |
| AirPlay discovery and RTSP | PARTIAL: active stream summary and 330 OPTIONS/200; one setup; detailed stage boundary must be interpreted from fresh logs |
| RTP reception / decryption / ALAC / PCM counters | PASS to reported active-stream counters: 69,632 RTP packets; 17 gaps all recovered; zero reported queue drops/concealment/skips/PCM underflows/ALAC failures |
| Audible music quality / 30-minute stability | Not established by the summarized log; 10m41s is shorter than the target and requires user listening confirmation |

## Development-unit v0.2.8 hardware log summary

The following is transcribed from the user's supplied sanitized summary of a read-only FTP log capture; the raw log is not committed. The active stream spans 07:05:05Z–07:15:46Z (10m41s). The summary reports 69,632 received RTP packets, 17 missing-packet observations all recovered, no queue drops, concealed/skipped frames, PCM underflows, or ALAC failures; RTP queue depth 79–102/256; PCM occupancy 12,056–12,635/16,384; maximum audio-service interval 15.33 ms; maximum render time 1.754 ms; 330 RTSP OPTIONS responses with status 200; one stream setup; 18 AirPlay volume updates; and one FLUSH.

Repeated rejected RTP timing-packet diagnostics and one accepted timing reply remain to investigate. The summary reports no audio loss alongside them. It contains no XMB menu-open/close or resume events, so it says nothing about display appearance or VSH composition. The packet, queue, and PCM metrics are diagnostic counters; no independent listening measurement is attached to this capture.

Keep these prior v0.2.7 comparisons separate from the v0.2.8 development-unit sample: the development-unit 30-minute run reported 26,814 PCM underflow frames, maximum service interval 35.842 ms, and queue occupancy averaging about 136/256 with peak 177. A different primary test unit's later 17m05s run after AirPlay reconnection reported no queue drops, concealment, underflows, or ALAC failures and 16.79 ms maximum service interval. That primary unit also had an earlier ~5.44-second RTSP/audio-service stall with full RTP queue, drops, concealment, and PCM underflows; a newly started AirPlay session restored operation, but the cause and within-session recovery remain unproven.

## Rollback and next actions

Keep the v0.2.7 artifact and `baseline-v0.2.7` tag untouched. v0.2.8 is installed on the development unit and the summarized log shows active receiver traffic, but it has not met stable-release criteria. Next hardware pass: confirm exact visible XMB/app artwork, PS-menu open/close/resume twice, START exit, user-audible playback, and then a 30-minute run with fresh log retrieval. If queue resync occurs, measure its count and time to stable occupancy; if the logger queue overruns or writer fails, preserve the exact private log tail and avoid claiming complete instrumentation.
