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
- The standalone app begins with a bounded five-second 440 Hz diagnostic rendered through the same GameOS audio blocks as receiver PCM; host waveform checks do not establish audible hardware output.
- Latest verified build: PPU ELF and standalone PKG built in the existing x86_64 Lima guest from the current source; package metadata uses title ID `AIRPLAY02` and Game category. The package was staged by FTP and downloaded back with an exact SHA-256 match. Hardware install, audio, and iPhone tests remain unverified.
- `experimental/resident`: diagnostic PRX candidate; it does not output PCM.

## Verified boundary

Host tests cover portable protocol, crypto, codec, PCM, and discovery components. Cross-build and package/container checks have been performed. PS3 hardware load, iPhone interoperability, audible playback, and game audio coexistence have not been demonstrated. Game mixing remains a separate feasibility gate.

Demonstration target: PS3 with Evilnat 4.93 CFW and Cobra 8.5 observed; the console model was not recorded. Hardware compatibility remains unverified.

## Invariants

- Keep protocol inputs, sessions, queues, and logs bounded.
- Do not log track metadata, credentials, cryptographic material, or peer addresses.
- Advertise only implemented capabilities and label diagnostic-only behavior accurately.
- Distinguish host tests and format checks from hardware behavior.
- Keep boot-time plugin configuration unchanged during build work.
- Preserve third-party notices and license obligations.
