# RPCS3 v0.1.26 ANNOUNCE Phase Replay

**Timestamp:** 2026-11-07 12:50 EDT (America/New_York)

## Test identity

- **Test ID:** RPCS3-ANNOUNCE-26
- **Source revision:** `6f8d6cb6f75e331d989419aaf51bd1d147704cad`
- **Package:** AIRPLAY02 v0.1.26, 3,231,408 bytes
- **Package SHA-256:** `82e1407b92a1218ec3085f57bd8f6c35e97255569b92a44249e14239108fc246`
- **Emulator configuration:** RPCS3-only RTSP port 5001; RTP ports 6000–6002

## Observed result

| Stage | Result | Evidence |
|---|---|---|
| Package structure | PASS | CellOS PPC64 ELF, APP_VER 0.1.26, title/content ID, embedded image, extraction, and finalized NPDRM checks passed. |
| App startup / network | PASS | Fresh capped app log records APP_START, NetCtl route readiness, and RTSP/RTP listener readiness. |
| OPTIONS | PASS | Synthetic host sender received 200; app log records RTSP status 200. |
| ANNOUNCE parsing | PASS | App log records `ANNOUNCE_SDP_PARSE_START` followed by `ANNOUNCE_SDP_PARSE_DONE value=1`. |
| ANNOUNCE response | BLOCKED | Sender received no response before its three-second receive timeout. No RSA unwrap start or subsequent RTSP status was logged. |
| SETUP/RECORD/RTP | NOT REACHED | Sender aborted at ANNOUNCE and did not transmit media packets. |
| mDNS | FAIL | Prior RPCS3 run still fails multicast-interface setup; this replay did not establish discovery. |
| Display / audio / hardware | NOT VERIFIED | Desktop automation reports locked. No visible-pixel, audible-tone, iPhone, or physical PS3 result is claimed. |

The app log ended after SDP parse completion. The test harness stopped RPCS3 without an APP_STOP event. The exact blocking operation is unknown; these observations do not implicate RSA key unwrap.

## Validation

- Full Lima host suite under ASan/UBSan passed, including crypto/session events, RTSP state transitions, receiver loopback and bounded slow-reader behavior, encrypted RTP/decrypt/ALAC/PCM, 4,096 seeded ALAC mutations plus 256 random packets, PCM conversion/gain, and diagnostic-tone waveform checks.
- PPU component objects compiled and validated as PowerPC relocatable objects.
- The standalone v0.1.26 package was built from the exact source revision using the established Lima PS3DK v0.27.0 + PSL1GHT pipeline and emulator-only port 5001.
- The validated extracted title payload was copied to local RPCS3 title data after preserving the prior title data. It was not installed through the GUI package manager.

## Conclusion and next action

v0.1.26 confirms ANNOUNCE SDP parsing completes, but the request still does not receive a response. v0.1.27 adds privacy-safe checkpoints for offer-field classification, validation completion, and response construction. Replay those checkpoints before changing crypto, protocol semantics, or sender behavior. Physical PS3 and boot plugin configuration remain unchanged.
