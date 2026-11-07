# RPCS3 v0.1.28 ANNOUNCE Replay and Diagnostic Logger Limit

**Timestamp:** 2026-11-07 13:10 EDT (America/New_York)

## Test identity

- **Test ID:** RPCS3-ANNOUNCE-28
- **Source revision:** `4308bb120d0b86e065481a1175990de47acb4a24`
- **Package:** AIRPLAY02 v0.1.28, 3,231,984 bytes
- **Package SHA-256:** `0e3201647bb83b85c0df4e08dacc780c9daf1468c9d261e321c10be0f8fa9cef`
- **Emulator configuration:** RTSP 5001; RTP ports 6000–6002

## Observed result

| Stage | Result | Evidence |
|---|---|---|
| Package/build | PASS | CellOS PPC64 ELF, SFO/title/content ID, assets, extraction, and finalized NPDRM checks passed. Full Lima host/ASan/UBSan and PPU component checks passed. |
| RPCS3 startup / listeners | PASS | Fresh log records app startup, route, RTSP 5001, and RTP 6000–6002. |
| OPTIONS | PASS | Synthetic sender received 200 OK. |
| ANNOUNCE SDP / fields | PASS | SDP parse completed; validation start and offer flags value 1539 (encryption type 1, RSA key, AES IV) were logged. |
| ANNOUNCE response | BLOCKED | Sender received no reply. The short `ANNOUNCE_VALIDATE_DONE` event was absent. Content-Type, RSA-start, crypto-input, and response event names exceeded the logger’s 24-character limit and were silently rejected, so their absence is inconclusive. |
| SETUP/RECORD/RTP | NOT REACHED | Probe timed out at ANNOUNCE and sent no media packets. |
| Display / audio / physical PS3 | NOT VERIFIED | Desktop CUA remained locked. No visible pixels, audible tone/music, iPhone session, or physical console result is claimed. |

The prior local AIRPLAY02 title data was preserved before staging the validated extracted payload. Raw logs remain local and ignored; physical console files/settings and boot plugins were unchanged.

## Diagnostic correction

The logger's event-name validator allowed at most 24 characters, but the new Content-Type/RSA/response names ranged from 25 to 33 characters. `receiver_log_file_event` returned failure and the session callback ignored it. Therefore, absence of those long event names cannot be used to infer which ANNOUNCE operation ran. The missing short validation-completion event remains evidence that that callback was not reached.

## Conclusion and next action

The exact ANNOUNCE stall remains unknown. v0.1.29 raises the event-name limit, enlarges the formatted-line buffer, and adds a persistent-log regression assertion for a 33-character event. Replay the same sender only after confirming the long events appear in the log.
