# Roadmap

**Current status (2026-11-07):** AIRPLAY02 v0.1.33 / SFO 00.33 is installed on the demonstration PS3. The maintainer confirms AirPlay music from multiple Spotify tracks is audible for the first time, but crackles and sounds unstable/low fidelity. The screen flickers, especially after XMB resume. The retrieved log confirms encrypted 44.1 kHz stereo RTP, successful ALAC decode and PCM generation; its retained segment also reports repeated RTP sequence gaps and zero ALAC failures. v0.1.34 adds bounded resend queuing/tracking and less chatty checkpoints; the full Lima host suite passes. The new PKG and hardware retest are pending. First playback is a breakthrough, not stable AirPlay. See the [hardware report](reports/2026-11-07_v0133_first_airplay_playback.md), [test matrix](TEST_MATRIX.md), and [versioning policy](VERSIONING.md).

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

Portable components cover bounded DNS-SD/mDNS, RTSP/SDP session handling, UDP RTP, timing probes, retransmission requests and validation, RAOP crypto, ALAC decoding, PCM buffering, and capped diagnostics. The standalone app plays one five-second, low-level 440 Hz startup diagnostic through GameOS audio blocks before normal PCM rendering. v0.1.33 music playback is confirmed by the maintainer; the static background flickers after XMB resume. It polls controller START for orderly return to XMB; the latest observed run exited with clean cleanup. The resident candidate decodes supported packets for diagnostics and discards PCM.

## Next steps

1. Finish and validate v0.1.34's bounded RTP retransmission queue and delayed-response tracking, then build a new package with SFO 00.34.
2. Install and run a short controlled stream. Compare missing, requested, recovered, late and dropped packets with the v0.1.33 baseline; ask the maintainer whether crackling improves.
3. Test the screen separately: launch, open PS menu, resume twice, verify stable background, then START-to-XMB. The captured log shows no presentation error but cannot prove a stable HDMI image.
4. Preserve a fresh log immediately after the controlled stream and before relaunch; the 16 KiB log wraps and startup/session negotiation can be lost.
5. If audio remains poor despite low RTP loss, measure audio underflows, sender-clock drift and output sample handling before attempting a decoder rewrite. Do not change the conservative output gain without a measured need.
6. Keep soak testing, resident PRX work, and per-title game audio mixing deferred until short playback, XMB resume and clean exit are stable.

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

### Audio-quality follow-up (2026-11-07)

- A fresh live log and byte comparison identify the active PS3 build as v0.1.33. The receiver obtained 44.1 kHz encrypted ALAC and decoded without reported ALAC failures; the retained log has 88 missing RTP sequence numbers in 1,664 received packets. User reports crackling increases while scrolling XMB. Source inspection found its 51,200-frame underflow counter includes intentional startup prebuffer silence.
- v0.1.35 combines v0.1.34's bounded resend recovery with corrected active-playback underflow accounting. The current title was still running during the log snapshot. Install after clean exit, then compare RTP gaps/recovery and valid underflow deltas on one short stream. Keep display resume separate.
- Do not attribute the crackling to Spotify's Lossless option yet. The RAOP log reports the AirPlay transport format, not Spotify's source format. If v0.1.35 still crackles, A/B Spotify quality with the same track and network while correlating RTP gaps and PCM underflows.

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
- Latest package version consumed: 0.1.28. v0.1.29 fixes long-event logging before another replay. Physical PS3 and boot plugin configuration were unchanged.
## Explicit boundaries

- A successful build, package inspection, or PRX structural check does not prove hardware compatibility.
- No real iPhone session or AirPlay music playback has been verified. The user reports hearing the diagnostic tone once; that result is not an independent or sustained playback test.
- System background-music controls do not demonstrate arbitrary PCM mixing.
- Do not claim universal in-game audio support without separate results from multiple games.

## v0.30 compatibility candidate

Release label 0.30 maps to PS3 SFO 00.30. Packaging uses the preserved v01.01 type-1 debug container. Older three-component SFO values are now rejected. Production RTSP remains 5000; hardware launch/display/exit and actual AirPlay playback require fresh evidence. See the current decision log and memory for validation boundaries.

## Hardware evidence after SFO correction

The historical 0.30 build starts and plays its tone on the PS3; START exits while idle (maintainer report). A real iPhone attempt ends in an unanswered OPTIONS request and no completed RAOP session. No streamed music. START and PS input become unresponsive during that attempt. Corrected release label: 0.1.30; SFO remains 00.30. Resolve the request-handling stall before extended playback tests.

## v0.1.32 checkpoint

Big-endian PCM bytes and paced target packet/decode/PCM path now validate in RPCS3. Candidate is installed. Regular scanout and input-first loop target the reported PS/START issue; hardware overlay/exit confirmation remains required. Graphics waits are bounded and idle logging no longer destroys connection history as quickly. No extended soak until physical controls and clean exit pass.
