# AIRPLAY02 v0.2.4 build and install

**Date:** 2026-11-07 (EDT)
**Source:** `f870d823f15398691a6a38e6393f8d1376161bee` on `main`
**Hardware:** PS3 demo system, Evilnat 4.93 CFW, Cobra 8.5 observed

## Changes in this build

- Keep the static framebuffer image idle normally, but resume paced flips while the PS system menu is open. This targets the prior invisible-menu report; visible menu behavior is still unverified.
- Open the diagnostic log in append mode. Each launch begins with a UTC-timestamped `APP_START`, and each session has its own 16 KiB limit. The cap marker is `LOG_SESSION_CAPPED`; logging stops for that run after the cap. Earlier sessions are preserved, so total log size grows by up to 16 KiB per launch.

## Build and package evidence

- Existing Lima VM, PS3DK v0.27.0 PPU + PSL1GHT; full host ASan/UBSan, PPU components, standalone ELF validation, and debug package checks passed.
- ELF: 2,754,776 bytes; SHA-256 `bd492c726b12e503d268e00cf03dee82c2c3d6f2aac7fa8f4f07c85a68b2e8ae`.
- PKG: AIRPLAY02 v0.2.4 / SFO 00.44; 3,231,984 bytes; SHA-256 `a27d5c7b5d2d84f12fbb82ee4e164b9b50ccc825b5104b9b749f053d72a42bf7`.
- Downloaded-back staged PKG is byte-identical. Installed SFO matches 00.44 exactly; installed EBOOT matches the package SELF exactly.

## Hardware status

The previously running app was closed through webMAN and its log recorded `SYSUTIL_EXIT_GAME`, screen/audio/receiver cleanup, and `APP_STOP`. v0.2.4 is installed, but no post-install launch check was performed. No claims are made about visible PS-menu output, controller behavior, AirPlay playback, or physical append-only log behavior.

## Next test

Launch v0.2.4 once from XMB. Test PS menu visibility and return, then test START. Retrieve the log after exit and verify the new timestamped `APP_START` follows and preserves the previous session.
