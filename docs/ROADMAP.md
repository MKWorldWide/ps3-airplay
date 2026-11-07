# Roadmap

**Status: experimental; an earlier standalone run reportedly played its diagnostic tone but showed a black screen, and the iPhone listed the receiver but did not connect. Version 0.1.2 is locally verified but not transferred or hardware-tested. The console is currently unreachable for read-only preflight, and a fresh receiver loopback test is scheduling-sensitive.** This roadmap separates build evidence from behavior that still requires a PS3 and an iPhone.

## Milestones

| Milestone | Goal | Status |
|---|---|---|
| Build foundation | Reproducible PPU build and sample PRX structure check | Host/toolchain work complete; hardware compatibility unknown |
| Standalone audio | Launch the receiver and verify audible output at the system menu | Earlier five-second tone reported; screen output and clean exit unresolved; 0.1.2 local only |
| AirPlay interoperability | Discover the receiver and complete a real iPhone RAOP session | Earlier iPhone listing followed by connection failure; cause unconfirmed |
| Standalone playback | Stable playback, controls, reconnects, and a 30-minute session | Not tested |
| Resident module | Safely load, receive, stop, and unload a diagnostic PRX | Build checks only; no hardware test |
| Game coexistence | Determine whether audio can coexist with game audio | Separate feasibility gate; no evidence yet |
| Release | Documented recovery, privacy, licensing, reliability, and install/removal process | Not ready |

## Current implementation

Portable components cover bounded DNS-SD/mDNS, RTSP/SDP session handling, UDP RTP, timing probes, retransmission requests and validation, RAOP crypto, ALAC decoding, PCM buffering, and capped diagnostics. The standalone app plays a five-second, low-level 440 Hz startup diagnostic through its GameOS audio blocks before normal PCM rendering. The revised local screen candidate primes scanout with PSL1GHT's sample sequence and overlays a bright `DISPLAY ACTIVE` banner and START prompt over the maintainer's background. It polls controller START for orderly return to XMB. The screen code cross-builds in Lima; hardware rendering and exit behavior are unverified. The resident candidate decodes supported packets for diagnostics and discards PCM.

## Next steps

1. Restore console reachability, complete read-only disk/package/log checks, and confirm the installed AIRPLAY02 version and recovery route.
2. Review the unbounded RSX label/flip polling in the v0.1.2 source before deployment; decide whether to test that exact artifact or make a small timeout-only safety release.
3. Once preflight is complete, obtain approval for the exact package filename, version, hash, destination, and rollback procedure before transfer.
4. Check the `DISPLAY ACTIVE` banner and controller START exit; stop extended testing if exit fails.
5. Run the receiver loopback test repeatedly without tracing and resolve its timing-probe stall before treating the host suite as green.
6. Try the iPhone once and retrieve the capped log; use accept, method, status, state, SDP, and RTP events to locate the failure before changing capabilities.
7. Re-test AirPlay audio after display, clean exit, and RTSP/RTP stage evidence are established.
8. Consider resident PRX lifecycle testing as a separate, reversible experiment.
9. Test audio coexistence per title only after resident lifecycle and standalone output are established.

## Explicit boundaries

- A successful build, package inspection, or PRX structural check does not prove hardware compatibility.
- No real iPhone session or AirPlay music playback has been verified. The user reports hearing the diagnostic tone once; that result is not an independent or sustained playback test.
- System background-music controls do not demonstrate arbitrary PCM mixing.
- Do not claim universal in-game audio support without separate results from multiple games.
