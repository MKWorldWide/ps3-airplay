# Roadmap

**Status: experimental; the maintainer confirms an earlier AIRPLAY02 run reached application startup and played the five-second tone, although its exact package identity is not established. AIRPLAY02 v0.1.2 and v0.1.3 later returned XMB error `80028F21` without a fresh log. The separate AIRTEST01 v0.1.0 silent diagnostic also returned the error and is parked. AIRPLAY02 v0.1.4 now builds locally from the earlier 2020 PSL1GHT runtime path with route-first network address selection and a bounded NetCtl fallback; it has not been transferred or hardware-tested.** This roadmap separates build evidence from behavior that still requires a PS3 and an iPhone.

## Milestones

| Milestone | Goal | Status |
|---|---|---|
| Build foundation | Reproducible PPU build and sample PRX structure check | Host/toolchain work complete; hardware compatibility unknown |
| Standalone audio | Launch the receiver and verify audible output at the system menu | Earlier AIRPLAY02 run reached startup and tone per maintainer; exact build unknown. v0.1.2/v0.1.3 and separate AIRTEST01 v0.1.0 each returned `80028F21`. v0.1.4 built locally; hardware test pending. |
| AirPlay interoperability | Discover the receiver and complete a real iPhone RAOP session | Earlier iPhone listing followed by connection failure; cause unconfirmed |
| Standalone playback | Stable playback, controls, reconnects, and a 30-minute session | Not tested |
| Resident module | Safely load, receive, stop, and unload a diagnostic PRX | Build checks only; no hardware test |
| Game coexistence | Determine whether audio can coexist with game audio | Separate feasibility gate; no evidence yet |
| Release | Documented recovery, privacy, licensing, reliability, and install/removal process | Not ready |

## Current implementation

Portable components cover bounded DNS-SD/mDNS, RTSP/SDP session handling, UDP RTP, timing probes, retransmission requests and validation, RAOP crypto, ALAC decoding, PCM buffering, and capped diagnostics. The standalone app plays a five-second, low-level 440 Hz startup diagnostic through its GameOS audio blocks before normal PCM rendering. The revised local screen candidate primes scanout with PSL1GHT's sample sequence and overlays a bright `DISPLAY ACTIVE` banner and START prompt over the maintainer's background. It polls controller START for orderly return to XMB. The screen code cross-builds in Lima; hardware rendering and exit behavior are unverified. The resident candidate decodes supported packets for diagnostics and discards PCM.

## Next steps

1. Keep AIRTEST01 parked; do not use it as a substitute for the receiver.
2. Diagnose the common pre-log XMB launch failure by comparing the earlier tone-producing AIRPLAY02 package/toolchain and exact current v0.1.4 SELF/package metadata. Treat `80028F21` as unresolved; public references reviewed do not map it to a specific cause.
3. Before deployment, bound the display's unbounded RSX waits and make sure controller exit remains available if graphics setup stalls.
4. Once a receiver build is safe to test, the maintainer's standing authorization allows package staging/installation without another approval request. First verify one launch, visible output, tone, and START-to-XMB; stop if launch or clean exit fails.
5. Retrieve a fresh log tied to the exact package. Attempt one iPhone connection and follow accept, RTSP method/status, session, SDP, and RTP stages.
6. Continue toward audible AirPlay only after fresh hardware evidence establishes the working boundary; keep protocol, decrypt, ALAC, PCM, and GameOS output claims separate.
7. Diagnose the receiver loopback test's repeated uninstrumented timing-probe hang before using it as a green regression gate.
8. Evaluate resident PRX lifecycle and per-title game audio only after standalone playback works; keep boot plugin configuration unchanged.
## Explicit boundaries

- A successful build, package inspection, or PRX structural check does not prove hardware compatibility.
- No real iPhone session or AirPlay music playback has been verified. The user reports hearing the diagnostic tone once; that result is not an independent or sustained playback test.
- System background-music controls do not demonstrate arbitrary PCM mixing.
- Do not claim universal in-game audio support without separate results from multiple games.
