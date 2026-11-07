# Roadmap

**Status: experimental; the standalone receiver played its diagnostic tone on PS3 hardware, but showed a black screen and did not complete an iPhone connection. Revised high-visibility version 01.02 builds locally; version 01.01 remains staged, and the observed run's package identity is unknown.** This roadmap separates build evidence from behavior that still requires a PS3 and an iPhone.

## Milestones

| Milestone | Goal | Status |
|---|---|---|
| Build foundation | Reproducible PPU build and sample PRX structure check | Host/toolchain work complete; hardware compatibility unknown |
| Standalone audio | Launch the receiver and verify audible output at the system menu | Five-second tone heard on hardware; screen output and clean exit unresolved; version 01.01 package staged |
| AirPlay interoperability | Discover the receiver and complete a real iPhone RAOP session | iPhone lists `ps3-airplay-diagnostic`; selecting it fails to connect |
| Standalone playback | Stable playback, controls, reconnects, and a 30-minute session | Not tested |
| Resident module | Safely load, receive, stop, and unload a diagnostic PRX | Build checks only; no hardware test |
| Game coexistence | Determine whether audio can coexist with game audio | Separate feasibility gate; no evidence yet |
| Release | Documented recovery, privacy, licensing, reliability, and install/removal process | Not ready |

## Current implementation

Portable components cover bounded DNS-SD/mDNS, RTSP/SDP session handling, UDP RTP, timing probes, retransmission requests and validation, RAOP crypto, ALAC decoding, PCM buffering, and capped diagnostics. The standalone app plays a five-second, low-level 440 Hz startup diagnostic through its GameOS audio blocks before normal PCM rendering. The revised local screen candidate primes scanout with PSL1GHT's sample sequence and overlays a bright `DISPLAY ACTIVE` banner and START prompt over the maintainer's background. It polls controller START for orderly return to XMB. The screen code cross-builds in Lima; hardware rendering and exit behavior are unverified. The resident candidate decodes supported packets for diagnostics and discards PCM.

## Next steps

1. Improve RTP jitter handling and sender-clock scheduling with bounded memory and measurable host tests.
2. Review the revised high-visibility 01.02 package; install only after the user approves that exact package.
3. Check the `DISPLAY ACTIVE` banner and controller START exit before another network attempt.
4. Retrieve the capped log and diagnose the iPhone connection attempt from RTSP/session events before changing advertised capabilities.
5. Re-test audio and iPhone RAOP behavior after the display/exit gate is stable.
6. Consider resident PRX lifecycle testing as a separate, reversible experiment.
7. Test audio coexistence per title only after resident lifecycle and standalone output are established.

## Explicit boundaries

- A successful build, package inspection, or PRX structural check does not prove hardware compatibility.
- No real iPhone session or AirPlay music playback has been verified. The user reports hearing the diagnostic tone once; that result is not an independent or sustained playback test.
- System background-music controls do not demonstrate arbitrary PCM mixing.
- Do not claim universal in-game audio support without separate results from multiple games.
