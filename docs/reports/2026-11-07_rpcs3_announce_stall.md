# RPCS3 Synthetic ANNOUNCE Stall

**Timestamp:** 2026-11-07 12:37 EST (America/New_York)

## Test identity

- **Test ID:** RPCS3-ANNOUNCE-25
- **Package:** AIRPLAY02 v0.1.25, 3,231,152 bytes
- **Package SHA-256:** `420971af3601d0a9813d4853f389f7e4ceb1c9f4a6077e27fcc3b81b292cbcd4`
- **Source revision:** `a59aa937401d432b5cc6f740eeda15bf0db091ba`
- **Build:** PS3DK v0.27.0 PPU + PSL1GHT; emulator-only RTSP port 5001; RTP base port 6000.

## Observed result

| Stage | Result | Evidence |
|---|---|---|
| RPCS3 app startup | PASS | Fresh log identified AIRPLAY02 v0.1.25 and showed NetCtl route readiness. |
| RTSP/RTP listeners | PASS | RTSP 5001 and UDP 6000–6002 opened. |
| OPTIONS | PASS | Synthetic sender received `RTSP/1.0 200 OK`; log recorded connect, OPTIONS, and status 200. |
| ANNOUNCE | BLOCKED | Log recorded `RTSP_ANNOUNCE` for a complete 613-byte request, but the sender received no response within three seconds. No later status, parse error, key error, or state event appeared before the emulator was stopped. |
| SETUP/RECORD/RTP | NOT REACHED | Sender timed out while waiting for ANNOUNCE and sent no RTP packets. |
| mDNS | FAIL | RPCS3 multicast-interface setup still failed at stage 6 / error 125. |
| Physical PS3 / audible output | NOT TESTED | No console files/settings or boot plugins were changed; no sound was observed. |

The original capped app log is preserved locally with SHA-256 `b7a50d833e7326de9f1e9b35d309299b4eee4b11045b2fbac6bb1370c7e265f8`. It is not included in this public report. The exact blocked operation is unknown: the method event occurs before the session handler, so this does not prove RSA is the cause.

## Implementation and host validation

To make the next run distinguish the synchronous handler stages, v0.1.26 source adds privacy-safe `ANNOUNCE_SDP_PARSE_START/DONE` and `ANNOUNCE_KEY_UNWRAP_START/DONE` events behind an optional session diagnostic callback. The host crypto/session test asserts the event order and successful key unwrap. `raop_audio_pipeline_test --remote <host> <port>` now replays the fixture using OPTIONS, ANNOUNCE, SETUP, RECORD, 128 encrypted RTP packets, and TEARDOWN.

Full Lima host tests pass under ASan/UBSan, including encrypted RTP-to-PCM, 4,096 seeded ALAC mutations plus 256 random packets, the remote-probe build, and PPU component compilation. The package and RPCS3 replay for v0.1.26 are pending.

The suite also exposed a test-only false failure: `raop_receiver_test` read a 16 KiB capped log into a 4 KiB buffer, truncating the slow-reader timeout event. Its buffer now uses `RECEIVER_LOG_MAX_BYTES + 1`, and the full suite passes.

## Conclusion and next action

v0.1.25 establishes the TCP/RTSP request path through OPTIONS but stalls on synthetic ANNOUNCE in RPCS3. Build and run v0.1.26 with `RTSP_PORT=5001`, then inspect whether SDP parse and key unwrap each start and complete. If the key unwrap starts without completion, investigate the PS3 crypto/RNG operation under RPCS3; if it completes, continue to the response and RTP events. Do not attribute this emulator result to physical PS3 until repeated on hardware.
