# Roadmap

**Status: experimental; the maintainer reports one earlier AIRPLAY02 startup and five-second tone, but the exact package identity is unknown. AIRPLAY02 v0.1.2 and v0.1.3 later returned XMB error `80028F21`; AIRTEST01 v0.1.0 is parked. RPCS3 rejects v0.1.4, v0.1.5, and v0.1.6 before application startup. A fresh PS3DK build matches the v01.01 reference's ELF ABI and PS3 segment layout; this is the recovered build path, but its v0.1.7 package still needs an RPCS3 launch result.** See [build pipeline](BUILD_PIPELINE.md) and [versioning policy](VERSIONING.md).

## Milestones

| Milestone | Goal | Status |
|---|---|---|
| Build foundation | Reproducible PPU build and package pipeline aligned to known v01.01 artifact | ELF validator and canonical PS3DK/Lima pipeline added; v0.1.7 package/emulator check pending; hardware compatibility unknown |
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
2. Build v0.1.7 from the committed PS3DK/Lima pipeline and test it in RPCS3. The v01.01 and v0.1.7 candidate ELF layout now match on OSABI, executable type, entry point, and PS3-specific program-header flags; exact binary equality is not expected because source differs.
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
