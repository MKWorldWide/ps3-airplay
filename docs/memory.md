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
- The standalone screen candidate uses PSL1GHT RSX framebuffers to show the public maintainer-provided background and a controller START exit prompt; orderly XMB return is not yet hardware-confirmed.
- The standalone app begins with a bounded five-second 440 Hz diagnostic rendered through the same GameOS audio blocks as receiver PCM; host waveform checks do not establish audible hardware output.
- Latest hardware observation: the user reports a five-second tone, a black screen, and iPhone discovery of `ps3-airplay-diagnostic` followed by a failed connection. The console required a forced power-button shutdown; the current-run log has not been retrieved. This confirms audio output and discovery only as user-reported observations, not a successful AirPlay session.
- Latest verified build: screen-enabled PPU ELF and standalone package built in the existing x86_64 Lima guest. The package includes a size-checked 1280x720 RGB background and NPDRM EBOOT. It has not been transferred or run on hardware; its package digest is pending approval before replacing the staged file.
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
