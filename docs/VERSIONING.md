# Versioning policy

AIRPLAY02 follows conservative pre-1.0 versioning. Increment the patch number for each distinct fix or small improvement that produces a new package: `0.1.4`, `0.1.5`, `0.1.6`, and so on. Never reuse a published or tested version number for different package contents. Record the exact package hash beside every result.

## Milestones

- **0.1.x — focused fixes and validation.** Each package changes one or a few clearly described items and carries a matching SFO and startup-log version. Failed candidates remain documented; the next distinct package increments the final number.
- **0.2.0 — stable diagnostic application.** The standalone app launches and exits reliably, provides actionable capped logs, and exposes basic settings and diagnostic toggles. This milestone does not claim working AirPlay audio.
- **0.3.0 — stable AirPlay receiver.** Discovery, connection, streamed audio, pause/resume, volume, reconnect, and sustained playback are verified on real hardware. RPCS3 and host tests are useful gates but do not replace console and iPhone evidence.
- **Later milestones — separately demonstrated capabilities.** Game-audio coexistence, resident-module operation, and other major features require their own feasibility and hardware evidence.

## Release rules

1. Choose the next version before building a new PKG; a failed package still consumes its version.
2. Keep `APP_VERSION`, SFO `APP_VER`, artifact filename, and startup log marker aligned.
3. Record source revision, package size, SHA-256, toolchain, checks run, and the exact failure boundary.
4. Reserve a minor milestone for a coherent capability boundary; do not advance minor versions for routine fixes.
5. Do not call a milestone stable until its acceptance tests in the roadmap pass at the stated hardware boundary.

## v0.1.16 package record

- Source: `0a204b04364803e2af9fcc8adf2f7420751ac83f` on `main`.
- Artifact: `ps3-airplay-receiver-0.1.16.pkg`, AIRPLAY02, APP_VER `0.1.16`.
- Size / SHA-256: 3,231,088 bytes / `3c69bff7ab626c433728ca131bc04ab0adbc117b8845d88c748e1dad1968bd3c`.
- ELF: 2,780,096 bytes, CellOS PPC64, five PT_LOAD segments; SHA-256 `968f9c3238e7c8d4479c0191452f378f4705d7d6585c68364cfcd7d792682f2a`.
- Build: Existing Lima VM; PS3DK v0.27.0 PPU compiler 16.1.0, GNU ld 2.42, PSL1GHT revision `dd4e8c9be744e3273d665c393fa6d97a87402e2e`.
- Package checks: SFO/version/title ID/content ID, icon/background, extraction, EBOOT comparison, and finalized NPDRM envelope PASS.
- RPCS3 installation/launch and physical hardware tests: pending. Do not infer volume behavior from host tests or ELF/package validation.
- At the time of this v0.1.16 record, the next package version was 0.1.17; that version is recorded below.

## v0.1.17 package record

- Source: `4a06c2a` on `main` (display cleanup: render the background without text overlays).
- Artifact: `ps3-airplay-receiver-0.1.17.pkg`, AIRPLAY02, APP_VER `0.1.17`.
- Size / SHA-256: 3,230,256 bytes / `2095046e20716d9f2768c847691515f1d1e922e5fbd96e15829f92c45816cf23`.
- ELF: 2,780,064 bytes, CellOS PPC64, five PT_LOAD segments; SHA-256 `5e80257671409775d98884c7101acf15d7e116448498ee8c69148843a96896dd`.
- Build: Existing Lima VM; PS3DK v0.27.0 PPU compiler 16.1.0, GNU ld 2.42, PSL1GHT revision `dd4e8c9be744e3273d665c393fa6d97a87402e2e`.
- Checks: full Lima host suite, PPU object compilation, standalone link/ELF validation, extracted SFO/title/content ID/assets, EBOOT equality, and finalized package envelope PASS.
- RPCS3 install/launch/display and physical PS3 tests: pending. Do not infer screen pixels, clean exit, discovery, or playback from the build.
- At the time of this v0.1.17 record, the next package version was 0.1.18; that version is recorded below.

## v0.1.18 package record

- Source: `8769324` on `main` (bounded periodic PCM pipeline counters).
- Artifact: `ps3-airplay-receiver-0.1.18.pkg`, AIRPLAY02, APP_VER `0.1.18`.
- Size / SHA-256: 3,230,448 bytes / `5548fde98bc08949e4f3a6739a95d5b3dce7dea0458131ec693c01be97766ae0`.
- ELF: 2,780,120 bytes, CellOS PPC64, five PT_LOAD segments; SHA-256 `ba7450441a8e2def2a1f55e17d523630d3d5af256520f6769154a347b09d3759`.
- Build: Existing Lima VM; PS3DK v0.27.0 PPU compiler 16.1.0, GNU ld 2.42, PSL1GHT revision `dd4e8c9be744e3273d665c393fa6d97a87402e2e`.
- Checks: full Lima host suite, PPU object compilation, standalone link/ELF validation, extracted SFO/title/content ID/assets, EBOOT equality, and finalized NPDRM envelope PASS.
- RPCS3 install/launch and emitted telemetry verification: pending. No physical hardware or audible playback evidence.
- The next unused package version after the v0.1.18 record was 0.1.19; its build record follows.

## v0.1.19 package record

- Source: `7b451031487aab7dbad147ab0b47ef80a1e22265` on `main` (ALAC negotiated sample-count bounds and composed encrypted RTP-to-PCM host test).
- Artifact: `ps3-airplay-receiver-0.1.19.pkg`, AIRPLAY02, APP_VER `0.1.19`.
- Size / SHA-256: 3,230,704 bytes / `41488b9683b901ecc4d71e113565fcdb10c1834c69e1642fbe774f5cdf4a31a5`.
- ELF: 2,780,120 bytes, CellOS PPC64, five PT_LOAD segments; SHA-256 `beb816b2aed2125a94584e4e820f18de66a98f21260ce47fe1654199322e7ec3`.
- Build: Existing Lima VM; PS3DK v0.27.0 PPU compiler 16.1.0, GNU ld 2.42, PSL1GHT revision `dd4e8c9be744e3273d665c393fa6d97a87402e2e`.
- Checks: full Lima host suite under ASan/UBSan, PPU component compilation, standalone link/ELF validator, package extraction, AIRPLAY02/APP_VER/content ID, asset equality, EBOOT equality, relative package paths, and finalized NPDRM envelope PASS.
- The package and ELF are preserved in the ignored local `work/builds/0.1.19/` directory. RPCS3 install/launch was not performed because Computer Use still reports the Mac desktop locked. No physical-console transfer/install, iPhone session, or audible playback is verified.
- Next unused package version: 0.1.20.

The standalone receiver `Makefile` and canonical Lima build/package scripts default to 0.1.22.

## v0.1.20 package record

- Source: `8a4e641d114e1a613421b9ffb51f8615ad851827` on `main` (ALAC malformed-bitstream bounds and arithmetic hardening).
- Artifact: `ps3-airplay-receiver-0.1.20.pkg`, AIRPLAY02, APP_VER `0.1.20`.
- Size / SHA-256: 3,230,384 bytes / `bfaa3f960de39daf95c2ab405bd361a031ed92bd35c76a99e667d4ffe8d19e87`.
- ELF: 2,780,120 bytes, CellOS PPC64, five PT_LOAD segments; SHA-256 `f50e6a4fe247149feb7759a69a81ae881c0972d789cb1a5dcfa2019bd202b3d9`.
- Build: Existing Lima VM; PS3DK v0.27.0 PPU compiler 16.1.0, GNU ld 2.42, PSL1GHT revision `dd4e8c9be744e3273d665c393fa6d97a87402e2e`.
- Checks: Full Lima host suite with ASan/UBSan halt-on-error, deterministic 256 malformed ALAC inputs, encrypted RTP-to-PCM golden test, PPU app link/ELF validation, SFO/version/title/content ID, assets, extraction, EBOOT comparison, and finalized NPDRM envelope PASS.
- Runtime: RPCS3 is not tested; current Computer Use reports the Mac desktop locked. No physical PS3 transfer/install, AirPlay sender session, or audible output is verified.
- The package, ELF, and build-info are preserved locally under ignored `work/builds/0.1.20/`. Next unused package version: 0.1.21.

## v0.1.21 package record

- Source: `3e529faac64b0b30989b955e01c991e63fdaeda8` on `main` (correct RAOP `vs` server-version TXT value).
- Artifact: `ps3-airplay-receiver-0.1.21.pkg`, AIRPLAY02, APP_VER `0.1.21`.
- Change: advertise `vs=105.1`, matching the RTSP `Server: AirTunes/105.1` header; the previous `vs=0.1.2` was stale package metadata.
- ELF: 2,780,120 bytes, CellOS PPC64, five PT_LOAD segments; SHA-256 `401225f2c8b704103c5f65683a5b48f9c7b34633ae57145ca92c190485543e1e`.
- Size / SHA-256: 3,230,384 bytes / `c8c77b78d34dc50a955383ddafe7c835a7848c62eb8a9673ede7ed0c2776ef73`.
- Build: existing Lima VM; PS3DK v0.27.0 PPU compiler 16.1.0, GNU ld 2.42, PSL1GHT revision `dd4e8c9be744e3273d665c393fa6d97a87402e2e`.
- Checks: RAOP TXT serializer assertion, full Lima host suite with ASan/UBSan, PPU component checks, standalone ELF validation/link, AIRPLAY02/APP_VER/content ID, asset equality, extraction, EBOOT comparison, relative package paths, and finalized NPDRM envelope PASS.
- Runtime: RPCS3 Computer Use reports the Mac is locked; no fresh v0.1.21 install/launch or matching log. Physical PS3 was not changed. No iPhone session or audible AirPlay playback is verified.
- The PKG, ELF, and build metadata are preserved in ignored `work/builds/0.1.21/`. Next unused package version: 0.1.22.

## v0.1.22 source pass

- Change: count successfully decrypted audio RTP payloads and log `RTP_DECRYPT_OK` for the first success and each 128-packet milestone. Failed decryptions retain `RTP_DECRYPT_FAILED`; the log contains only a bounded numeric count and payload byte count.
- Validation: full Lima host/sanitizer suite, composed encrypted RTP-to-PCM test with a new decrypt-counter assertion, and PPU component compilation PASS.
- Full standalone ELF/PKG build and artifact hashes pending. RPCS3 runtime and physical hardware remain unverified.
