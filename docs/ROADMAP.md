# Roadmap

**Status: experimental; v0.1.2 is installed, but XMB launch returns error `80028F21`. The installed EBOOT and PARAM.SFO match the verified package. The existing diagnostic log is from an earlier run, so display, tone, clean exit, and AirPlay behavior for v0.1.2 remain untested.** This roadmap separates build evidence from behavior that still requires a PS3 and an iPhone.

## Milestones

| Milestone | Goal | Status |
|---|---|---|
| Build foundation | Reproducible PPU build and sample PRX structure check | Host/toolchain work complete; hardware compatibility unknown |
| Standalone audio | Launch the receiver and verify audible output at the system menu | v0.1.2 launch fails with `80028F21`; earlier five-second tone is from an unidentified build; display and clean exit unresolved |
| AirPlay interoperability | Discover the receiver and complete a real iPhone RAOP session | Earlier iPhone listing followed by connection failure; cause unconfirmed |
| Standalone playback | Stable playback, controls, reconnects, and a 30-minute session | Not tested |
| Resident module | Safely load, receive, stop, and unload a diagnostic PRX | Build checks only; no hardware test |
| Game coexistence | Determine whether audio can coexist with game audio | Separate feasibility gate; no evidence yet |
| Release | Documented recovery, privacy, licensing, reliability, and install/removal process | Not ready |

## Current implementation

Portable components cover bounded DNS-SD/mDNS, RTSP/SDP session handling, UDP RTP, timing probes, retransmission requests and validation, RAOP crypto, ALAC decoding, PCM buffering, and capped diagnostics. The standalone app plays a five-second, low-level 440 Hz startup diagnostic through its GameOS audio blocks before normal PCM rendering. The revised local screen candidate primes scanout with PSL1GHT's sample sequence and overlays a bright `DISPLAY ACTIVE` banner and START prompt over the maintainer's background. It polls controller START for orderly return to XMB. The screen code cross-builds in Lima; hardware rendering and exit behavior are unverified. The resident candidate decodes supported packets for diagnostics and discards PCM.

## Next steps

1. Diagnose the XMB launch failure before another console launch. Verify NPDRM SELF metadata, application startup/link assumptions, and package contract against the installed file hashes; do not change CFW or system settings based on the undocumented error alone.
2. Build a minimal, reversible launch/display control using the same toolchain if static package checks do not identify the cause. Require a known recovery route before testing another build.
3. After a corrected build launches, run the visible display, five-second tone, and START-to-XMB checks. Stop if the screen is black or normal exit fails.
4. Run the receiver loopback test repeatedly without tracing and resolve its timing-probe stall before treating the host suite as green.
5. Try the iPhone once and retrieve the capped log; use accept, method, status, state, SDP, and RTP events to locate the failure before changing capabilities.
6. Re-test AirPlay audio after display, clean exit, and RTSP/RTP stage evidence are established.
7. Consider resident PRX lifecycle testing as a separate, reversible experiment.
8. Test audio coexistence per title only after resident lifecycle and standalone output are established.

## Explicit boundaries

- A successful build, package inspection, or PRX structural check does not prove hardware compatibility.
- No real iPhone session or AirPlay music playback has been verified. The user reports hearing the diagnostic tone once; that result is not an independent or sustained playback test.
- System background-music controls do not demonstrate arbitrary PCM mixing.
- Do not claim universal in-game audio support without separate results from multiple games.
