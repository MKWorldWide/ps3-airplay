# Roadmap

**Status: experimental; v0.1.2 is installed, but XMB launch returns error `80028F21`. A separately built v0.1.3 candidate uses the 2020 PSL1GHT runtime and passes host build/package checks. Its launch behavior on Evilnat CFW is unknown; it has not been transferred or installed.** This roadmap separates build evidence from behavior that still requires a PS3 and an iPhone.

## Milestones

| Milestone | Goal | Status |
|---|---|---|
| Build foundation | Reproducible PPU build and sample PRX structure check | Host/toolchain work complete; hardware compatibility unknown |
| Standalone audio | Launch the receiver and verify audible output at the system menu | v0.1.2 launch fails with `80028F21`. v0.1.3 legacy-runtime candidate is ready for an approved controlled launch test; earlier five-second tone came from an unidentified build. |
| AirPlay interoperability | Discover the receiver and complete a real iPhone RAOP session | Earlier iPhone listing followed by connection failure; cause unconfirmed |
| Standalone playback | Stable playback, controls, reconnects, and a 30-minute session | Not tested |
| Resident module | Safely load, receive, stop, and unload a diagnostic PRX | Build checks only; no hardware test |
| Game coexistence | Determine whether audio can coexist with game audio | Separate feasibility gate; no evidence yet |
| Release | Documented recovery, privacy, licensing, reliability, and install/removal process | Not ready |

## Current implementation

Portable components cover bounded DNS-SD/mDNS, RTSP/SDP session handling, UDP RTP, timing probes, retransmission requests and validation, RAOP crypto, ALAC decoding, PCM buffering, and capped diagnostics. The standalone app plays a five-second, low-level 440 Hz startup diagnostic through its GameOS audio blocks before normal PCM rendering. The revised local screen candidate primes scanout with PSL1GHT's sample sequence and overlays a bright `DISPLAY ACTIVE` banner and START prompt over the maintainer's background. It polls controller START for orderly return to XMB. The screen code cross-builds in Lima; hardware rendering and exit behavior are unverified. The resident candidate decodes supported packets for diagnostics and discards PCM.

## Next steps

1. With approval, transfer v0.1.3 under a versioned filename, install it through XMB, and test whether it launches. This candidate is based on an upstream report that modern PSL1GHT runtime builds failed before `main()` on 4.93 HEN while a 2020 runtime build worked; that report does not cover Evilnat CFW, so treat the result as a controlled test, not a proven fix.
2. If launch succeeds, verify visible display, five-second tone, and START-to-XMB twice. Stop if output is black or normal exit fails.
3. Try the iPhone once and retrieve the capped log; use accept, method, status, state, SDP, and RTP events to locate the failure before changing capabilities.
4. Re-test AirPlay audio after display, clean exit, and RTSP/RTP stage evidence are established.
5. Repeat the receiver loopback test without tracing; this run passed, but the earlier untraced timing-probe stall remains an intermittent host-test concern.
6. Consider resident PRX lifecycle testing as a separate, reversible experiment.
7. Test audio coexistence per title only after resident lifecycle and standalone output are established.

## Explicit boundaries

- A successful build, package inspection, or PRX structural check does not prove hardware compatibility.
- No real iPhone session or AirPlay music playback has been verified. The user reports hearing the diagnostic tone once; that result is not an independent or sustained playback test.
- System background-music controls do not demonstrate arbitrary PCM mixing.
- Do not claim universal in-game audio support without separate results from multiple games.
