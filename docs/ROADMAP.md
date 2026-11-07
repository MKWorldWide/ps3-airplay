# Roadmap

**Current status (2026-11-07):** The last physical PS3 evidence remains v0.1.35: 195 RTP gaps, no retained resend recoveries, zero ALAC failures, and 86,106 active-playback PCM underflow frames. In RPCS3, v0.1.38 / SFO 00.38 launched, rendered the image, completed the diagnostic tone, accepted an encrypted synthetic RAOP session, decrypted RTP, decoded ALAC, submitted PCM, and exited cleanly on keyboard START. That fixture had zero RTP gaps and zero ALAC failures; it does not exercise loss recovery or prove audible output. v0.1.39 adds an upper bound for silence concealment after a very large sequence jump. Full Lima sanitizer/PPU tests pass. Its package and emulator replay are the next step; no PS3 files were touched.

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

Portable components cover bounded DNS-SD/mDNS, RTSP/SDP session handling, UDP RTP, timing probes, retransmission requests and validation, RAOP crypto, ALAC decoding, PCM buffering, and capped diagnostics. The standalone app plays one five-second, low-level 440 Hz startup diagnostic through GameOS audio blocks before normal PCM rendering. Real Spotify audio over AirPlay was previously confirmed on v0.1.33, with crackle/dropouts and XMB-resume flicker. The latest capture identifies v0.1.35 as installed and records active starvation without ALAC decode failures. The resident candidate remains separate and discards decoded PCM.

## Next steps

1. Restore console reachability, then read installed SFO and the current app log over direct FTP/webMAN; only install after confirming the app is safely exited.
2. Upload the production port-5000 v0.1.37 PKG as a new versioned file over direct FTP, verify its remote bytes, and install through the established webMAN/XMB process. Keep the port-5001 RPCS3 package off-console.
3. On hardware, verify the background before/after two PS-menu resumes and a clean START-to-XMB exit. Compare RTP gaps, resend recovery, and active PCM underflows against v0.1.35; separately record whether the user hears fewer dropouts.
4. Retrieve the capped log immediately and verify installed SFO version before attributing counters.
5. Defer soak tests, resident PRX work, and game-audio mixing until short playback, XMB resume, and clean exit are stable.

### RTP loss handling and interoperability research (2026-11-07)

- The C queue previously released the earliest queued packet immediately even when its sequence skipped the next expected number. A later resend was then rejected as late. It now holds the hole for a bounded 80 ms, accepts a resend during that interval, and emits one zero-payload loss marker per missing ALAC frame after expiry so the app can insert exact-frame silence. Gaps larger than 128 packets skip directly to the next received frame after the same wait, preventing unbounded silence/latency.
- OpenAirPlay1 uses a 512-slot sequence buffer and forces loss handling after 128 packets of lead (about one second for 352-frame/44.1 kHz packets); it explicitly returns a lost-frame marker and has wraparound/reordering/retransmission tests. Shairport Sync uses a 1,024-packet buffer and documents 512 as its minimum (~4 seconds), reflecting a more latency-tolerant desktop receiver. The PS3's current 256-packet queue and 128-packet startup prebuffer trade lower memory for about one second of startup buffering; do not enlarge it without measuring the actual sender and console latency.
- Next: build/run v0.1.39 in RPCS3 with the 128-packet encrypted fixture and the deterministic host gap/recovery suite. On hardware when reachable, compare short Spotify sessions with and without XMB scrolling, and validate recovered packets, concealed/skipped gaps, PCM underflows, and user-heard artifacts. Tune the 80 ms wait and one-second prebuffer only from those measurements.
- Clock synchronization remains incomplete. OpenAirPlay1's clock model selects a low-RTT clock offset and maps an RTP timestamp anchor to the DAC; our current timing probe/RTT diagnostics do not yet provide this playback scheduling model. Keep clock mapping/drift correction as a measured follow-up after hardware RTP recovery tests.

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
- v0.1.35 combines v0.1.34's bounded resend recovery with corrected active-playback underflow accounting. Lima host/sanitizer and PPU/package validation pass; its finalized PKG is staged and byte-verified. The v0.1.33 title is still streaming, so install after clean exit. Then compare RTP gaps/recovery and valid underflow deltas on one short stream. Keep display resume separate.
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


## v0.1.37 audio and display work

The v0.1.35 capture aligns missing RTP packet duration with active PCM underruns and records no resend recoveries. v0.1.36 accepts valid in-window resend packets after their tracking entry expires. v0.1.37 pairs that transport recovery with a static-framebuffer policy: no continuous flips, one redraw/flip after menu resume, then an inactive-buffer refresh. This is a measured mitigation, not a proven display fix. Build, RPCS3 replay, and PS3 audio/display/exit tests remain required.
