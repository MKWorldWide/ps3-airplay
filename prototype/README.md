# Prototype workspace

The repository contains a standalone low-volume PCM tone application and a
standalone RAOP receiver application. The receiver connects mDNS, persistent
RTSP/RTP, session crypto, ALAC decode, the bounded PCM ring, diagnostic logging,
and the PSL1GHT GameOS audio port. The current diagnostic-screen candidate adds
an RSX image framebuffer and a controller START exit action. The preceding
hardware build played its tone but displayed black; an iPhone found its service
but could not connect. The new screen and exit behavior still need hardware
validation.

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

Version 0.1.2 builds locally with a NetCtl-derived advertised IPv4 address,
verbose bounded RTSP/SDP/RTP diagnostics, and session response compatibility
updates. Its PKG has not been transferred or tested on hardware. After approval
of the exact package, verify visible rendering and controller exit, then make
one iPhone connection attempt and retrieve the capped log. Its service TXT
remains diagnostic-only until an iPhone session succeeds.

See [ROADMAP](../docs/ROADMAP.md).
