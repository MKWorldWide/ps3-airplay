# RPCS3 v0.1.27 ANNOUNCE Validation Replay

**Timestamp:** 2026-11-07 13:02 EDT (America/New_York)

## Test identity

- **Test ID:** RPCS3-ANNOUNCE-27
- **Source revision:** `eebd588f4d4e7baa8da95d5f4127b9155d90b858`
- **Package:** AIRPLAY02 v0.1.27, 3,231,920 bytes
- **Package SHA-256:** `0a3d1c409fb1044231256090cc3693da3e1aaa5abddd5bc6e00f29a9aa407b6c`
- **Emulator configuration:** RTSP 5001; RTP ports 6000–6002

## Observed result

| Stage | Result | Evidence |
|---|---|---|
| Build/package | PASS | CellOS PPC64 ELF, APP_VER/SFO, title/content ID, extracted assets, and finalized NPDRM envelope validated. |
| RPCS3 startup/network | PASS | Fresh log records APP_START, NetCtl route, RTSP and RTP listeners. |
| mDNS | FAIL | `MDNS_FAILED` stage 6, error 125. Discovery remains unavailable in this emulator environment. |
| OPTIONS | PASS | Synthetic sender received 200 OK. |
| ANNOUNCE SDP | PASS | Parser completion event returned value 1 for 508 SDP bytes. |
| ANNOUNCE offer fields | PASS | Encoded field flags value 1539 identifies encryption type 1, RSA key present, and AES IV present; wrapped key data length is 344 bytes. |
| ANNOUNCE validation / response | BLOCKED | No following Content-Type, validation-completion, RSA-start, or RTSP-status event appeared. The sender timed out waiting for the response. The exact stalled expression is unknown. |
| RTP / audio | NOT REACHED | No SETUP/RECORD or audio packets were sent. |
| Display / sound / hardware | NOT VERIFIED | Desktop CUA was locked. No screen, audible tone, iPhone, or physical PS3 result is claimed. |

The package payload was staged in local RPCS3 title data after preserving the previous title data. The raw capped log remains local under ignored `work/`; no console or boot-plugin changes were made.

## Validation

The full Lima host/ASan/UBSan suite and PPU object compilation passed for the v0.1.27 source. The final package passed ELF, SFO, content ID, extraction, asset, and NPDRM validation.

## Conclusion and next action

The stall occurs after offer parsing/classification and before ANNOUNCE validation completion. v0.1.28 changes the fixed-size Content-Type check to use a bounded length scan and adds events around that operation. This is a defensive improvement and a test hypothesis; a fresh RPCS3 replay must determine whether it resolves or further narrows the stall.
