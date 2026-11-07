# Align RAOP TXT and RTSP server versions

## Timestamp in EDT

2026-11-07 11:00 EDT

## Scope of the pass

Changed the RAOP mDNS TXT `vs` value from the stale package version `0.1.2` to protocol/server version `105.1`, matching the RTSP `Server: AirTunes/105.1` response. Updated the discovery wire-format expectation, advanced package defaults to v0.1.21, and refreshed release/test/memory documentation.

## Why this pass was needed

The iPhone has discovered the receiver in prior user testing, but connection has not succeeded. Research showed a concrete inconsistency in the receiver's advertised protocol metadata. This correction improves discovery fidelity; it is not claimed as the cause or full fix for the connection failure.

## Files changed

- `src/discovery/raop_mdns_service.c`
- `tests/discovery/raop_mdns_test.c`
- `prototype/standalone-receiver/Makefile`
- `scripts/build-lima-standalone-receiver.sh`
- `scripts/package-lima-standalone-receiver.sh`
- `docs/BUILD_PIPELINE.md`
- `docs/ROADMAP.md`
- `docs/TEST_MATRIX.md`
- `docs/VERSIONING.md`
- `docs/lessons-learned.md`
- `docs/memory.md`
- `docs/scratchpad.md`
- `docs/decisions/2026-11-07_1100_raop_server_version_txt.md`

Local build output remains outside Git under ignored `work/`.

## Architecture / logic changes

The DNS-SD TXT metadata now reports the receiver server version rather than an old app package version. The RTSP protocol response is unchanged. A regression assertion ensures serializer output includes `vs=105.1`.

## Runtime behavior changes

New RAOP service announcements carry `vs=105.1`. The receiver continues to advertise only stereo ALAC over UDP with the existing encryption capability values.

## Validation performed

- Full `scripts/build-lima-crypto.sh` in the existing Lima VM: PASS after correcting the shell environment setup.
- Crypto, RTSP session/SDP, mDNS, persistent socket, encrypted RTP-to-ALAC-to-PCM, malformed ALAC sanitizer, PCM, gain, tone, and PPU object checks: PASS.
- Initial run reached all host tests but failed its final PPU object step because the environment assigned `PS3DEV` before `PS3DK_SOURCE`; rerun with sequential exports completed successfully.
- Standalone app/ELF/PKG build: pending.
- RPCS3 UI remains inaccessible while the Mac is locked; no runtime test.

## Immediate results

The source and host tests consistently advertise and expect the RAOP server version `105.1`. No iPhone connection or playback conclusion follows from host serializer tests.

## Known limitations / follow-ups

Build and hash the v0.1.21 package, validate ELF/SELF/SFO/package structure, then test in RPCS3 once desktop UI access is available. A real iPhone session and audible output remain separate requirements.

## Risks introduced

Incorrect or outdated protocol metadata could affect sender compatibility. This correction aligns the RAOP TXT and RTSP version strings, but its effect on iPhone connection behavior is unverified.

## Decision record (embedded)

### Decision: Use RAOP server version 105.1 in the TXT `vs` field

- **Context:** The RAOP TXT record used `vs=0.1.2` while the RTSP Server header already used `AirTunes/105.1`.
- **Options considered:** Keep the old package-version string; use the RAOP implementation version shared with the RTSP header; derive `vs` from `APP_VERSION`.
- **Why this option was chosen:** Protocol references identify `vs` as the server version, and established classic RAOP implementations use the same server identity in their responses and advertisements.
- **Tradeoffs accepted:** Package APP_VER and startup logs remain the product version; `vs` is protocol metadata, not a duplicate of the package version.

## Next recommended step

Build v0.1.21 from its committed source, then install and launch it in RPCS3 to capture a fresh startup and network diagnostic log.
