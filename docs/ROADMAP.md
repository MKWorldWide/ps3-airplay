# Roadmap

**Status: experimental; the standalone receiver played its diagnostic tone on PS3 hardware, but showed a black screen and did not complete an iPhone connection. The screen-enabled candidate builds locally and awaits hardware validation.** This roadmap separates build evidence from behavior that still requires a PS3 and an iPhone.

## Milestones

| Milestone | Goal | Status |
|---|---|---|
| Build foundation | Reproducible PPU build and sample PRX structure check | Host/toolchain work complete; hardware compatibility unknown |
| Standalone audio | Launch the receiver and verify audible output at the system menu | Five-second tone heard on hardware; screen was black; latest screen-enabled candidate not yet installed |
| AirPlay interoperability | Discover the receiver and complete a real iPhone RAOP session | iPhone lists `ps3-airplay-diagnostic`; selecting it fails to connect |
| Standalone playback | Stable playback, controls, reconnects, and a 30-minute session | Not tested |
| Resident module | Safely load, receive, stop, and unload a diagnostic PRX | Build checks only; no hardware test |
| Game coexistence | Determine whether audio can coexist with game audio | Separate feasibility gate; no evidence yet |
| Release | Documented recovery, privacy, licensing, reliability, and install/removal process | Not ready |

## Current implementation

Portable components cover bounded DNS-SD/mDNS, RTSP/SDP session handling, UDP RTP, timing probes, retransmission requests and validation, RAOP crypto, ALAC decoding, PCM buffering, and capped diagnostics. The standalone app plays a five-second, low-level 440 Hz startup diagnostic through its GameOS audio blocks before normal PCM rendering. The screen-enabled candidate initializes an RSX framebuffer, draws the maintainer's background and an exit prompt, and polls controller START for orderly return to XMB. It cross-builds and packages in Lima; hardware rendering and exit behavior are unverified. The resident candidate decodes supported packets for diagnostics and discards PCM.

## Next steps

1. Improve RTP jitter handling and sender-clock scheduling with bounded memory and measurable host tests.
2. Review the new package digest before replacing the staged PKG; install from XMB and check rendering plus controller START exit.
3. Retrieve the capped log and diagnose the iPhone connection attempt from the RTSP/session events before changing advertised capabilities.
4. Re-test audio and iPhone RAOP behavior after the display/exit gate is stable.
5. Consider resident PRX lifecycle testing as a separate, reversible experiment.
6. Test audio coexistence per title only after resident lifecycle and standalone output are established.

## Explicit boundaries

- A successful build, package inspection, or PRX structural check does not prove hardware compatibility.
- No real iPhone session or audible output has been verified.
- System background-music controls do not demonstrate arbitrary PCM mixing.
- Do not claim universal in-game audio support without separate results from multiple games.
