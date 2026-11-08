# AIR PS3 v1.0.0 candidate build report

**Date:** 2026-11-08 (EST)
**Source revision:** `f3da352809f81daac5e3b61a35d7fc18362bf1cc`
**Validation boundary:** host tests, cross-build, and package inspection; no PS3 or RPCS3 runtime test.

## Release identity

- Application version: `1.0.0`
- SFO `APP_VER`: `01.00`
- SFO `VERSION`: `01.00`
- Title ID: `AIRPLAY02`
- XMB title: `AIR PS3`
- Package: debug/type 1

## Build and artifact

The committed source archive was built in the existing Lima VM using PS3DK v0.27.0 PPU, PSL1GHT revision `dd4e8c9be744e3273d665c393fa6d97a87402e2e`, GCC 16.1.0, GNU ld/binutils 2.42, and Mbed TLS 3.6.7.

| Artifact | Size | SHA-256 |
|---|---:|---|
| `ps3-airplay-receiver-1.0.0.pkg` | 3,906,640 bytes | `ef8f0afdacef560e9f5825fa9539247f3eb17e7791282aa6790f086fa8b1288e` |
| PPU ELF | 2,755,120 bytes | `82fb2d4cc4d7e341267337f77ba8b75bdc50e8c3f704b0346113744263a863e9` |
| SELF EBOOT | 494,320 bytes | `ac07abd3ddc99f1b962402c3bf15b7a9725f232c44415a16554a8d3250e00759` |
| Extracted PARAM.SFO | 1,040 bytes | `26de2a6b23df8e33d974351f10e474416b00e3ea171d03fdb4cce9101d341b16` |
| Embedded background RGB | 2,764,800 bytes | `602d7df8352a816a27cb1152a18f5062ff25e709c31bf564787b0da02980efde` |

The package hash and size match the build's `release-evidence.json` and `SHA256SUMS.txt`. The artifact and copied evidence are preserved locally under ignored `work/builds/1.0.0/`.

## Validation

- Full Lima release build script completed successfully from the exact source revision.
- Host tests with AddressSanitizer and UndefinedBehaviorSanitizer passed.
- PPU crypto/receiver build and CellOS ELF validation passed.
- Package envelope, extraction structure, relative paths, and embedded artwork checks passed.
- Independent extracted-SFO check passed: `APP_VER=01.00`, `VERSION=01.00`, `TITLE_ID=AIRPLAY02`, category `HG`, bootable flag `1`, title `AIR PS3`.
- Local release config and evidence regression tests passed; evidence verifier accepted the matching source and PKG hash.

## Status and boundary

The app is `1.0.0`; its stored fixed-width SFO version is `01.00` (the requested numeric `1.00`). No tag or GitHub Release was created. The candidate source revision remains on `lily/air-ps3-v028` and is not the current `origin/main` tip, so this package is not eligible for release staging. Rebuild from the exact main tip after source integration. No PS3 was installed, restarted, or otherwise changed during this pass.

## Next action

Integrate the reviewed implementation into `main`, build again from that exact main commit, then repeat artifact/privacy validation before staging a draft release.
