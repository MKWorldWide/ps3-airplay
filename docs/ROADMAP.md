# Roadmap

**Status: experimental; the maintainer reports one earlier AIRPLAY02 startup and five-second tone, but the exact package identity is unknown. AIRPLAY02 v0.1.2 and v0.1.3 later returned XMB error `80028F21`; AIRTEST01 v0.1.0 is parked. RPCS3 rejected v0.1.4–v0.1.6 before app startup. PS3DK-built v0.1.7–v0.1.9 installed and reached APP_START in RPCS3. v0.1.7/v0.1.8 failed RTSP bind with error 112 while emulator network status was disconnected. v0.1.9 then logged a bounded IP timeout while disconnected, and a bind error after route readiness when connected. The host had a wildcard listener on TCP 5000; a port collision remains a hypothesis. v0.1.10 tests port 5001 with default production port unchanged.** See [build pipeline](BUILD_PIPELINE.md) and [versioning policy](VERSIONING.md).

## Milestones

| Milestone | Goal | Status |
|---|---|---|
| Build foundation | Reproducible PPU build and package pipeline aligned to known v01.01 artifact | ELF and package validators added; v0.1.7 installs and reaches APP_START in RPCS3. Finalized-package extraction ordering corrected for v0.1.8; hardware compatibility unknown |
| Standalone audio | Launch the receiver and verify audible output at the system menu | Earlier AIRPLAY02 run reached startup and tone per maintainer; exact build unknown. v0.1.2/v0.1.3 and separate AIRTEST01 v0.1.0 returned `80028F21`; RPCS3 rejects current v0.1.4–v0.1.6 before app startup. |
| Diagnostic application | Reliable launch/exit, logs, basic settings and diagnostic toggles | Target for v0.2.0; not achieved |
| Stable AirPlay audio | Discovery, connection, streamed audio, controls, reconnect, and sustained playback on PS3/iPhone | Target for v0.3.0; not achieved |
| AirPlay interoperability | Discover the receiver and complete a real iPhone RAOP session | Earlier iPhone listing followed by connection failure; cause unconfirmed |
| Standalone playback | Stable playback, controls, reconnects, and a 30-minute session | Not tested |
| Resident module | Safely load, receive, stop, and unload a diagnostic PRX | Build checks only; no hardware test |
| Game coexistence | Determine whether audio can coexist with game audio | Separate feasibility gate; no evidence yet |
| Release | Documented recovery, privacy, licensing, reliability, and install/removal process | Not ready |

## Current implementation

Portable components cover bounded DNS-SD/mDNS, RTSP/SDP session handling, UDP RTP, timing probes, retransmission requests and validation, RAOP crypto, ALAC decoding, PCM buffering, and capped diagnostics. The standalone app plays a five-second, low-level 440 Hz startup diagnostic through its GameOS audio blocks before normal PCM rendering. The revised local screen candidate primes scanout with PSL1GHT's sample sequence and overlays a bright `DISPLAY ACTIVE` banner and START prompt over the maintainer's background. It polls controller START for orderly return to XMB. The screen code cross-builds in Lima; hardware rendering and exit behavior are unverified. The resident candidate decodes supported packets for diagnostics and discards PCM.

## Next steps

1. Keep AIRTEST01 parked; do not use it as a substitute for the receiver.
2. Build and validate v0.1.10 on port 5001 in RPCS3 to isolate the occupied host port 5000; check whether listener, mDNS, and UDP sockets reach ready. The normal receiver build default remains port 5000.
3. Before hardware deployment, bound the display's unbounded RSX waits and make sure controller exit remains available if graphics setup stalls.
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
