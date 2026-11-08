# Risks and safety gates

| Risk | Impact | Mitigation / evidence needed |
| Logger writer thread fails or is starved | Events can be dropped, or the capped log may miss its latest buffered tail | Fixed 128-record queue, explicit overrun/timing counters, low-priority worker, bounded 500 ms shutdown drain; validate on real PS3 and inspect queue depth/writer errors. |
| Queue resynchronization discards recoverable audio | A transient burst may cause a short audible gap | Trigger only at full queue and more than 128 packets of live sequence lag; preserve drop/resync counters; exercise jitter, burst, sequence wrap, FLUSH, and reconnect on host before hardware. |
| XMB artwork or callback rendering conflicts with VSH composition | The PS menu could obscure the app or resume to stale/dim frames | Keep title framebuffer presentation bounded and never draw over VSH; confirm repeated PS-menu cycles on hardware before changing compositing behavior. |
| Supplied artwork carries source metadata | Personal metadata could enter the public repository | Store a lossless PNG of the decoded RGB pixels with source metadata removed; verify exact pixel equality and derivative checksums. |
|---|---|---|
| Game audio cannot mix with external PCM | In-game background audio goal may not be achievable | Treat as a separate feasibility gate; test per title and make no universal claim |
| Resident module lifecycle is incompatible or unstable | Freeze, crash, or loss of console access | Do not load until exact compatibility and a tested recovery path are established; never use boot loading for initial tests |
| Standalone and resident SDK assumptions differ | Link/load failure or instability | Validate application and PRX paths independently |
| RAOP behavior varies across senders | Discovery or connection may fail | Keep protocol bounded, log sanitized state, and test actual sender versions |
| A connected sender stops reading RTSP replies | Blocking writes could stall network/audio/controller polling | Accepted sockets use a two-second send timeout and total response deadline. A host slow-reader backpressure test confirms disconnect within four seconds; PS3 stack/runtime behavior remains pending. |
| RTP loss or queue pressure is hidden by receive-only telemetry | Audio can fail later in the pipeline without enough evidence to locate packet loss, reordering, late data, or queue saturation | v0.1.24 emits privacy-safe 128-packet aggregate checkpoints; validate the exact event behavior in RPCS3/PS3 and keep logs capped. |
| Port or multicast conflicts | Service may not start or could disrupt another service | Check port ownership and multicast behavior before a controlled runtime test |
| Audio buffering/clock behavior is incomplete | Dropouts, latency, or drift | Add timed jitter handling and sender-clock mapping; measure on hardware |
| Diagnostic service is mistaken for playback-ready software | Silent or failed connections | Label capability accurately and gate claims on iPhone and audible-output evidence |
| RTSP response is logged as sent but probe client sees EOF | iPhone may list the service but fail during session setup | v0.1.39 target logged RTSP OPTIONS status 200 after send_all success; bounded client saw EOF. Preserve both observations and resolve response framing or connection teardown before changing protocol behavior speculatively. |
| Startup log is treated as proof of visible or audible output | Display and tone acceptance could be falsely marked complete | v0.1.39 fresh log reaches SCREEN_READY and AUDIO_TEST_TONE_COMPLETE; these are code-path checkpoints only. Require direct user confirmation of image, tone, stable display, and clean XMB return. |
| Controller presence is mistaken for button input | Start-to-exit failure may be misdiagnosed as disconnected controller or frozen app | `PAD_HEALTH` records successful polling and a connected port only; it omits button masks/transitions. Add privacy-safe input transition counters and compare with SYSUTIL exit events. |
| RTP recovery counters are mistaken for a direct jitter measurement | Audio artifacts could be attributed to a single cause without evidence | v0.1.39 reports sequence gaps, resend recovery, concealed frames, and queue snapshots but not packet arrival jitter; implement monotonic inter-arrival metrics before tuning the jitter buffer. |
| Sensitive data enters source, issues, or logs | Privacy exposure | Use the contributor redaction checklist; never publish identifiers, credentials, addresses, or raw logs without review |
| Third-party licensing is incomplete | Redistribution may violate license terms | Preserve notices and review dependencies before binary releases |
| RSX label or flip polling never completes | Foreground app can appear frozen and prevent controller exit; this resembles the reported black-screen/forced-shutdown failure | v0.1.4 still inherits unbounded waits from the screen implementation, and controller setup occurs after graphics setup. Bound those waits and preserve an early exit path before the next launch test. |
| Receiver loopback timing test is scheduling-sensitive | A host run can stall at its first timing-probe receive, reducing confidence in protocol changes | Latest direct guest run timed out; trace-assisted run passed. Diagnose the discrepancy and require repeated uninstrumented passes before marking green |
| App fails before fresh diagnostics appear | XMB launch error prevents display, controller, and AirPlay testing; a stale log can hide whether `main()` or the logger opened | AIRPLAY02 v0.1.2/v0.1.3 and AIRTEST01 v0.1.0 each returned `80028F21`; receiver log remained unchanged. The reviewed PS3 developer error table has no entry for this exact code. v0.1.4 is only locally built. Compare against the earlier tone-producing AIRPLAY02 build and do not infer a root cause from the code alone. |
| Network startup depends on immediate NetCtl state | The receiver could exit before mDNS/listener startup if NetCtl has not reported an IP yet, even when routing is available | v0.1.4 restores the earlier UDP-route address lookup first and adds a five-second bounded NetCtl wait/fallback. Cross-build passes; hardware startup behavior remains unverified. |
| Native launch control diverts effort from working receiver | The silent test app does not preserve diagnostic tone or AirPlay behavior and cannot establish receiver playback | AIRTEST01 v0.1.0 is a separate installed app, not an AIRPLAY02 upgrade. It returned `80028F21` and is parked; do not use it as the receiver test path. |
| Receiver version string drifts from package metadata | Hardware logs could be misattributed to a different package | v0.1.4 makes `APP_VERSION` a build input for both startup log and SFO packaging; the extracted SFO reports 0.1.4. Verify exact installed package before interpreting future logs. |
| Legacy linker output is structurally unlike the known v01.01 ELF | XMB/RPCS3 may reject an otherwise valid NPDRM package before application startup | New validator compares the CellOS/PPC64 header and PS3-specific load flags against the preserved v01.01 ELF. v0.1.6 fails this layout check; the precise reason RPCS3 rejects it is still unproven. Use the modern PS3DK/Lima pipeline and require RPCS3 launch evidence. |
| RPCS3 multicast discovery cannot bind host UDP 5353 | The emulator cannot run its mDNS listener while the host owns the standard mDNS port | v0.1.12 logged stage 4 (bind), error 112 (`EADDRINUSE`) both with Lima running and stopped. With Lima stopped, the system mDNS responder still owned wildcard UDP 5353 and a wildcard bind probe failed. Do not stop the system service. Test native PS3 discovery separately because its port ownership differs. |
| RPCS3 exits before listener startup when it has no usable local route | Emulator evidence cannot validate mDNS, RTSP, or RTP; repeated UI launch attempts only reproduce an early timeout | v0.1.14 logged `NET_IP_TIMEOUT` then `APP_STOP result=1` before crypto/listeners. Inspect the emulator network configuration and distinguish emulated route failure from receiver defects; do not change physical PS3 network settings. |
| RPCS3 guest networking is disabled | App startup stops during address acquisition, so package and protocol behavior after NetCtl cannot be exercised | v0.1.15 logs `NET_IP_TIMEOUT`; RPCS3 reports `EADDRNOTAVAIL`, and its saved Internet setting is `Disconnected`. A one-run change to `Connected` followed by restoration is the next controlled emulator check. |
| FLUSH drains only a bounded number of pending datagrams | If the socket contains more than 256 queued datagrams, old packets may remain after the single bounded drain | The receiver logs `RTP_FLUSH_DRAIN_LIMIT` when more data remains. Host tests verify normal queue/socket clearing; add a cap-exhaustion test and confirm sender behavior before claiming complete flush under overload. |
| AirPlay volume transfer is not yet hardware-verified | A correct SET_PARAMETER response may still differ from the PS3 audio port's effective output behavior | v0.1.16 source clamps dB attenuation to -30..0 dB, maps -144 dB to mute, and applies a block-sized gain ramp. Host parser/PCM tests and PPU objects pass; verify level changes and unmute with an iPhone on physical hardware before claiming volume support. |
| Malformed ALAC frame fields could trigger out-of-bounds writes or undefined arithmetic | A crafted or incompatible packet could corrupt memory or produce unstable decode behavior | Current source bounds frame counts, predictor modes, quantization widths, interlace shifts, and Rice k-modifier; 4,096 seeded valid-frame mutations plus 256 random inputs pass ASan/UBSan halt-on-error. v0.1.20 packages these changes; broader fuzzing and real sender/hardware compatibility remain open. |

## Current RTP recovery candidate (2026-11-07)

- **Resend timing:** an 80 ms hold is a bounded starting point, not tuned to an iPhone/Spotify resend distribution. Too short can still conceal recoverable frames; too long increases jitter/latency and can drain PCM. Measure hardware before tuning.
- **Silence concealment:** substituting exact ALAC frame-duration silence prevents decoder failures on missing encrypted frames, but sounds like a short dropout; it cannot reconstruct lost music. Gaps over 128 frames skip directly to the next packet to prevent multi-second or minute-scale latency after a sender restart/corrupt sequence jump.
- **Sequence window:** signed 16-bit ordering is unambiguous only within half the sequence-number space. Existing queue capacity and new 128-frame cap keep normal playout inside that window; test wraparound and restart paths.
- **Emulator boundary:** RPCS3's 128-packet synthetic fixture passed without loss, so it validates encrypted negotiation/decode/PCM and clean app exit but not the new recovery path under loss. No human-audible emulator output or PS3 acoustic result was established.
- **Reference tuning:** other receivers retain 512–1024 packets and use sender clock mapping; copying those values wholesale could raise PS3 latency or memory use. Keep current queue/prebuffer values until measured data justifies a change.
- **v0.1.39 runtime gap:** production and emulator-only packages validate from the pushed source revision, but v0.1.39 has not yet been installed or replayed in RPCS3. The checked-in runtime evidence is still v0.1.38 and has no packet loss. Keep the recovery implementation a candidate until the new target binary emits expected gap counters.

## Latest emulator findings (2026-11-07)

- RPCS3's disconnected virtual network causes the bounded NetCtl address wait to expire. With its network set to Connected, a route is available, but host TCP 5000 is occupied. The v0.1.25 emulator build on port 5001 opens its RTSP and RTP sockets and answers local OPTIONS; keep the production port at 5000.
- RPCS3 rejects the app's multicast-interface selection (mDNS stage 6, error 125), so emulator discovery remains untested. Do not stop or reconfigure host multicast services to work around this.
- The emulator confirms neither HDMI display output nor audible sound. Physical PS3 validation remains separate and requires an exact package test, a working recovery path, and safe launch/exit evidence.
- v0.1.27 accepted a TCP client and answered OPTIONS. Synthetic ANNOUNCE parsing identified encryption type 1 plus RSA key and IV fields, then the log stopped before validation completion. RSA and Content-Type events in v0.1.28 exceed the logger’s 24-character event-name limit and were silently rejected. Their absence cannot identify the stalled operation. v0.1.29 raises the limit and tests persistence of a 33-character event.

## Required gates before hardware testing

1. Identify the exact target software/hardware compatibility requirements.
2. Establish and document a tested recovery route.
3. Begin with a reversible standalone test.
4. Keep boot-time plugin settings unchanged unless a separately reviewed recovery plan explicitly requires otherwise.
5. Test resident lifecycle without audio before evaluating game coexistence.

## v0.30 compatibility candidate

Release label 0.30 maps to PS3 SFO 00.30. Packaging uses the preserved v01.01 type-1 debug container. Older three-component SFO values are now rejected. Production RTSP remains 5000; hardware launch/display/exit and actual AirPlay playback require fresh evidence. See the current decision log and memory for validation boundaries.

## Hardware evidence after SFO correction

The historical 0.30 build starts and plays its tone on the PS3; START exits while idle (maintainer report). A real iPhone attempt ends in an unanswered OPTIONS request and no completed RAOP session. No streamed music. START and PS input become unresponsive during that attempt. Corrected release label: 0.1.30; SFO remains 00.30. Resolve the request-handling stall before extended playback tests.

## v0.1.32 checkpoint

Big-endian PCM bytes and paced target packet/decode/PCM path now validate in RPCS3. Candidate is installed. Regular scanout and input-first loop target the reported PS/START issue; hardware overlay/exit confirmation remains required. Graphics waits are bounded and idle logging no longer destroys connection history as quickly. No extended soak until physical controls and clean exit pass.


## Current observed playback risks (v0.1.33)

- Audible music is confirmed, but crackling and low fidelity remain. The wrapped log shows 106 sequence gaps in 1,792 visible received packets; these are an observed risk signal, not a proven one-to-one cause of audible artifacts. v0.1.34 now queues throttled gaps and keeps 16 outstanding response ranges for up to one second. Validate loss/recovery on hardware before concluding the fix worked.
- XMB resume produces inconsistent image presentation/flicker. The retained log shows menu callbacks and no screen-present error but cannot verify displayed pixels or prove the cause. Keep video work separate from RTP recovery.
- The current 16 KiB log wrapped during the test and discarded startup/negotiation context. v0.1.34 reduces verbose packet/decode checkpoints to every 512 packets and adds five-second loss/recovery totals; retain the cap and confirm a fresh log still contains sufficient diagnostics.

## Live audio follow-up (v0.1.33, 2026-11-07)

- The installed binary was verified as v0.1.33; v0.1.34 is staged, and v0.1.35 is the next candidate. A live v0.1.33 snapshot reports 88 sequence gaps per 1,664 received packets, zero ALAC failures, and five resend-recovery events. Its 51,200-frame `PCM_UNDERFLOW` delta includes intentional silence during the initial prebuffer and cannot be used as proof of a playback dropout.
- The user reports crackle worsens while moving through XMB. This implicates realtime resource contention or network scheduling as a plausible contributor; do not conclude Spotify lossless input is the root cause. Compare with v0.1.35 first, then run a controlled source-quality A/B if loss/underflow persists.
- Do not install AIRPLAY02 over a still-running title. The live snapshot ended on a heartbeat/display event rather than APP_STOP. Require a clean app exit before staging installation.
- The supplied MacBook microphone sample is quiet and acoustic; gross-level analysis did not detect hard silence, but it cannot rule out short/subtle dropouts. Do not use it to override packet/underflow telemetry or to claim clean audio.
- v0.1.35 gates PCM underflow accounting until decoder/prebuffer readiness. Hardware logs must confirm the corrected counters correlate only with active stream output; packet loss can still cause audible gaps even if the PCM ring doesn't empty.
- v0.1.35 build is validated and staged but not installed. The running v0.1.33 stream must exit before replacing AIRPLAY02; no playback, display, or XMB-resume claim is made for v0.1.35.

## v0.2.7 measured hardware boundary

- The 30m03s dev run reports 26,814 PCM underflow output frames and a maximum audio-service gap of 35.842ms. These are plausible dropout/pacing risks; neither counter establishes an audible artifact or identifies its cause. A controlled same-source listening comparison is still needed.
- The RTP queue averaged about 136 of 256 slots across the full v0.2.7 application session and peaked at 177. No pending queue drops were reported. Monitor latency/backlog before changing the bound.
- The capture contains no XMB menu/resume cycle. The 350ms host/mock guard is not physical display acceptance.
