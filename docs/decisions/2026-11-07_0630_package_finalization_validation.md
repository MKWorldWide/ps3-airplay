# Separate package extraction from NPDRM finalization

**Timestamp:** 2026-11-07 06:30 EDT

## Scope of the pass

Use the committed recovered build pipeline to build, install, and launch AIRPLAY02 v0.1.7 in RPCS3. Record the fresh startup failure boundary and correct package validation ordering so the next distinct package is v0.1.8.

## Why this pass was needed

The finalized v0.1.7 package installed and reached APP_START in RPCS3, but packaging exited nonzero when the older bundled pkg reader tried to extract the finalized NPDRM type. Logs also showed the app exiting before receiver readiness at RTSP bind error 112.

## Files changed

- README.md
- docs/BUILD_PIPELINE.md
- docs/RISKS.md
- docs/ROADMAP.md
- docs/TEST_MATRIX.md
- docs/decisions/2026-11-07_0630_package_finalization_validation.md
- docs/lessons-learned.md
- docs/memory.md
- docs/reports/2026-11-07_v017_rpcs3_launch.md
- docs/scratchpad.md
- prototype/standalone-receiver/Makefile
- scripts/build-lima-standalone-receiver.sh
- scripts/package-lima-standalone-receiver.sh
- scripts/validate-ps3-pkg.py

## Architecture / logic changes

The package script now builds and extracts the unfinalized package first, checks relative paths, SFO APP_VER, content ID, EBOOT and image bytes, and then finalizes a copy. The new final-PKG checker validates PKG magic, finalized NPDRM type, declared size, data bounds, item count, and content ID without asking the legacy pkg reader to decode the finalized type.

The build and package defaults now target version 0.1.8 because v0.1.7 has already been generated and installed. RPCS3 networking was observed disconnected during the v0.1.7 run; no setting was changed and its relationship to the bind error remains unknown.

## Runtime behavior changes

No receiver runtime code changed. The packaging gate now reports the actual failing stage more accurately. Version defaults advance to 0.1.8 for the next PKG.

## Validation performed

- v0.1.7 finalized package installed in RPCS3 and the title list showed AIRPLAY02 0.1.7.
- Fresh log event sequence reached APP_START, then RTSP_BIND_FAIL value 112, RECEIVER_OPEN_FAILED, and APP_STOP.
- Package artifact: 3,225,904 bytes; SHA-256 fc8899dd3f62cbd9678bf3f27bdb542770fe9516ad142d37235ea3f958e09674.
- RPCS3 package metadata shows finalized type 0x80000001 and declared size equals actual size.
- Host protocol/audio suite and PPU checks passed in the prior pipeline pass.
- The v0.1.8 package has not been built yet; corrected extraction/finalization flow must be exercised after this commit.

## Immediate results

The executable now passes RPCS3 installation and reaches application entry, unlike v0.1.4-v0.1.6. Runtime networking remains the immediate blocker. No display, tone, AirPlay connection, RTP, or streamed music was established.

## Known limitations / follow-ups

- Build v0.1.8 from this commit and run the full package validation.
- Inspect RPCS3 network settings read-only. Do not change them without authorization.
- If active RPCS3 networking can be authorized and configured, retry receiver bind; otherwise treat emulator networking as blocked and use it only for package/loader validation.
- Physical PS3 display, clean exit, iPhone connection, audible playback, and long soak remain unverified.

## Risks introduced

- The package envelope checker relies on the fixed PKG header layout and currently validates only finalized package identity/size bounds, not cryptographic authenticity.
- v0.1.8 packaging may stop if the tool outputs or SFO schema differ from the currently staged tool versions.

## Decision record

### Decision 1: Extract before finalization

- **Context:** pkg --extract works on the generated package before finalization but reports Unsupported Type after package_finalize changes the type to 0x80000001.
- **Options considered:** Remove finalization; find another extraction tool; validate contents before finalization and verify final package envelope afterward.
- **Chosen:** Preserve PSL1GHT finalization and move content extraction checks before it.
- **Tradeoffs:** Finalized package internals are not re-extracted by the old bundled reader; installation in RPCS3 provides a separate end-to-end package acceptance check.

### Decision 2: Keep the emulator network failure separate from executable loading

- **Context:** v0.1.7 reaches APP_START, then bind fails with value 112 and the app stops; RPCS3 reported networking disconnected.
- **Options considered:** Treat the failure as another loader error; make speculative receiver socket changes; record the boundary and inspect emulator network setup.
- **Chosen:** Record a runtime socket failure and investigate emulator configuration read-only before changing any network setting.
- **Tradeoffs:** This does not immediately test RAOP on RPCS3, but avoids changing unrelated emulator configuration or misdiagnosing a network environment issue as an application defect.

### Decision 3: Advance to v0.1.8

- **Context:** The v0.1.7 package is already generated, installed, and has a recorded hash.
- **Options considered:** Rebuild a different package under 0.1.7; reserve 0.1.7; advance the package version.
- **Chosen:** Keep v0.1.7 immutable and set the next package version to 0.1.8.
- **Tradeoffs:** A tooling-only validation correction consumes a patch number when it produces a new package, preserving one-to-one version/hash history.

## Next recommended step

Commit this pass after the privacy review, rebuild AIRPLAY02 v0.1.8 from the committed source, validate its unfinalized contents and finalized header, then install/launch it in RPCS3 and inspect its fresh log.
