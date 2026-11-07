# v0.1.13 build and deployment readiness

**Date:** 2026-11-07
**Source revision:** 70fa6654bd2d092a5f674dd526c05a2e5ccd4703
**Package:** AIRPLAY02 v0.1.13, RTSP TCP 5000
**PKG size:** 3,224,944 bytes
**PKG SHA-256:** f7d076e32520a552349ebb973894409d66fd429ff255d0919c66401fabe9f1e9
**ELF SHA-256:** 8d52e871e437359b634f6d9a67d485346736251e16ce84ff09df24d43d528809

## Scope

Rebuilt the current receiver from the pushed main source on the existing Lima guest using the canonical PS3DK v0.27.0 PPU pipeline. This produced a versioned package that includes the simplified background screen and privacy-safe mDNS bind-stage diagnostics.

## Validation

- bash scripts/build-lima-crypto.sh: PASS. RAOP crypto/challenge, session crypto, RTSP session, SDP, mDNS serialization, persistent RTSP/RTP loopback, capped RTSP logging, ALAC decode, PCM ring/resampling, diagnostic tone tests all passed.
- PPU objects for crypto, session, receiver, ALAC, and PCM built successfully.
- bash scripts/build-lima-standalone-receiver.sh: PASS. CellOS PPC64 ELF validation passed; 2,758,392-byte ELF.
- bash scripts/package-lima-standalone-receiver.sh: PASS. NPDRM EBOOT generation, package listing/extraction, SFO version/title ID, content ID, icon/background equality, relative entry paths, and finalized package envelope validated.
- The freshly cloned Lima workspace did not have Pillow. The packaged RGB background was reused from the prior guest build only after the JPEG source hashes matched and the RGB output compared byte-for-byte.

## Hardware and emulator status

RPCS3 install/launch was not performed in this pass. The interactive UI check remains pending. No launch result is inferred from build validation.

The PS3 was not reachable at its last known address during the read-only connectivity check, so no package transfer, installation, launch, or log retrieval was performed. No console files or settings were changed.

| Acceptance area | Result |
|---|---|
| Host protocol and audio tests | PASS |
| PPU object compile | PASS |
| ELF and NPDRM PKG structure | PASS |
| RPCS3 install / launch | NOT TESTED |
| Physical display / START exit | NOT TESTED |
| Native mDNS bind / iPhone discovery | NOT TESTED |
| RTSP / RTP / ALAC / PCM | NOT TESTED |
| Audible AirPlay or Spotify | NOT TESTED |

## Next action

When the desktop is available, install and launch this exact package in RPCS3, inspect its fresh log, and verify the screen/exit path. Once the PS3 is reachable, stage the same validated package and test native UDP 5353/mDNS first. Continue to RTSP, RTP, decode, and audible output only when each previous boundary is evidenced.
