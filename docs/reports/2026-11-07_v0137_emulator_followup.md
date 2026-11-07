# v0.1.37 emulator and deployment follow-up

**Date:** 2026-11-07
**Source:** `43eb638a4e76060fbe346385f1d76e9275db4e23`
**Candidate:** AIRPLAY02 v0.1.37, SFO `00.37`
**Test identifier:** RPCS3-037-RAOP-01

## Artifact verification

The production PKG is 3,236,528 bytes, SHA-256 `9ee7f1a419fea99a1fcbf40306159db100a665ae7c482f12376b1f18bd9104a3`. Its SFO identifies AIRPLAY02 / `00.37`. The RPCS3-only package uses RTSP port 5001 and is the same size, SHA-256 `b6eb56a42dfaa65e6ccb273595c9e047e4c312a428f3a11c3b2bbcc3fa5c0524`. The emulator package installed and launched successfully in RPCS3; its window displayed the project background.

## RPCS3 evidence

The app log identifies v0.1.37, initializes the display, controller, network, crypto, RTSP listener on 5001, RTP ports, and audio. The five-second tone completed. mDNS reports an emulator-specific failure, so discovery was not established.

The existing paced sender completed a direct encrypted RAOP session: OPTIONS, ANNOUNCE, SETUP, and RECORD returned 200; the receiver unwrapped the key, negotiated encrypted stereo ALAC at 44.1 kHz, received/decrypted RTP, and decoded a 353-sample frame. It then completed TEARDOWN. The sender's bounded integration run reports 128 RTP fixture packets and the TCP/UDP, decrypt, ALAC, and PCM-ring loopback checks passing. No RTP loss or ALAC failure was logged in this short run.

The log contains repeated `SCREEN_FLIPS value=0` heartbeats after initial rendering, consistent with the static renderer no longer submitting continuous flips. A single RPCS3 screenshot showed the background. This does not verify PS3 HDMI stability or the screen after opening/resuming the PS menu.

The short emulator session does not establish sustained playback, audible output, or that the measured PS3 dropouts are fixed. It does establish that the v0.1.37 network listener and the encrypted receive/decode path can operate in RPCS3 with the existing test sender.

## Console preflight and boundary

Direct FTP and webMAN HTTP checks to the last confirmed console address both failed with no route/connection. The PS3's staged packages and installed title were not accessed or changed in this pass. No console upload, installation, launch, or log retrieval occurred.

The prior v0.1.35 capture remains the latest physical audio evidence: 195 RTP sequence gaps, zero recoveries in retained checkpoints, zero ALAC failures, and 86,106 active-stream PCM underflow frames. v0.1.37's bounded resend acceptance still needs a version-matched PS3 run to show whether it reduces those gaps and dropouts.

## Results and next action

| Test | Result |
|---|---|
| Lima host/sanitizer/PPU/package validation | PASS |
| RPCS3 package install and launch | PASS |
| Static background in RPCS3 | PASS for one observed frame |
| RTSP + encrypted RTP + ALAC decode | PASS for bounded synthetic session |
| Sustained PCM playback / audible output | NOT ESTABLISHED |
| Physical display and PS-menu resume | NOT TESTED |
| Physical START exit and clean XMB return | NOT TESTED |
| Physical audio recovery/dropout improvement | NOT TESTED |
| PS3 transfer readiness | BLOCKED: console endpoint unreachable |

When the PS3 endpoint is reachable and the app is safely idle/exited, upload the production package over direct FTP as a new versioned file, verify the remote bytes, then install through the established webMAN/XMB process. Capture a fresh version-matched log and compare RTP gaps, recovered packets, active PCM underflows, and user-heard dropouts. Keep the RPCS3 port-5001 build off the console.
