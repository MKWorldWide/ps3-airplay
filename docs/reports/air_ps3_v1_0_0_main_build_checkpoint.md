# AIR PS3 v1.0.0 exact-main build checkpoint

**Date:** 2026-11-08 (EST)
**Source revision:** `38f13ba2b3255352a9957160c3d075e5426cd069`
**Environment:** existing Ubuntu 24.04 x86_64 Lima guest; PS3DK v0.27.0 PPU, PSL1GHT `dd4e8c9be744e3273d665c393fa6d97a87402e2e`, GCC 16.1.0, GNU ld 2.42, Mbed TLS 3.6.7.
**Validation boundary:** host, sanitizer, cross-build, and package validation. No PS3 or RPCS3 operation was performed.

## Release identity

- App version: `1.0.0`
- SFO `APP_VER` and `VERSION`: `01.00`
- Title ID: `AIRPLAY02`
- XMB title: `AIR PS3`
- AirPlay service name: `AirPS3`
- Package format: type-1 debug/homebrew PKG

## Results

| Artifact | Size | SHA-256 |
|---|---:|---|
| `ps3-airplay-receiver-1.0.0.pkg` | 3,906,640 bytes | `1244be0bb7ef1e6c8565a2a9f06e82bf7d43ad7d10f570fa1ad30dc394bee259` |
| CellOS PPU ELF | 2,755,120 bytes | `82fb2d4cc4d7e341267337f77ba8b75bdc50e8c3f704b0346113744263a863e9` |
| NPDRM EBOOT | 494,320 bytes | `05b074706476fa220da35c8dfcae8b813acea6d24a334d02b3b0f28d8ebd6e42` |
| Extracted PARAM.SFO | 1,040 bytes | `26de2a6b23df8e33d974351f10e474416b00e3ea171d03fdb4cce9101d341b16` |
| Embedded background RGB | 2,764,800 bytes | `602d7df8352a816a27cb1152a18f5062ff25e709c31bf564787b0da02980efde` |
| `release-evidence.json` | 2,300 bytes | `c1c4902bbf70b8d202405209918e3c1a30a2925ee670de4f49075b9d290b18a3` |

The exact-source release script passed version/evidence regressions; host ASan/UBSan and protocol, RTSP/RTP, crypto, ALAC, PCM, gain, resampling, timing, logging, and display-resume regressions; PPU crypto/receiver compilation; CellOS PPC64 ELF validation; artwork source/derivative comparison; extracted SFO identity; package extraction/path and debug-envelope validation; and evidence/checksum verification. The source-stamped evidence records all seven required validators as passed.

## Comparison with the pre-attribution candidate

The earlier candidate was 3,906,640 bytes with SHA-256 `ef8f0afdacef560e9f5825fa9539247f3eb17e7791282aa6790f086fa8b1288e` and referred to a former source SHA. The new exact-main package has a different PKG and EBOOT hash. The PPU ELF, SFO, and artwork hashes match the earlier candidate. This exact-main checkpoint and its source-bound evidence are valid only for the listed source revision; repeat the build after any subsequent source commit before staging. The older package is retained only as historical evidence. No cause for the changed NPDRM/package bytes is asserted.

## Hardware boundary

The maintainer reports v1.0.0 is running beautifully on the development PS3. This is qualitative, user-reported feedback. No run duration, fresh log, or individual playback/control checks were included. The exact-main package built here was not installed or launched, and neither PS3 was changed.

## Artifact retention

The package, checksum manifest, release evidence, and source archive are retained in ignored local build storage. Only the verified package, evidence, checksum, release notes, installation guide, notices, and required license files are intended for the GitHub release.
