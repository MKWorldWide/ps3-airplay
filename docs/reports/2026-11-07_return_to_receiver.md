# Return to AIRPLAY02: local build and validation

**Date:** 2026-11-07 (EDT)

## Summary

The maintainer reports that an earlier AIRPLAY02 run reached application startup and played the five-second diagnostic tone. Its exact package version is not established. The separate AIRTEST01 v0.1.0 app was a silent launch-control diagnostic, not an AirPlay receiver update; the user reports it also returned XMB error `80028F21`. AIRTEST01 remains a separate installed title and is parked.

This pass resumed AIRPLAY02 and restored its earlier route-based local IPv4 lookup as the primary method. If that lookup fails, the app waits at most five seconds for NetCtl and retries the route lookup once. Version logging now comes from the same build input as package SFO version. The change addresses a possible delayed-network-startup failure; it is not a proposed explanation for the XMB launch error. The candidate is not yet cleared for a hardware launch because inherited RSX waits are unbounded and controller initialization follows graphics setup.

## Source and artifacts

- Source branch: `docs/v012-preflight-review`; implementation commit to be recorded after privacy review.
- Package: AIRPLAY02 v0.1.4 candidate.
- Local PKG: 3,183,584 bytes; SHA-256 `0c1e3c74eea71f7b5829201673f4dd5211bcbf704ab00cb55e1b41cb87020ee4`.
- ELF: big-endian ELF64 PowerPC64; SHA-256 `6e2352c5f7e3cb65f1cf9615606d790d60a02a3cc6bd36bb4e09639a8fea65fc`.
- Packaged EBOOT SHA-256: `639115c46db122e28d88db27ece84e1a85cc5ac2f6943b55a2afd99315d6ccff`.
- SFO: `APP_VER=0.1.4`, `TITLE_ID=AIRPLAY02`; package and SFO content IDs agree.
- Runtime: PSL1GHT commit `6e565a70e927f55813babe86dc6f64149535f908` with GCC 7.2 and matching Mbed TLS 3.6.7 build.
- Hardware: no transfer/install/launch in this pass. Boot plugin settings unchanged.

## Validation results

| Layer | Result | Evidence |
|---|---|---|
| Crypto/session/SDP/mDNS host checks | PASS | Four test groups completed before receiver loopback test |
| Receiver timing-probe loopback | FAIL / unresolved | Direct test stalled at timing-probe wait and was interrupted; prior trace-assisted result is not counted as a direct pass |
| RTSP listener and capped log | PASS | Targeted host test |
| ALAC decode | PASS | Reference PCM comparison, 353 samples |
| PCM ring / conversion / resampling | PASS | Targeted host test |
| Diagnostic tone | PASS, host only | Bounded five-second 440 Hz tone test |
| PPU build | PASS | Full standalone receiver ELF linked under the legacy runtime |
| PKG structure and extraction | PASS | SFO, title ID, content ID, EBOOT, icon, and RGB asset checks |
| PS3 launch/display/exit | NOT TESTED | Candidate not transferred or installed |
| iPhone discovery/RTSP/RTP/audio | NOT TESTED | No hardware sender attempt |

The [`PS3 Developer Wiki error-code reference`](https://www.psdevwiki.com/ps3/Error_Codes) reviewed in this pass has no `80028F21` entry. It is not possible to derive a specific loader diagnosis from that code alone.

## Earlier hardware evidence and current boundary

The maintainer confirms the earlier AIRPLAY02 run did start and play the five-second tone. A saved earlier log contains app/audio startup and tone-completion events, but lacks a package-version marker and clean user-exit evidence. The user also previously observed a black display and an iPhone receiver listing followed by connection failure. These observations do not establish that v0.1.4 displays correctly or completes an AirPlay session.

AIRPLAY02 v0.1.2 and v0.1.3 were reported to fail at XMB with `80028F21`; the existing receiver log remained unchanged. AIRTEST01 v0.1.0 is a separate title; read-only retrieval found package-matching installed files, but its fresh diagnostic log was unavailable. No cause for these launch failures is established.

## Next step

First bound the RSX flip/label waits and preserve early controller exit, then compare the exact v0.1.4 package against the preserved earlier AIRPLAY02 candidate. After the user resumes the paused goal, install the validated candidate under the standing authorization and test XMB launch, visible display, diagnostic tone, START-to-XMB, and fresh log. Only after a clean exit should the iPhone connection stage be attempted. Do not change boot plugin configuration; do not proceed into extended tests if the display or exit hangs.
