# Project state

## Architecture

PS3 AirPlay is an experimental AirPlay 1 / RAOP receiver for PS3 homebrew. The intended audio path is DNS-SD/mDNS discovery, bounded RTSP session control, UDP RTP receive, decryption and ALAC decode, bounded PCM buffering, and output through the standalone GameOS audio API.

## Components

- `src/discovery`: RAOP DNS-SD serialization and service behavior.
- `src/protocol`: bounded RTSP sessions, SDP, RTP receive, retransmission, and timing probes.
- `src/crypto`: RAOP challenge/session crypto helpers.
- `src/codec`: bounded ALAC wrapper and decoder.
- `src/audio`: bounded PCM ring and conversion.
- `src/observability`: capped, privacy-conscious diagnostic logging.
- `prototype/standalone-receiver`: standalone receiver wiring protocol and audio components.
- The standalone screen candidate uses PSL1GHT RSX framebuffers, primes scanout using the PSL1GHT `videoTest` sample's spare-buffer-first pattern, and overlays a bright `DISPLAY ACTIVE` banner and controller START exit prompt over the public maintainer-provided background. This makes black/no-signal distinguishable from the intentionally dark image; visible output and orderly XMB return remain unconfirmed on hardware.
- The standalone app begins with a bounded five-second 440 Hz diagnostic rendered through the same GameOS audio blocks as receiver PCM; host waveform checks do not establish audible hardware output.
- Latest hardware observation: the user reports a five-second tone, a black screen, and iPhone discovery of `ps3-airplay-diagnostic` followed by a failed connection. A 350-byte log snapshot contains `SCREEN_READY`, `CONTROLLER_READY`, `MDNS_READY`, and `AUDIO_TEST_TONE_START` but no completion, RTSP, or stop event; its package version is unconfirmed, and FTP later timed out. Do not infer that visible pixels reached the display or that AirPlay connected.
- Previous verified build: version 01.01 was staged with an exact downloaded-back SHA-256 match; the later black-screen run's package identity is unknown. The prior staged package was preserved locally before replacement.
- Version 0.1.2 builds with the high-visibility display overlay, verbose RAOP logging, common response headers, PAUSE/resume transitions, and NetCtl-derived IPv4 service advertisement. The package extracted with the expected SFO (APP_VER 0.1.2), NPDRM EBOOT, icon, and exact 2,764,800-byte RGB background. It is now staged and hash-verified but not installed or hardware-tested.
- Initial review on 2026-11-07 reconfirmed the v0.1.2 candidate is 3,225,712 bytes with SHA-256 `923aebf6eca5693b21b494a9a4225c36629f13e8b316b2825d5deb3763226ebf`; package structure and embedded assets match the previous extraction. The console was unreachable on that attempt; a later read-only preflight succeeded and the subsequent approved staging is recorded below.
- Fresh receiver loopback rerun did not complete without tracing and timed out while waiting for the timing probe. A traced run completed the timing exchange and the persistent RTSP/RTP loopback test. Treat this as unresolved host-test flakiness, not a pass. The display code also contains unbounded polling for RSX label completion and flip status; the controller is initialized only after display setup returns.
- Diagnostic coverage review: current sources log startup/display/controller/network/listener/session/RTSP method/status/SDP/RTP batches, decrypt failures, ALAC frames/failures, PCM drops/underflows, user exit, and cleanup. They do not yet record successful decrypt counters or PCM frames rendered/output occupancy, and `SCREEN_READY` remains initialization evidence only.
- Read-only hardware preflight later succeeded: PS3 status showed XMB and 9.2 GB free; installed `AIRPLAY02` SFO reports 01.01. Immediately before approved v0.1.2 staging, `/dev_hdd0/packages/ps3-airplay-receiver.pkg` was 3,222,640 bytes with SHA-256 `7824a4b15e4757afad3213a3f55cd3bea816b33795403da12bfd7ede29f93db4`. An exact local rollback copy and the 1,817-byte log are preserved under ignored `work/hardware/2026-11-07-preflight/`.
- Sanitized event review of that log: `APP_START`, `SCREEN_READY`, `CONTROLLER_READY`, `MDNS_READY`, `AUDIO_READY`, tone start/completion, PCM underflows, and `APP_STOP`; no RTSP event or `USER_EXIT_START`. It has no package-version marker, so it cannot conclusively identify the run's package or prove a normal XMB exit. No v0.1.2 transfer/install occurred.
- After explicit user approval, v0.1.2 replaced only the staged PKG. Local size (3,225,712 bytes), local SHA-256, downloaded-back SHA-256, and remote size all match. The old 01.01 staged PKG remains preserved locally at the verified rollback path. Installed AIRPLAY02 remains 01.01; no install, launch, boot-plugin, or firmware operation occurred.
- Version 0.1.2 diagnostics log allowlisted RTSP methods, response statuses, state changes, SDP format fields, listener/UDP socket readiness and failures, RTP batches, and bounded-log wrap events. Logs remain capped at 16 KiB and exclude raw requests, peer addresses, stream content, keys, and track metadata.
- PSL1GHT's upstream issue #144 documents a similar black screen and unavailable XMB exit on real hardware with Evilnat; its author reported graphics samples using `-O2` eventually booting, and this project already uses `-O2`. This is a compatibility clue, not a confirmed root cause or fix.
- `experimental/resident`: diagnostic PRX candidate; it does not output PCM.

## Verified boundary

Host tests cover portable protocol, crypto, codec, PCM, and discovery components. Cross-build and package/container checks have been performed. PS3 hardware has played the diagnostic tone and advertised the receiver per user report; video rendering, iPhone interoperability, normal music playback, and game audio coexistence remain unverified. Game mixing remains a separate feasibility gate.

Demonstration target: PS3 with Evilnat 4.93 CFW and Cobra 8.5 observed; the console model was not recorded. Hardware compatibility remains unverified.

## Invariants

- Keep protocol inputs, sessions, queues, and logs bounded.
- Do not log track metadata, credentials, cryptographic material, or peer addresses.
- Advertise only implemented capabilities and label diagnostic-only behavior accurately.
- Distinguish host tests and format checks from hardware behavior.
- Keep boot-time plugin configuration unchanged during build work.
- Preserve third-party notices and license obligations.
