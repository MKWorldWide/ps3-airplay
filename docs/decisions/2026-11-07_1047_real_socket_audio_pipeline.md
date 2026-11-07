# Real-socket RAOP-to-PCM host integration

## Timestamp in EDT

2026-11-07 10:47 EDT

## Scope of the pass

Added a host integration test that drives the receiver through real TCP RTSP and UDP RTP sockets, decrypts a fixture ALAC frame, decodes reference PCM, and enqueues stereo frames into the PCM ring. Also attempted a reversible direct RPCS3 headless launch of the v0.1.20 EBOOT while the UI remained locked.

## Why this pass was needed

Existing tests covered real RTSP/RTP sockets and encrypted RTP-to-PCM composition separately. They did not exercise those layers together through the receiver's live socket path.

## Files changed

- `tests/protocol/raop_audio_pipeline_test.c`
- `scripts/build-lima-crypto.sh`
- `docs/TEST_MATRIX.md`
- `docs/ROADMAP.md`
- `docs/memory.md`
- `docs/lessons-learned.md`
- `docs/scratchpad.md`
- `docs/decisions/2026-11-07_1047_real_socket_audio_pipeline.md`

Local PKG, ELF, and logs remain ignored under `work/`.

## Architecture / logic changes

The composed test opens the actual receiver on ephemeral TCP/UDP ports, accepts a loopback client, handles OPTIONS, fragmented ANNOUNCE, SETUP, and RECORD, receives the AES-CBC encrypted ALAC RTP datagram, decrypts via `raop_receiver_pop_audio`, decodes to reference PCM, and pushes 353 stereo frames to the PCM ring. The Lima host test build now links `pcm_ring.c` into this test. No production protocol or audio code changed.

## Runtime behavior changes

No application runtime behavior change; this is host test coverage only.

## Validation performed

- `scripts/build-lima-crypto.sh` in the existing Lima VM.
- Full Lima crypto, SDP/session, mDNS, persistent socket loopback, ALAC sanitizer, PCM, gain, tone, and PPU object checks: PASS.
- New live TCP/UDP encrypted RTP-to-PCM-ring test: PASS.
- RPCS3 direct `--no-gui` EBOOT attempt: stopped at the 28-second guard; stdout was empty and the capped app log remained v0.1.15 with no v0.1.20 startup marker. The exact v0.1.20 package was not installed.

## Immediate results

The full host receive/decode/buffer path now has one real-socket integration test. Emulator runtime remains unverified.

## Known limitations / follow-ups

Loopback fixture tests do not prove modern iPhone interoperability, wireless discovery, sender timing behavior, GameOS audio submission, physical HDMI output, Spotify, or audible music. The RPCS3 headless attempt did not yield enough evidence to classify the title launch.

## Risks introduced

The test introduces ephemeral local TCP/UDP sockets and uses fixed loopback peer control/timing port values that are only advertised in test RTSP; it does not send timing probes or retransmission requests during the fixture run. Ports are allocated by the receiver at runtime.

## Decision record (embedded)

### Decision: Compose the real receiver socket path with known-good crypto and audio fixtures

- **Context:** The protocol socket path and decoder path had been tested in separate host tests.
- **Options considered:** Keep those tests separate; build an iPhone-only test; add a deterministic socket simulation with known reference data.
- **Why chosen:** The composed fixture exercises TCP framing, state transitions, UDP delivery, crypto, codec, and PCM buffering without external services or private sender data.
- **Tradeoffs accepted:** The test still cannot establish iPhone-specific request compatibility or PS3 GameOS output.

### Decision: Preserve the direct RPCS3 launch attempt as inconclusive

- **Context:** The UI remained locked, but a command-line attempt was possible.
- **Options considered:** Treat process execution as a launch; classify it as failure; require a fresh app log and version marker.
- **Why chosen:** The installed app log did not change and contained no v0.1.20 marker, so there is no evidence the title code ran.
- **Tradeoffs accepted:** A GUI install/launch remains pending; the v0.1.20 package stays untouched.

## Next recommended step

When the Mac is unlocked, install the exact v0.1.20 PKG in RPCS3 and collect a fresh matching diagnostic log. Use that first failing stage to determine the next v0.1.21 application fix.
