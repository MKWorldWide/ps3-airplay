# v0.1.3 Legacy Runtime Candidate Report

**Date:** 2026-11-07 (EST)

**Base source revision:** `3dfa172` on `docs/v012-preflight-review`, plus the Makefile's opt-in legacy-runtime selection documented in this report.

**Hardware:** No console action was performed during this build pass.

## Why this candidate exists

AIRPLAY02 v0.1.2 is installed and its SFO/EBOOT hashes match the validated package. The user reports that XMB launch returns `80028F21`; the existing log did not change. The code's cause is not established.

The [PS3 Health project](https://github.com/lucasdaddiego/ps3-health) reports a similar pre-`main()` failure with PSL1GHT 2021+ runtimes on HFW 4.93 + PS3HEN 3.6.0, with its 2020-runtime build working. The author identifies runtime changes as suspects, not a fully isolated cause. This is a compatibility clue only: the PS3 AirPlay target uses Evilnat CFW, so hardware validation is still necessary.

## Candidate details

| Item | Value |
|---|---|
| Package | `ps3-airplay-receiver-0.1.3.pkg` |
| Application | AIRPLAY02 / APP_VER 0.1.3 |
| Package size | 3,183,408 bytes |
| Package SHA-256 | `98d8701ab018df882e30ba28f851a994c195d0a144dafe7c68c77d86d94126aa` |
| EBOOT size | 402,928 bytes |
| EBOOT SHA-256 | `1b7a1e0b2430fc91c6b2969e3d7b6e61c57aa945e58833b33d0ddc699f0b9b82` |
| PSL1GHT | `6e565a70e927f55813babe86dc6f64149535f908` (2020-11-25) |
| PPU compiler | GCC 7.2 from the checksum-verified Linux x86_64 PS3DEV bundle |
| PS3DEV bundle SHA-256 | `dcbed747e094c6a382dae5b0aacc322a1d4390d9f2849a7d750a96ea398ee8ab` |
| Package content ID | `UP0001-AIRPLAY02_00-0000000000000000` |

The package builder extracted the candidate and checked the SFO version/title ID, NPDRM EBOOT, package content ID, icon, and embedded 1280x720 RGB background. This is build/package validation only. The candidate is local and has not been transferred, installed, or launched on a PS3.

## Validation

Eleven existing host test executables passed, including protocol session/crypto, SDP, mDNS, receiver RTSP/RTP loopback, listener/logging, ALAC, PCM ring, tone, and an additional session test. Earlier untraced timing-probe flakiness remains unresolved despite this passing run.

The PPU receiver ELF compiled with the legacy runtime and matching Mbed TLS library. The result is big-endian ELF64 PowerPC64. No new package or hardware behavior is inferred from these checks.

The staged source/doc diff passed `git diff --cached --check` and a manual privacy review. `gitleaks` and `trufflehog` were unavailable in the host environment. Targeted searches found no local usernames/home paths, private network addresses, email addresses, secrets, or agent instruction files in the staged changes.

## Hardware results

| Check | Result |
|---|---|
| v0.1.2 package identity | PASS; installed SFO and EBOOT match the candidate |
| v0.1.2 XMB launch | FAIL; user reports `80028F21` |
| v0.1.3 package validation | PASS locally |
| v0.1.3 FTP transfer | PASS; staged under a versioned filename, downloaded back byte-for-byte with the expected size and SHA-256 |
| v0.1.3 XMB install/launch | NOT RUN; awaiting separate approval |
| Visible display / START exit / diagnostic tone | NOT RUN for v0.1.3 |
| iPhone connection / RTP / ALAC / audible music | NOT RUN for v0.1.3 |

## Next action

Request approval to transfer the exact v0.1.3 package under a versioned filename while preserving the staged v0.1.2 package. After an approved install, test launch first. If it starts, verify the display, five-second tone, and clean START exit before attempting an iPhone connection. If it still fails, retain the failure evidence and continue loader/package isolation without changing CFW or boot settings.
