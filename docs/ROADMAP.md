# Roadmap

**Current status (2026-11-07):** AIRPLAY02 v0.1.25 runs in RPCS3 when built with the emulator-only TCP 5001 override. It opens RTP 6000–6002, completes the diagnostic-tone code path, and answers local `OPTIONS` with `200 OK`; mDNS multicast-interface setup fails. A cross-host synthetic RAOP probe then reached `ANNOUNCE`, but RPCS3 did not answer within three seconds and the last reliable log event was `RTSP_ANNOUNCE`. v0.1.26 adds privacy-safe SDP parse and RSA key-unwrapping phase events plus a reusable sender probe for 128 RTP fixture packets; full Lima host/sanitizer tests and PPU component compilation pass. Package/runtime test of v0.1.26 is next. No iPhone session or audible streamed music is proven. The desktop remained locked, so display pixels and controller exit remain unverified. Physical PS3 state and boot plugins were not changed. See [build pipeline](BUILD_PIPELINE.md), [test matrix](TEST_MATRIX.md), and [versioning policy](VERSIONING.md).

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
2. RPCS3 networking needs to be Connected for NetCtl to obtain a route. Keep production builds on TCP 5000; use `RTSP_PORT=5001` only for emulator probes when host port 5000 is occupied. Do not stop or alter the host mDNS responder. Use v0.1.26 phase logs to isolate the stalled ANNOUNCE before attempting RTP again.
3. Before hardware deployment, bound the display's unbounded RSX waits and make sure controller exit remains available if graphics setup stalls.
4. Once a receiver build is safe to test, the maintainer's standing authorization allows package staging/installation without another approval request. First verify one launch, visible output, tone, and START-to-XMB; stop if launch or clean exit fails.
5. Retry the synthetic ANNOUNCE probe against v0.1.26 and inspect SDP/key-unwrapping phase events. Retry RPCS3 with the desktop unlocked for visible-screen and controller-exit evidence. Then test mDNS on real PS3 hardware; for the emulator, investigate its multicast-interface failure without changing host-wide services. Attempt one iPhone connection only when discovery is advertised.
6. Continue toward audible AirPlay only after fresh hardware evidence establishes the working boundary; keep protocol, decrypt, ALAC, PCM, and GameOS output claims separate.
7. Repeat the latest uninstrumented receiver loopback/timing pass to confirm the earlier intermittent timing-probe hang is no longer reproducible.
8. Evaluate resident PRX lifecycle and per-title game audio only after standalone playback works; keep boot plugin configuration unchanged.

### Current protocol hardening (2026-11-07)

- ASan found and the receiver now guards ALAC in-band frame sample counts against the negotiated maximum before sample-array writes.
- The new host composition test covers RSA-OAEP key recovery, encrypted RTP payload decryption, ALAC decode, and reference PCM equality in one run. It does not substitute for the existing socket loopback or any iPhone/console test.
- AIRPLAY02 v0.1.20 is built from commit `8a4e641d114e1a613421b9ffb51f8615ad851827`; Lima host sanitizer tests, PPU link, ELF validation, and finalized package checks pass. RPCS3 UI remains inaccessible while the Mac is locked. Physical PS3 remains a separate, approval-gated test.
- Expanded the sanitizer corpus to mutate a valid ALAC payload after its fixed frame header; 4,096 seeded mutations plus 256 random inputs pass ASan/UBSan halt-on-error. This improves decoder malformed-input coverage but is not AirPlay sender or playback evidence.
- Added a host integration test that completes fragmented RTSP ANNOUNCE/SETUP/RECORD over TCP, receives encrypted RTP over UDP, decrypts and decodes the fixture frame, and pushes 353 stereo frames into the PCM ring. This is simulated sender evidence only.
- A direct `--no-gui` launch attempt against the extracted v0.1.20 EBOOT did not refresh the app log or produce `APP_START`; retain it as inconclusive. The installed emulator title remains 0.1.15.
- Display follow-up: the user reports a visible image and successful app opening but still sees blue framing and `DISPLAY ACTIVE`. The current source and v0.1.20 ancestry already remove blue diagnostics and all text; no duplicate code patch is warranted without matching the observed package. The local desktop bridge continues to report locked.
- Research found the RAOP TXT `vs` value is the receiver server version. The service advertised stale `vs=0.1.2` while RTSP identifies `AirTunes/105.1`; the v0.1.21 source now advertises `vs=105.1`, with a wire-format regression assertion.
- The full Lima host/sanitizer and PPU component suite passes after this metadata fix. The v0.1.21 standalone ELF/PKG also validates in Lima; the exact package is preserved locally. RPCS3 installation/launch remains pending because desktop automation reports the Mac is locked.
- v0.1.22 adds a bounded success checkpoint for RTP payload decryption; the encrypted RTSP/RTP-to-PCM host integration now asserts this stage. The complete Lima host/sanitizer and PPU object suite passes.
- The v0.1.22 standalone PPU ELF and finalized NPDRM PKG validate from pushed source `7b44acf`; the artifact is preserved locally. The RPCS3 app-data log remains an older v0.1.15 record and the desktop is locked, so runtime validation is pending.
- v0.1.23 source bounds accepted-client RTSP writes with a two-second socket send timeout and overall response deadline. The full Lima host/sanitizer and PPU component suite passes, including a slow-reader backpressure test that verifies timeout logging and disconnect. Its finalized package also validates in Lima; RPCS3 runtime remains pending because the desktop is locked.
- v0.1.24 adds bounded 128-packet RTP health checkpoints for loss, reordering, duplicates, late arrivals, queue drops, payload throughput, and queue depth. The Lima integration/sanitizer suite, PPU component compile, standalone ELF, and finalized PKG validate. RPCS3 runtime remains pending because desktop access still reports locked.
- Historical next-validation note above is superseded by the current RPCS3 probe below.

## Latest update (2026-11-07)

- AIRPLAY02 v0.1.14 was installed and launched in RPCS3. The fresh capped log reached screen/controller and NetCtl initialization, then `NET_IP_TIMEOUT` and `APP_STOP result=1`. It did not reach crypto, receiver listeners, mDNS, or audio. The package is structurally validated, but its SO_REUSEPORT discovery change was not exercised.
- AIRPLAY02 v0.1.15 includes bounded RTSP FLUSH propagation across RTP and PCM buffers. Full Lima tests, PPU application build, and package validation passed; RPCS3 installed/booted the package but again timed out at address acquisition (`EADDRNOTAVAIL`) before listeners. FLUSH is host-tested only.
- AIRPLAY02 v0.1.16 now parses RAOP `SET_PARAMETER` volume, clamps output to attenuation only, maps `-144 dB` to mute, and ramps PCM gain across audio blocks. Full Lima host tests, PPU objects, standalone ELF and finalized PKG validation pass. Package SHA-256 is recorded in `docs/VERSIONING.md`; no RPCS3 or physical output result yet.
- v0.1.17 removes the remaining `PRESS START TO EXIT` overlay so the background image is unobstructed. Full Lima host tests, PPU build, ELF validation, and finalized package checks pass; the artifact is recorded in `docs/VERSIONING.md`. The RPCS3 UI remains unavailable, so visual/runtime verification is pending.
- v0.1.18 adds five-second PCM buffer/frame-flow diagnostics with numeric, privacy-safe counters. Full Lima host tests, PPU build, ELF validation, and finalized package checks pass. Runtime log behavior is unverified because RPCS3 UI access remains unavailable; no audible output is claimed.
- Historical next-validation note above is superseded by the current RPCS3 probe below.

### RPCS3 network and RTSP probe (2026-11-07)

- v0.1.24 run with RPCS3 Internet disconnected reached `NET_IP_TIMEOUT` and `APP_STOP`. With RPCS3 Internet set to Connected, NetCtl obtained a route but TCP 5000 bind failed because the host port was occupied.
- v0.1.25 was built from source `a59aa937401d432b5cc6f740eeda15bf0db091ba` with the existing emulator-only `RTSP_PORT=5001` override. RTSP 5001 and RTP 6000–6002 opened; mDNS failed at interface setup (stage 6, error 125). A local OPTIONS request received RTSP `200 OK`; the log recorded accept, OPTIONS, 200, and peer close.
- The five-second diagnostic tone completed in the app log. Audible tone, display pixels, controller START, clean app teardown, iPhone discovery, RAOP session, and streamed music remain unverified in this run. The RPCS3 desktop was locked, preventing visual and controller observation.
- Latest package version consumed: 0.1.25. v0.1.26 source diagnostics are committed; package validation and runtime replay remain next. Physical PS3 and boot plugin configuration were unchanged.
## Explicit boundaries

- A successful build, package inspection, or PRX structural check does not prove hardware compatibility.
- No real iPhone session or AirPlay music playback has been verified. The user reports hearing the diagnostic tone once; that result is not an independent or sustained playback test.
- System background-music controls do not demonstrate arbitrary PCM mixing.
- Do not claim universal in-game audio support without separate results from multiple games.
