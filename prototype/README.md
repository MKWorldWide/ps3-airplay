# Prototype workspace

The repository contains a standalone low-volume PCM tone application and a
standalone RAOP receiver application. The receiver connects mDNS, persistent
RTSP/RTP, session crypto, ALAC decode, the bounded PCM ring, diagnostic logging,
and the PSL1GHT GameOS audio port. Lima builds it as a PPU ELF; packaging and
all console/iPhone playback tests remain open.

Current and planned directories:
- `prototype/standalone/`: PSL1GHT app and audio tone; no RAOP listener yet.
- `prototype/standalone-receiver/`: PSL1GHT receiver/audio app; uses shared RAOP
  code and sends decoded PCM to the audio ring; see its [build and runtime
  notes](standalone-receiver/README.md).
- `prototype/discovery/`: mDNS packet-format scope and validation boundary.
- `src/discovery/`: portable RAOP DNS-SD response builder; no network transport.
- `src/protocol/`: platform-neutral AirPlay 1 / RAOP implementation adapted with license notices.
- `src/platform/ps3/`: PS3 GameOS sockets, threads, audio and logger.
- `experimental/resident/`: opt-in Cobra .sprx plugin experiment; NOT installed by default.

Current gate: package the receiver ELF in Lima, then establish a verified
console recovery route before a standalone app launch. Its service TXT remains
diagnostic-only until iPhone discovery and audible output work.

See [ROADMAP](../docs/ROADMAP.md).
