# Roadmap

**Status: experimental; v0.1.3 is installed and XMB launch returns the same `80028F21` error as v0.1.2. Installed SFO and EBOOT match the extracted package, while the diagnostic log is unchanged. The 2020 PSL1GHT runtime did not solve the launch failure.** This roadmap separates build evidence from behavior that still requires a PS3 and an iPhone.

## Milestones

| Milestone | Goal | Status |
|---|---|---|
| Build foundation | Reproducible PPU build and sample PRX structure check | Host/toolchain work complete; hardware compatibility unknown |
| Standalone audio | Launch the receiver and verify audible output at the system menu | v0.1.2 and v0.1.3 both fail at XMB with `80028F21`. Earlier five-second tone came from an unidentified build. |
| AirPlay interoperability | Discover the receiver and complete a real iPhone RAOP session | Earlier iPhone listing followed by connection failure; cause unconfirmed |
| Standalone playback | Stable playback, controls, reconnects, and a 30-minute session | Not tested |
| Resident module | Safely load, receive, stop, and unload a diagnostic PRX | Build checks only; no hardware test |
| Game coexistence | Determine whether audio can coexist with game audio | Separate feasibility gate; no evidence yet |
| Release | Documented recovery, privacy, licensing, reliability, and install/removal process | Not ready |

## Current implementation

Portable components cover bounded DNS-SD/mDNS, RTSP/SDP session handling, UDP RTP, timing probes, retransmission requests and validation, RAOP crypto, ALAC decoding, PCM buffering, and capped diagnostics. The standalone app plays a five-second, low-level 440 Hz startup diagnostic through its GameOS audio blocks before normal PCM rendering. The revised local screen candidate primes scanout with PSL1GHT's sample sequence and overlays a bright `DISPLAY ACTIVE` banner and START prompt over the maintainer's background. It polls controller START for orderly return to XMB. The screen code cross-builds in Lima; hardware rendering and exit behavior are unverified. The resident candidate decodes supported packets for diagnostics and discards PCM.

## Next steps

1. Add a minimal native PS3DK standalone launch control without PSL1GHT, using the existing Lima toolchain. The current `build-lima-smoke.sh` only validates receiver objects and a PRX sample, so it is not this test. Do not transfer the control without separate approval.
2. Check whether a licensed Sony PS3 SDK 4.75 installation and its Windows build VM are available. The public Thermal Bench project says it uses that SDK and MSBuild in a Windows 7 VM; this is a separate, proprietary toolchain and not a drop-in replacement.
3. Fix the hard-coded `PKG_VERSION_0.1.2` startup event so future logs identify the actual package version.
4. After a build launches, verify visible display, five-second tone, and START-to-XMB twice. Stop if output is black or normal exit fails.
5. Try the iPhone once and retrieve the capped log; use accept, method, status, state, SDP, and RTP events to locate the failure before changing capabilities.
6. Re-test AirPlay audio after display, clean exit, and RTSP/RTP stage evidence are established.
7. Repeat the receiver loopback test without tracing; this run passed, but the earlier untraced timing-probe stall remains an intermittent host-test concern.
8. Consider resident PRX lifecycle testing as a separate, reversible experiment.
9. Test audio coexistence per title only after resident lifecycle and standalone output are established.
## Explicit boundaries

- A successful build, package inspection, or PRX structural check does not prove hardware compatibility.
- No real iPhone session or AirPlay music playback has been verified. The user reports hearing the diagnostic tone once; that result is not an independent or sustained playback test.
- System background-music controls do not demonstrate arbitrary PCM mixing.
- Do not claim universal in-game audio support without separate results from multiple games.
