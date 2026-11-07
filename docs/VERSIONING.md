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
- The next unused package version is 0.1.18.
