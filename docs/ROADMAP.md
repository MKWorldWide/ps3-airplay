# Roadmap

**Status: experimental; the maintainer reports one earlier AIRPLAY02 startup and five-second tone, but the exact package identity is unknown. AIRPLAY02 v0.1.2 and v0.1.3 later returned XMB error `80028F21`; AIRTEST01 v0.1.0 is parked. RPCS3 rejected v0.1.4–v0.1.6 before app startup. PS3DK-built v0.1.7–v0.1.9 installed and reached APP_START in RPCS3. v0.1.7/v0.1.8 failed RTSP bind with error 112 while emulator network status was disconnected. v0.1.9 then logged a bounded IP timeout while disconnected, and a bind error after route readiness when connected. v0.1.10–v0.1.12 bind RTSP on TCP 5001 and RTP on UDP 6000–6002. v0.1.12 then failed mDNS UDP bind on 5353 with `EADDRINUSE`; the host has a UDP listener on that port. The app screen is visible in RPCS3, while physical PS3 display/exit remain unverified.** See [build pipeline](BUILD_PIPELINE.md) and [versioning policy](VERSIONING.md).

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

Portable components cover bounded DNS-SD/mDNS, RTSP/SDP session handling, UDP RTP, timing probes, retransmission requests and validation, RAOP crypto, ALAC decoding, PCM buffering, and capped diagnostics. The standalone app plays a five-second, low-level 440 Hz startup diagnostic through its GameOS audio blocks before normal PCM rendering. The local screen candidate primes scanout with PSL1GHT's sample sequence and shows the background with a white START-to-exit prompt. v0.1.11 visibly rendered this in RPCS3; physical output remains unverified. It polls controller START for orderly return to XMB. The screen code cross-builds in Lima; hardware rendering and exit behavior are unverified. The resident candidate decodes supported packets for diagnostics and discards PCM.

## Next steps

1. Keep AIRTEST01 parked; do not use it as a substitute for the receiver.
2. Stage 4/errno 112 identifies the RPCS3 failure as the mDNS UDP 5353 bind colliding with a host listener. Do not stop or alter Lima's listener during builds. Continue RAOP discovery testing on physical PS3 after its deployment approval gate; keep RPCS3 evidence separate.
3. Before hardware deployment, bound the display's unbounded RSX waits and make sure controller exit remains available if graphics setup stalls.
4. Once a receiver build is safe to test, the maintainer's standing authorization allows package staging/installation without another approval request. First verify one launch, visible output, tone, and START-to-XMB; stop if launch or clean exit fails.
5. Retrieve a fresh log tied to the exact package. Attempt one iPhone connection and follow accept, RTSP method/status, session, SDP, and RTP stages.
6. Continue toward audible AirPlay only after fresh hardware evidence establishes the working boundary; keep protocol, decrypt, ALAC, PCM, and GameOS output claims separate.
7. Repeat the latest uninstrumented receiver loopback/timing pass to confirm the earlier intermittent timing-probe hang is no longer reproducible.
8. Evaluate resident PRX lifecycle and per-title game audio only after standalone playback works; keep boot plugin configuration unchanged.

### Current protocol hardening (2026-11-07)

- ASan found and the receiver now guards ALAC in-band frame sample counts against the negotiated maximum before sample-array writes.
- The new host composition test covers RSA-OAEP key recovery, encrypted RTP payload decryption, ALAC decode, and reference PCM equality in one run. It does not substitute for the existing socket loopback or any iPhone/console test.
- AIRPLAY02 v0.1.20 is built from commit `8a4e641d114e1a613421b9ffb51f8615ad851827`; Lima host sanitizer tests, PPU link, ELF validation, and finalized package checks pass. RPCS3 UI remains inaccessible while the Mac is locked. Physical PS3 remains a separate, approval-gated test.
- Next: when desktop access is restored, install/run the exact v0.1.20 package in RPCS3 and collect its version-matched log. Then continue with the first controlled iPhone session; classify evidence by RTSP, RTP, decrypt/decode, PCM submission, and audible output. Next package version is 0.1.21 only after a new implementation pass.

## Latest update (2026-11-07)

- AIRPLAY02 v0.1.14 was installed and launched in RPCS3. The fresh capped log reached screen/controller and NetCtl initialization, then `NET_IP_TIMEOUT` and `APP_STOP result=1`. It did not reach crypto, receiver listeners, mDNS, or audio. The package is structurally validated, but its SO_REUSEPORT discovery change was not exercised.
- AIRPLAY02 v0.1.15 includes bounded RTSP FLUSH propagation across RTP and PCM buffers. Full Lima tests, PPU application build, and package validation passed; RPCS3 installed/booted the package but again timed out at address acquisition (`EADDRNOTAVAIL`) before listeners. FLUSH is host-tested only.
- AIRPLAY02 v0.1.16 now parses RAOP `SET_PARAMETER` volume, clamps output to attenuation only, maps `-144 dB` to mute, and ramps PCM gain across audio blocks. Full Lima host tests, PPU objects, standalone ELF and finalized PKG validation pass. Package SHA-256 is recorded in `docs/VERSIONING.md`; no RPCS3 or physical output result yet.
- v0.1.17 removes the remaining `PRESS START TO EXIT` overlay so the background image is unobstructed. Full Lima host tests, PPU build, ELF validation, and finalized package checks pass; the artifact is recorded in `docs/VERSIONING.md`. The RPCS3 UI remains unavailable, so visual/runtime verification is pending.
- v0.1.18 adds five-second PCM buffer/frame-flow diagnostics with numeric, privacy-safe counters. Full Lima host tests, PPU build, ELF validation, and finalized package checks pass. Runtime log behavior is unverified because RPCS3 UI access remains unavailable; no audible output is claimed.
- Next validation: install/launch v0.1.17 in RPCS3 and inspect its versioned startup log when UI access is available, then test the exact v0.1.18 telemetry package. The saved network setting remains Disconnected; any route change needs separate action-time confirmation. After the display/exit check, continue toward an AirPlay session. Physical PS3 remains gated on safe recovery. Next package: v0.1.19.
## Explicit boundaries

- A successful build, package inspection, or PRX structural check does not prove hardware compatibility.
- No real iPhone session or AirPlay music playback has been verified. The user reports hearing the diagnostic tone once; that result is not an independent or sustained playback test.
- System background-music controls do not demonstrate arbitrary PCM mixing.
- Do not claim universal in-game audio support without separate results from multiple games.
