# Apply RAOP volume to the PCM stream

Timestamp: 2026-11-07 09:14 America/New_York (EST)

## Scope of the pass

Implemented bounded RTSP volume parsing and state, a mute sentinel, attenuation-only gain conversion, and a one-block PCM gain ramp. Added parser, state-transition, gain-bound, ramp, and mute tests. Updated the Lima test/build recipe to link math and compile the new audio object. Updated project memory, roadmap, risk, lesson, and scratchpad records.

## Why this pass was needed

The RTSP session returned 200 to `SET_PARAMETER` without applying the requested volume to PCM, so it advertised behavior the output path did not provide.

## Files changed

- `src/protocol/raop_session.c`
- `src/protocol/raop_session.h`
- `src/audio/pcm_gain.c`
- `src/audio/pcm_gain.h`
- `prototype/standalone-receiver/Makefile`
- `prototype/standalone-receiver/source/main.c`
- `tests/protocol/raop_session_test.c`
- `tests/audio/pcm_gain_test.c`
- `scripts/build-lima-crypto.sh`
- `docs/ROADMAP.md`
- `docs/TEST_MATRIX.md`
- `docs/RISKS.md`
- `docs/lessons-learned.md`
- `docs/memory.md`
- `docs/scratchpad.md`
- `docs/decisions/2026-11-07_0914_airplay-volume-control.md`

Generated host/PPU test objects and the pre-commit working-tree ELF are local validation artifacts and remain outside Git.

## Architecture / logic changes

`SET_PARAMETER` parses only the bounded `text/parameters` body. A valid volume value is stored in session state after a successful response. Typical -30..0 dB attenuation is converted to linear amplitude gain; positive values cap at unity, values below -30 dB down to the mute sentinel clamp to -30 dB, and -144 dB or lower maps to mute. Malformed or duplicate volume lines produce 400 and preserve the prior state. Other parameter lines remain no-op compatible.

The standalone app transfers session gain to a small PCM gain state machine. It linearly ramps to the target across one stereo output block and clamps the result to [-1,1]. Diagnostic-tone rendering returns before stream gain is applied, so the startup tone remains a separate audio-path check.

## Runtime behavior changes

Accepted volume changes now affect streamed PCM, are ramped across one audio block, and emit privacy-safe `AUDIO_VOLUME` values. Mute is represented by zero gain. Session teardown resets output gain to unity for the next sender session.

## Validation performed

- `scripts/build-lima-crypto.sh` in existing Lima VM: PASS, including RAOP crypto, RTSP crypto/session/SDP, mDNS, persistent RTSP/RTP loopback, capped logging, ALAC, PCM ring, PCM gain, and diagnostic tone tests.
- PPU object compilation: PASS for RAOP session/receiver and PCM gain; objects were identified as PowerPC64 relocatable ELF.
- Standalone build: `make -C prototype/standalone-receiver clean`, then `make -C prototype/standalone-receiver CC=powerpc64-ps3-elf-gcc VERBOSE=1 APP_VERSION=0.1.16 RTSP_PORT=5000`: PASS.
- ELF check: `python3 scripts/validate-ps3-elf.py prototype/standalone-receiver/ps3-airplay-receiver.elf`: PASS; CellOS PPC64 executable with five load segments, 2,780,096 bytes, SHA-256 `968f9c3238e7c8d4479c0191452f378f4705d7d6585c68364cfcd7d792682f2a`.
- Volume cases: attenuation conversion, `-144 dB` mute, lower/upper bounds, malformed `nan` and duplicate volume rejected without mutation, progress-only request accepted as no-op, teardown resets gain.
- The first make attempt failed because dependent Lima paths were exported together before parent values existed; sequential exports fixed the environment without source changes.
- Canonical build-script manifest, PKG generation, RPCS3 volume runtime, and physical audio-level test: not yet performed.
- RPCS3 Internet setting was read-only verified as Disconnected; unchanged.

## Immediate results

The previous false-success volume no-op is fixed in source and host tests prove the parsed value reaches a bounded PCM sample transform. Cross-compiled audio gain and session objects compile for PPU; the full application links and passes the CellOS ELF validator. The canonical versioned package remains to be generated from a source commit.

## Known limitations / follow-ups

- Confirm actual iPhone volume payload behavior and output gain on PS3 hardware.
- Current mapping is a conventional amplitude dB conversion; AirPlay implementations use different transfer curves, so subjective slider shape remains unverified.
- RPCS3 cannot test this path until its guest network route reaches listener/audio startup. No iPhone connection, ALAC stream, or Spotify playback is proven.
- Build the next package as v0.1.16 after the source commit; retain v0.1.15 unchanged.

## Risks introduced

The volume curve could feel different from an iPhone slider even though its dB attenuation and mute semantics are bounded. Per-block ramping mitigates abrupt gain steps but is not hardware-tested. Maximum gain remains at unity, preventing software amplification.

## Decision record

### Decision 1: Use attenuation-only dB volume and the -144 dB mute sentinel

- **Context:** Shairport Sync's maintainer describes the typical AirPlay volume range as -30..0 dB with -144 dB used for mute.
- **Options considered:** Acknowledge volume but ignore it; map the requested dB to amplitude gain; add a custom linear slider scale; use a hardware mixer control.
- **Why chosen:** This matches documented behavior from an established AirPlay 1 receiver and uses a portable GameOS PCM path without assuming a controllable PS3 mixer.
- **Tradeoffs accepted:** The iPhone's exact transfer curve remains unverified; values between -30 and -144 dB are safely clamped to -30 dB except the explicit mute sentinel.

### Decision 2: Ramp gain at the output block boundary

- **Context:** Immediate gain changes can click; allocation and protocol work do not belong in the real-time sample loop.
- **Options considered:** Apply a new gain abruptly; implement a sample-by-sample one-block ramp; build a larger configurable fade engine.
- **Why chosen:** A small state object provides deterministic bounded work and is independently host-tested.
- **Tradeoffs accepted:** Ramp duration follows the GameOS audio block and has not been measured on console.

### Decision 3: Keep unknown parameter lines as no-op compatible

- **Context:** RAOP `SET_PARAMETER` can carry metadata and progress fields in addition to volume.
- **Options considered:** Reject every field except volume; parse/apply only volume and retain the existing 200 behavior for unknown fields.
- **Why chosen:** It avoids breaking sender metadata/keepalive traffic outside the implemented audio control while ensuring malformed volume is not falsely acknowledged.
- **Tradeoffs accepted:** Unknown parameter fields remain accepted but are not applied or logged.

## Next recommended step

Commit the validated source pass, then run the canonical v0.1.16 build/package scripts from that exact commit. Install and test in RPCS3 once its virtual network route is available; verify volume on the physical PS3 before claiming hardware support.
