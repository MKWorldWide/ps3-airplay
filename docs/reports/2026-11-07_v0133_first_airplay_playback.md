# v0.1.33 first confirmed hardware AirPlay playback

## Test identity

- Application: AIRPLAY02 v0.1.33; PS3 SFO version 00.33.
- Source revision: `7352a77cbaa7d13d57ad93a2788bdc0ba26e4fd2`.
- PKG SHA-256: `aa3b456caa983b85b26fa63a43289b2a301a262a2d9d5f8748783897ddea79f5`; package size 3,235,056 bytes.
- Log: locally preserved in the maintainer's private test workspace. The sanitized report records the original 9,973-byte log hash `350ad4ab8e614b91ddd0c15e65bfe38eb37ce5286565fc52056e3e6147b29a3c`; raw log is not included in this public repository.
- Last log timestamp: 2026-11-07 19:46:06 UTC.

## User observations

The maintainer reports multiple songs streamed from iPhone Spotify to the PS3 through AirPlay. Music was audible only after the XMB menu had been opened and the application resumed; the screen then flickered. Audio was audible but crackly, unstable, and low fidelity. The image was sometimes stable on a fresh launch and flickered after XMB resume. The PS button returned to XMB; the app was subsequently quit normally. The first launch produced one short tone; later tone components were not heard.

The v0.1.33 diagnostic implementation contains one five-second, 440 Hz test tone at a conservative 5% peak level. It does not contain a second or low-frequency tone. This is the expected behavior of this artifact, though the observed shortened duration remains unexplained.

## Sanitized log findings

The captured capped log had wrapped, so it no longer contains application startup or initial RTSP negotiation. Its retained portion establishes active streaming but cannot reconstruct the whole session.

- 44.1 kHz, stereo stream; encryption active.
- RTP audio arrived in 128-packet reporting intervals. Fourteen visible intervals account for at least 1,792 packets in the retained window.
- The visible `RTP_MISSING_BATCH` deltas sum to 106 packet sequence gaps. Some retransmissions were recovered; the sparse event cadence does not expose the exact total in this wrapped log.
- ALAC frames of 352 samples decoded. Periodic `ALAC_FAIL_TOTAL` values were zero.
- PCM queued and consumed counters advanced by about 220,000 frames per five-second report; the PCM ring held roughly 12,000 frames. GameOS audio blocks were submitted. This proves the software pipeline produced output frames, not that every frame reached HDMI cleanly.
- No queue-drop or ALAC-decode-failure event appears in the retained portion. The log does not establish the exact number of audible dropouts or their cause.
- The log records menu/draw callbacks, RTSP TEARDOWN, and complete cleanup ending in `APP_STOP value=0`. No `SCREEN_PRESENT_FAILED` is retained.

## Result by gate

| Gate | Result |
|---|---|
| Application launch and visible background | PASS by maintainer report; image stability is inconsistent |
| PS button opens XMB | PASS by maintainer report |
| Resume after XMB | FAIL: frozen loading screen was observed earlier; later resume runs flicker |
| Controller/XMB quit | PASS in this run; log ends with clean cleanup |
| AirPlay discovery and RTSP | PASS by user report and retained RTSP/RTP activity |
| RTP reception, decryption, ALAC decode, PCM generation | PASS for the captured active segment |
| Audible real music | PASS: first user-confirmed AirPlay music, multiple songs |
| Stable, clean audio | FAIL: crackle, instability, low fidelity |
| 30-minute playback | NOT TESTED |

## Assessment and next action

This is the first confirmed end-to-end music playback, but not a stable MVP. The strongest measurable lead for crackling is repeated RTP sequence gaps combined with a resend scheduler that can discard a gap when the 50 ms request throttle is active and tracks only one outstanding resend range. This is a code defect and a plausible contributor; the wrapped log does not prove it caused the audible artifacts.

v0.1.34 adds a bounded pending resend queue, tracks up to 16 concurrent resend ranges for one second, and reports interval-level loss/recovery totals. The host test simulates two close gaps and delayed responses. Physical improvement and the XMB flicker remain to be tested independently.
