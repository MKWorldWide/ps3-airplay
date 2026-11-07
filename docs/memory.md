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
- Latest local candidate: version 01.02 builds with the high-visibility display overlay and two-step scanout prime. The package extracted with the expected SFO, NPDRM EBOOT, icon, and exact 2,764,800-byte RGB background. It remains local and has not been transferred or hardware-tested.
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
