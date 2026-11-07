# v0.1.2 review and first hardware-test preflight

**Timestamp:** 2026-11-07 02:42 EDT
**Source revision:** `a6b36cd43fe289b63549cbd06b888f984fe87936`
**Purpose:** Revalidate the existing candidate and prepare the first display/exit test without changing the console.

## Artifact verification

- Candidate: `build/lima-0.1.2/ps3-airplay-receiver-0.1.2.pkg`
- Version / application ID: `0.1.2` / `AIRPLAY02` (verified in the package extraction record)
- Size: 3,225,712 bytes
- SHA-256: `923aebf6eca5693b21b494a9a4225c36629f13e8b316b2825d5deb3763226ebf`
- Existing extraction checks report a big-endian PowerPC64 ELF input, matching NPDRM EBOOT, expected SFO version/title ID, icon, and 2,764,800-byte RGB background.
- No package rebuild or console transfer was performed. The previous transfer authorization was for a different hash and does not apply here.

## Hardware and host evidence

| Gate | Result | Evidence |
|---|---|---|
| Latest source | PASS | Fetched `origin/main`; local `main` and remote main match the source revision above. |
| Candidate hash/size | PASS | Recomputed SHA-256 and file size. |
| Package structure | PASS, host-side | Existing fresh extraction record matches SFO, EBOOT, icon, background, and content identity. |
| PS3 reachability | BLOCKED | Last known endpoint did not answer ping or FTP. No console request that writes data was made. |
| Disk, staged PKG, installed version, log | UNKNOWN | FTP preflight could not reach the console. |
| Receiver loopback | UNRESOLVED | Direct run timed out during the timing-probe receive; a trace-assisted run completed the timing exchange and full loopback scenario. Do not count the instrumented run as an ordinary pass. |
| Display/START exit | NOT TESTED for v0.1.2 | No new hardware observation tied to this package. |
| iPhone RTSP/RTP/audio | NOT TESTED for v0.1.2 | No new iPhone attempt or corresponding log. |

## Display and exit review

The source draws the `DISPLAY ACTIVE` banner, color bars, background, and START prompt, but the screen code contains software waits without an overall deadline: the RSX backend label wait and the two flip-status waits. The controller is initialized only after the display setup function returns. PSL1GHT documents polling `gcmGetFlipStatus()` after `gcmSetWaitFlip()` with a sleep, but that example does not establish that these waits are safe when RSX progress stalls. A timeout-only screen correction is the smallest plausible safety change; it would require a new versioned package and its own hash, while retaining normal cleanup and treating display failure as a logged error.

`SCREEN_READY` means initialization reached the code's ready path. It does not prove that pixels were visible on HDMI. For the first hardware run, record independently whether the background, blue banner, color bars, and START prompt are visible; whether the tone plays; whether the image remains stable after the tone; and whether START returns to XMB. Do not continue to an iPhone test unless the app remains responsive and exits normally.

## Logging review

The source has privacy-safe events for package startup, display/controller/network initialization, listener and UDP readiness, mDNS/audio readiness, RTSP accepts/methods/statuses/state changes, negotiated SDP values, batched RTP, decrypt failures, ALAC decode frames/failures, prebuffer readiness, PCM underflows/drops, user exit, and app stop. Identified gaps for a future targeted diagnostics pass are successful decryption counters, PCM frames actually rendered, and periodic buffer occupancy/output counters. Do not add peer addresses, raw SDP, keys, audio, track metadata, or device identifiers.

## Decision and next action

Do not transfer or install while the console is unreachable. First restore connectivity and complete read-only checks for free space, the staged package, installed AIRPLAY02 version, current log, and recovery procedure. Then present the exact filename/version/hash/destination and rollback steps for approval. Given the unbounded RSX waits and prior forced shutdown, review a bounded-wait safety fix before asking the user to repeat a launch; do not describe that change as a display fix until the user sees pixels and exits to XMB twice.

The iPhone's earlier receiver-list entry remains discovery evidence only. It does not prove TCP reachability, RTSP negotiation, RTP, ALAC, or audible streamed audio. No Spotify playback, background mixing, or game audio compatibility is claimed.
