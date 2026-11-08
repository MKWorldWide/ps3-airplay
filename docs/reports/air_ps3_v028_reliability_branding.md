# AIR PS3 v0.2.8 reliability, XMB and branding candidate

**Source baseline:** `baseline-v0.2.7` / AIRPLAY02 v0.2.7 / SFO `00.47`

**Candidate:** v0.2.8 / SFO `00.48`; XMB title `AIR PS3`; RAOP friendly suffix `AirPS3`

**Status:** source implemented; Lima release gate pending
**Hardware:** no PS3 was accessed, installed, restarted, or modified for this candidate.

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

Pending exact-source Lima release-gate run:

- Source revision: pending
- PPU ELF size/SHA-256: pending
- PKG filename/size/SHA-256: pending
- SFO APP_VER/VERSION: pending validation of `00.48`
- Title ID/content ID continuity: pending
- Extracted EBOOT/ICON0/PIC1/runtime background: pending
- Host ASan/UBSan, PPU, ELF, SFO and package-envelope checks: pending

## Acceptance boundary

| Check | Result |
|---|---|
| Host RTP queue delay/recovery test | PASS |
| Mac host forensic/regression script | PASS; Linux-only resend-policy reproduction skipped on macOS |
| Exact PS3 PPU and package gate | Pending |
| Installed version / physical display / START exit | Not tested |
| AirPlay discovery and RTSP | Not tested |
| RTP reception, decryption, ALAC, PCM output | Not tested |
| Audible music / 30-minute stability | Not tested |

## Rollback and next actions

Keep the v0.2.7 artifact and `baseline-v0.2.7` tag untouched. This candidate is not deployed. After the Lima gate passes, perform an authorized hardware test as a separate operation: confirm XMB title/art, visible/responsive display, START exit, one AirPlay connection, fresh verbose log, and audible audio. If queue resync occurs, verify its count and time to stable occupancy; if the logger queue overruns or writer fails, preserve the exact log tail and avoid claiming that instrumentation is complete.
