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
