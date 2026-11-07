# Roadmap

**Status: experimental; target hardware validation has not started.** This roadmap separates what has been exercised on the host or through cross-builds from what still requires a PS3 and an iPhone.

## Milestones

| Milestone | Goal | Status |
|---|---|---|
| Build foundation | Reproducible PPU build and sample PRX structure check | Host/toolchain work complete; hardware compatibility unknown |
| Standalone audio | Launch the receiver and verify audible output at the system menu | Five-second GameOS-path tone implemented; PS3 not tested |
| AirPlay interoperability | Discover the receiver and complete a real iPhone RAOP session | Not tested |
| Standalone playback | Stable playback, controls, reconnects, and a 30-minute session | Not tested |
| Resident module | Safely load, receive, stop, and unload a diagnostic PRX | Build checks only; no hardware test |
| Game coexistence | Determine whether audio can coexist with game audio | Separate feasibility gate; no evidence yet |
| Release | Documented recovery, privacy, licensing, reliability, and install/removal process | Not ready |

## Current implementation

Portable components cover bounded DNS-SD/mDNS, RTSP/SDP session handling, UDP RTP, timing probes, retransmission requests and validation, RAOP crypto, ALAC decoding, PCM buffering, and capped diagnostics. The standalone app now plays a five-second, low-level 440 Hz startup diagnostic through its GameOS audio blocks before normal PCM rendering. Host tests and PS3 cross-build/package checks exist. No console playback test has been performed. The resident candidate decodes supported packets for diagnostics and discards PCM.

## Next steps

1. Improve RTP jitter handling and sender-clock scheduling with bounded memory and measurable host tests.
2. Establish a documented, tested console recovery path before installing or loading any candidate.
3. Validate standalone launch and audio output at the system menu.
4. Test iPhone discovery and RAOP session behavior; update advertised capabilities only when interoperability supports them.
5. Consider resident PRX lifecycle testing as a separate, reversible experiment.
6. Test audio coexistence per title only after resident lifecycle and standalone output are established.

## Explicit boundaries

- A successful build, package inspection, or PRX structural check does not prove hardware compatibility.
- No real iPhone session or audible output has been verified.
- System background-music controls do not demonstrate arbitrary PCM mixing.
- Do not claim universal in-game audio support without separate results from multiple games.
