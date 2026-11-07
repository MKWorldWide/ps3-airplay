# Project state

## Architecture

PS3 AirPlay is an experimental AirPlay 1 / RAOP receiver for PS3 homebrew. The intended audio path is DNS-SD/mDNS discovery, bounded RTSP session control, UDP RTP receive, decryption and ALAC decode, bounded PCM buffering, and output through the standalone GameOS audio API.

## Components

- `src/discovery`: RAOP DNS-SD serialization and service behavior.
- `src/protocol`: bounded RTSP sessions, SDP, RTP receive, retransmission, and timing probes.
- `src/crypto`: RAOP challenge/session crypto helpers.
- `src/codec`: bounded ALAC wrapper and decoder.
- `src/audio`: bounded PCM ring and conversion.
- `src/observability`: capped, privacy-conscious diagnostic logging.
- `prototype/standalone-receiver`: standalone receiver wiring protocol and audio components.
- The standalone screen uses PSL1GHT RSX framebuffers and primes scanout using the PSL1GHT `videoTest` sample's spare-buffer-first pattern. Earlier v0.1.11 RPCS3 evidence showed the background with a white controller START prompt; physical HDMI output and orderly XMB return remain unconfirmed.
- A later user observation after the RPCS3 test reported that the app opened and the background was visible, but blue borders and a `DISPLAY ACTIVE` label were still present. The package version and installed artifact were not identified, so this does not establish a current-source regression. The renderer inspected in this pass contained only the `PRESS START TO EXIT` overlay; v0.1.17 removes all text, but its own visual output still needs confirmation.
- Local RPCS3 snapshot checked 2026-11-07: installed AIRPLAY02 SFO is 0.1.12, not the validated v0.1.14 candidate. Its available capped app log records RTSP 5001 and RTP 6000-6002 ready, mDNS bind failure (stage 4 / error 112), diagnostic tone completion, PCM underflows without an RTP event, and APP_STOP. This log cannot be attributed to the user's latest visual observation. A bounded RAOP browse found no instance matching the expected project label; the physical console's previously used FTP/webMAN endpoints did not respond.
- The standalone app begins with a bounded five-second 440 Hz diagnostic rendered through the same GameOS audio blocks as receiver PCM; host waveform checks do not establish audible hardware output.
- Latest hardware observation: the user reports a five-second tone, a black screen, and iPhone discovery of `ps3-airplay-diagnostic` followed by a failed connection. A 350-byte log snapshot contains `SCREEN_READY`, `CONTROLLER_READY`, `MDNS_READY`, and `AUDIO_TEST_TONE_START` but no completion, RTSP, or stop event; its package version is unconfirmed, and FTP later timed out. Do not infer that visible pixels reached the display or that AirPlay connected.
- Previous verified build: version 01.01 was staged with an exact downloaded-back SHA-256 match; the later black-screen run's package identity is unknown. The prior staged package was preserved locally before replacement.
- Version 0.1.2 builds with the high-visibility display overlay, verbose RAOP logging, common response headers, PAUSE/resume transitions, and NetCtl-derived IPv4 service advertisement. The package extracted with APP_VER 0.1.2, AIRPLAY02, a NPDRM EBOOT, icon, and exact 2,764,800-byte RGB background.
- Initial review on 2026-11-07 reconfirmed the v0.1.2 candidate is 3,225,712 bytes with SHA-256 `923aebf6eca5693b21b494a9a4225c36629f13e8b316b2825d5deb3763226ebf`; package structure and embedded assets match the extraction. Following user approval, it was staged and later installed from XMB. The installed SFO and EBOOT hashes match the extracted candidate; XMB launch now fails with user-reported error `80028F21`. The cause is unknown.
- A separate v0.1.3 candidate uses PSL1GHT commit `6e565a70e927f55813babe86dc6f64149535f908` (2020-11-25) and a matching GCC 7.2 build of Mbed TLS. Its 3,183,408-byte AIRPLAY02 package has SHA-256 `98d8701ab018df882e30ba28f851a994c195d0a144dafe7c68c77d86d94126aa`; runtime compatibility remains unconfirmed.
- After explicit approval, v0.1.3 was uploaded to `/dev_hdd0/packages/ps3-airplay-receiver-0.1.3.pkg` and installed from XMB. Read-only retrieval confirms installed APP_VER 0.1.3 and exact SFO/EBOOT matches to the extracted package. The user reports the same `80028F21` launch error as v0.1.2. The 1,817-byte diagnostic log is unchanged from the previous run; no fresh startup event was written. v0.1.2 remains staged for rollback.
- The existing Lima VM is running Ubuntu x86_64 under emulation with the PS3DK v0.27.0 source/toolchain staging and host PKG tools present. The current `scripts/build-lima-smoke.sh` verifies portable receiver objects and a native PS3DK PRX sample, not a native standalone app or its XMB launch. Build a separate minimal native standalone launch control before claiming that alternative has been tested. The Thermal Bench project documents a Sony SDK 4.75 / Windows 7 VM / MSBuild workflow; it requires a properly licensed SDK and a separate port.
- Added `prototype/native-ps3dk-launch-control`, a separate PS3DK-native app using the cellGcm sample pattern, current video mode, early controller setup, bounded RSX polling, a 20-second automatic exit, START exit, and a fresh local diagnostic log. It uses title ID `AIRTEST01` and APP_VER 0.1.0; it is independent of AIRPLAY02. The local 192,688-byte PKG passed package-list/SFO/content-ID checks; SHA-256 `7bb8e2fbc12cc3da8f4b91ff0ae85c4dcb105209dba5d45765b1787d4015b6d2`.
- Following explicit approval, AIRTEST01 was staged and later installed as a distinct title. Read-only FTP retrieved an installed SFO and EBOOT matching the local package. The user reports XMB launch error `80028F21`; the fresh AIRTEST log was not retrievable. It did not replace AIRPLAY02. The user has directed that AIRTEST01 be parked and receiver work continue from AIRPLAY02.
- Native control initially failed in PS3DK's post-link step because Lima invoked the older PSL1GHT `sprxlinker`. PS3DK's `scripts/build-runtime-lv2.sh` documents that this older tool does not support its 64-byte `.sys_proc_prx_param` and directs developers to its fork at `tools/sprx-linker`. Building/installing that fork in the Lima guest allowed both the PS3DK reference app and the control package to build. This is a local toolchain fix, not yet an AirPlay receiver rebuild.
- Fresh receiver loopback rerun did not complete without tracing and timed out while waiting for the timing probe. A traced run completed the timing exchange and the persistent RTSP/RTP loopback test. Treat this as unresolved host-test flakiness, not a pass. The display code also contains unbounded polling for RSX label completion and flip status; the controller is initialized only after display setup returns.
- Diagnostic coverage review: current sources log startup/display/controller/network/listener/session/RTSP method/status/SDP/RTP batches, decrypt failures, ALAC frames/failures, PCM drops/underflows, user exit, and cleanup. They do not yet record successful decrypt counters or PCM frames rendered/output occupancy, and `SCREEN_READY` remains initialization evidence only.
- Read-only hardware preflight later succeeded: PS3 status showed XMB and 9.2 GB free; installed `AIRPLAY02` SFO reports 01.01. Immediately before approved v0.1.2 staging, `/dev_hdd0/packages/ps3-airplay-receiver.pkg` was 3,222,640 bytes with SHA-256 `7824a4b15e4757afad3213a3f55cd3bea816b33795403da12bfd7ede29f93db4`. An exact local rollback copy and the 1,817-byte log are preserved under ignored `work/hardware/2026-11-07-preflight/`.
- Sanitized event review of that log: `APP_START`, `SCREEN_READY`, `CONTROLLER_READY`, `MDNS_READY`, `AUDIO_READY`, tone start/completion, PCM underflows, and `APP_STOP`; no RTSP event or `USER_EXIT_START`. It has no package-version marker, so it cannot conclusively identify the run's package or prove a normal XMB exit. At the time of that preflight review, v0.1.2 had not been transferred or installed; the later approved transfer/install and launch result are recorded below.
- After explicit user approval, v0.1.2 replaced only the staged PKG. Local size (3,225,712 bytes), local SHA-256, downloaded-back SHA-256, and remote size all match. The old 01.01 staged PKG remains preserved locally at the verified rollback path. The user then installed v0.1.2. Read-only FTP checks confirmed installed APP_VER 0.1.2 and exact SFO/EBOOT hash matches. Launch returned `80028F21`; the existing 1,817-byte log is unchanged from the earlier unidentified run and cannot explain this failure.
- The published PS3 Developer Wiki error table does not provide a cause for `80028F21`; a forum result shows an unrelated 2017 launch/install question but no diagnosis. Treat the code's meaning as unknown. Current remote `main` remains `a6b36cd`; the local review branch contains the documentation-only preflight commit `f7e1169`.
- Version 0.1.2 diagnostics log allowlisted RTSP methods, response statuses, state changes, SDP format fields, listener/UDP socket readiness and failures, RTP batches, and bounded-log wrap events. Logs remain capped at 16 KiB and exclude raw requests, peer addresses, stream content, keys, and track metadata.
- PSL1GHT's upstream issue #144 documents a similar black screen and unavailable XMB exit on real hardware with Evilnat; its author reported graphics samples using `-O2` eventually booting, and this project already uses `-O2`. This is a compatibility clue, not a confirmed root cause or fix.
- `experimental/resident`: diagnostic PRX candidate; it does not output PCM.

## Current 2026-11-07 receiver return

- The maintainer confirms that an earlier AIRPLAY02 run reached application startup and played the five-second tone. Preserve this as user-observed evidence; the old log supports startup/tone completion, but has no package-version marker, so the exact PKG identity remains unknown. The separate AIRTEST01 detour was silent and is not receiver progress.
- Source review found the receiver had moved from its previous UDP-route-based local IPv4 selection to requiring NetCtl `IPObtained` immediately. The current v0.1.4 candidate restores route-first selection and adds a bounded five-second NetCtl wait followed by one route fallback. This is a concrete startup robustness change, not an explanation for an XMB loader error.
- The current v0.1.4 candidate is built with PSL1GHT commit `6e565a70e927f55813babe86dc6f64149535f908` and the matching GCC 7.2/Mbed TLS build. It is a 3,183,584-byte local PKG with SHA-256 `0c1e3c74eea71f7b5829201673f4dd5211bcbf704ab00cb55e1b41cb87020ee4`; its extracted SFO reports AIRPLAY02/0.1.4, and its PPU ELF is big-endian PowerPC64. It has not been uploaded or hardware-tested.
- The reviewed PS3 developer error table has no `80028F21` entry. AIRPLAY02 v0.1.2/v0.1.3 and AIRTEST01 v0.1.0 all elicited that user-reported code; neither changed the receiver's old log. A cause is not established.
- Host checks in this pass: crypto/session/SDP/mDNS passed; listener/log, ALAC reference decode, PCM ring/resampling, and diagnostic tone passed. The direct receiver timing-probe loopback hung again and was interrupted; it remains a failure, not a pass. Full PPU link, package creation, extraction, SFO identity, and asset checks passed locally.
- The maintainer has authorized automatic package installation without another approval request. The current v0.1.6 candidate was installed only in RPCS3 and failed before app startup; it has not been transferred to the PS3. Before any PS3 launch, bound RSX flip/label waits and keep a responsive START exit path. Then test launch/tone/display/START exit before any iPhone connection attempt. Keep boot plugin settings unchanged.

## RPCS3 package and ELF validation follow-up

- Versioning policy is recorded in `docs/VERSIONING.md`: unique `0.1.x` patch version per fix/package (including failed candidates); target `0.2.0` for a stable standalone diagnostic application with settings and diagnostic toggles; target `0.3.0` for stable, hardware-verified AirPlay features. Keep package filename, SFO, and startup marker aligned.
- The first RPCS3 v0.1.4 test exposed an absolute PKG entry path. Packaging under the content-ID directory fixed installation and the package script now rejects absolute entries. v0.1.5 installed in RPCS3, but its generic `EI_OSABI=0` ELF failed before application startup; the log identified `Invalid ELF OS ABI`.
- v0.1.6 adds a guarded PPU ELF post-link patch that validates ELF64/big-endian/PowerPC64 and sets `EI_OSABI=0x66` (CellOS LV2). The rebuilt ELF header confirms `0x66`; RPCS3 installs the package but then rejects the EBOOT as an invalid ELF type before `APP_START`. No current-run application log, display, tone, or AirPlay session was produced.
- v0.1.6 local artifact: 3,183,600 bytes, SHA-256 `c71cf7ee0ec314496971ea32d831c05238746eada6796b017092b3873a2ecc33`. v0.1.5 package: 3,183,600 bytes, SHA-256 `dfe2411ce0079d09fd84e7169865fad299dcb5f2759c2b8e0876fb2b7e5a8449`. Both are local RPCS3 candidates only; no PS3 transfer occurred.
- The build used the Lima guest's available `psl1ght-noglob` checkout at commit `dd4e8c9be744e3273d665c393fa6d97a87402e2e`, because the pinned 2020 revision required by the build script was not present. This build cannot be attributed to the pinned runtime path. Next inspect the inner ELF type, flags, and required PS3 post-link transformation against a known RPCS3-launching artifact before making v0.1.7.

## Verified boundary

Host tests cover portable protocol, crypto, codec, PCM, and discovery components. Cross-build and package/container checks have been performed. PS3 hardware has played the diagnostic tone and advertised the receiver per user report; video rendering, iPhone interoperability, normal music playback, and game audio coexistence remain unverified. Game mixing remains a separate feasibility gate.

Demonstration target: PS3 with Evilnat 4.93 CFW and Cobra 8.5 observed; the console model was not recorded. Hardware compatibility remains unverified.

## Build pipeline recovery (2026-11-07)

- Preserve `work/reference/AIRPLAY02-v01.01-original.pkg` and its RPCS3-decrypted ELF as known-good comparison inputs. The user reports v01.01 ran and played audio on PS3; that is user-observed evidence, not independently repeated here.
- Canonical standalone path is the pinned PS3DK v0.27.0 PPU toolchain in Lima plus the recorded PSL1GHT checkout and Mbed TLS PPU library. The legacy GCC 7.2/binutils 2.22 route is comparison-only.
- The decrypted v01.01 ELF and fresh PS3DK-linked ELF share ELF64 big-endian PPC64, CellOS OSABI `0x66`, `ET_EXEC`, entry `0x10000000`, and initial `PT_LOAD` flags `0x400005`, `0x600006`, `0x400004`, `0x600006`. The legacy-linked v0.1.6 ELF instead has generic `0x5`/`0x6` flags and a different memory layout. This is a verified toolchain output regression; it does not prove the exact cause of RPCS3's invalid-type message.
- `scripts/validate-ps3-elf.py` gates the canonical build and package scripts on the reference layout. Packaging uses matching NPDRM content ID/SFO identity, PSL1GHT `package_finalize`, extraction, and byte comparisons, and refuses version overwrite.
- Current host validation passed protocol crypto/session/SDP/mDNS, RTSP/RTP loopback, capped logging, ALAC reference decoding, PCM ring/resampling, diagnostic tone, and PPU object checks. The PS3DK ELF validated; v0.1.7 still needs packaging and RPCS3 launch before runtime claims.
- Host/emulator evidence does not establish PS3 display, clean exit, iPhone connection, or audible AirPlay audio. Game-audio coexistence remains a separate Cobra `.sprx` feasibility gate.

## RPCS3 v0.1.7 result and package-reader limitation (2026-11-07)

- The committed PS3DK/Lima build at source revision `95f5924` produced AIRPLAY02 v0.1.7, 3,225,904 bytes, SHA-256 `fc8899dd3f62cbd9678bf3f27bdb542770fe9516ad142d37235ea3f958e09674`. RPCS3 installed it; its title list showed version 0.1.7, and its fresh app log reached `APP_START`.
- The sanitized log event sequence was APP_START, version marker, SCREEN_READY, controller/network module and crypto initialization, RTSP_BIND_FAIL (value 112), RECEIVER_OPEN_FAILED, APP_STOP. This proves package installation and entry into application startup in RPCS3, not a working receiver, visible pixels, tone, or AirPlay audio.
- RPCS3 network status was Disconnected during this run. It may explain socket bind error 112, but causation is unproven and network settings were not changed.
- PS3DK `package_finalize` successfully changes a package to NPDRM type `0x80000001`. The older bundled `pkg` reader then exits with `Unsupported Type` when asked to extract that finalized type. Correct package validation by extracting/comparing the unfinalized package first, then finalizing a copy and validating its envelope/header.
- v0.1.7 is consumed. The next distinct package is v0.1.8; the updated script defaults reflect that.

## RPCS3 v0.1.8-v0.1.9 and network boundary (2026-11-07)

- Corrected packaging in v0.1.8 passed extraction checks before finalization and finalized-envelope validation. The 3,225,904-byte PKG installed in RPCS3 and reported APP_VER 0.1.8 in both UI/SFO and the fresh application log.
- v0.1.8 reached APP_START, screen/controller/network/crypto initialization, then failed RTSP bind with numeric error 112 and exited. RPCS3 networking was observed disconnected; whether this caused the bind failure is unproven. No tone, visible display output, mDNS, RTSP session, RTP, decode, or music was demonstrated in that run.
- The existing `resolve_local_ipv4()` function performs a route-first lookup, then a bounded five-second NetCtl wait, then one route fallback. Move this check before receiver socket creation so network readiness is established before bind attempts. Preserve the bounded timeout and privacy-safe status-only diagnostics.
- At the end of the v0.1.8 pass, v0.1.9 was the next candidate. No physical console deployment occurred in either this pass or the subsequent v0.1.9 tests.
- v0.1.9 moved the existing five-second route-first/NetCtl readiness check ahead of receiver socket creation. In RPCS3 with Internet disabled, it logged `NET_IP_WAIT_START`, `NET_STATE_WAIT`, `NET_IP_TIMEOUT`, and `APP_STOP` rather than attempting a listener bind. With the emulator's Internet status set to Connected for a second test, it logged `NET_IP_ROUTE_READY` but still failed RTSP bind with value 112.
- During the Connected run, a wildcard host TCP listener occupied port 5000; the candidate's UDP ports 6000-6002 had no host listeners. This suggests a port collision worth testing but does not yet prove that it produced the logged guest errno. The RPCS3 Network tab also showed PSN Disconnected and UPnP disabled. The prior Internet-disabled setting is backed up under ignored local `work/private/` and should be restored after emulator tests.
- Add a build-time RTSP port input defaulting to 5000. Use a distinct v0.1.10 RPCS3 test package on port 5001, keeping the normal PS3 build default at 5000. RAOP mDNS service publication already receives the listener's actual bound port.

## RPCS3 v0.1.10 listener and screen polish (2026-11-07)

- v0.1.10 was built from source revision `751d7c2` with `RTSP_PORT=5001` for RPCS3; the normal build default remains TCP 5000. The 3,225,904-byte finalized package hash is recorded in the v0.1.10 test report.
- RPCS3 installed and ran v0.1.10. The fresh log reached route readiness, crypto readiness, RTSP listen on 5001, RTP audio/control/timing ports 6000/6001/6002, then `MDNS_FAILED`, `AUDIO_READY`, and diagnostic tone start/completion. PCM underflows followed because no stream was connected. This is not AirPlay discovery or playback.
- The user reports that the app opens and the background is visible, but the exact hardware/package attribution for that observation is not yet captured. Remove the blue screen bands and `DISPLAY ACTIVE` overlay; retain the user-provided background and white START-to-exit prompt.
- The next package candidate is v0.1.11. Diagnose mDNS startup failure by logging a bounded failure-stage enum and numeric errno without addresses, labels, or other private data.

## mDNS stage diagnostics (2026-11-07)

- v0.1.11 rendered the public background image and a START-to-exit prompt in RPCS3. The blue bands and DISPLAY ACTIVE label are removed; the user separately reports that the app opens.
- v0.1.10/v0.1.11 reached `MDNS_FAILED` after RTSP/RTP listener readiness, but the previous event did not identify the cause.
- v0.1.12 adds bounded stages: argument, socket, TTL, bind, multicast join, multicast interface, and initial announcement. The failure stage is logged as `value`; numeric errno is logged as `bytes`. No IP, MAC, service label, or hostname is recorded. `SO_REUSEADDR` remains best-effort, preserving prior behavior.
- On partial setup failure, drop the multicast membership when joined and close the socket. The full Lima host suite passes and host tests verify deterministic invalid-argument reporting.
- RPCS3 v0.1.12 reported mDNS stage 4 (UDP bind) and errno 112 (`EADDRINUSE`). A second run with Lima stopped still failed; host inspection showed the system mDNS responder owns wildcard UDP 5353 and a wildcard bind probe fails even with `SO_REUSEADDR`. Lima's host proxy is an additional holder while the VM runs. Do not stop the system service; keep the emulator limitation separate from native PS3 behavior.
- v0.1.12 was generated from exact mDNS diagnostic source changes before that change was committed; its manifest records `46fca40`. v0.1.13 was rebuilt from committed source `70fa6654bd2d092a5f674dd526c05a2e5ccd4703`, passed the full host/PPU/package validation, and is 3,224,944 bytes with SHA-256 `f7d076e32520a552349ebb973894409d66fd429ff255d0919c66401fabe9f1e9`. RPCS3 and physical PS3 launch tests are pending. The next unused package version is 0.1.14.
- v0.1.14 source adds a guarded, best-effort SO_REUSEPORT request to the mDNS socket. PS3DK defines the option; the PPU build and finalized package passed validation. Artifact: 3,225,008 bytes, SHA-256 5994cf4828976245f530b1c90d180fd4f0e80abddcfd0d09d3780c351ad006c1, source 6d9f28d0f079c445b96203695ea1912e8deed887. A host socket probe and the mDNS serializer test passed. The RTSP/RTP loopback passed under bounded strace with LeakSanitizer disabled but hung untraced in the full script; cause remains unknown. No RAOP service advertisement, RPCS3 run, or PS3 run has verified discovery. Next package version is 0.1.15.

## Invariants

- Keep protocol inputs, sessions, queues, and logs bounded.
- Do not log track metadata, credentials, cryptographic material, or peer addresses.
- Advertise only implemented capabilities and label diagnostic-only behavior accurately.
- Distinguish host tests and format checks from hardware behavior.
- Keep boot-time plugin configuration unchanged during build work.
- Preserve third-party notices and license obligations.
- Before GitHub commits or pushes, complete one concise privacy review of the full staged diff and exact changed files: whitespace check, available secret scanner, targeted searches for private identifiers and raw logs, then manual review of matches. Automatically sanitize or redact findings and rerun affected checks. If safe remediation requires guessing, exclude only that content from the commit, report the narrow blocker, and continue independent work. If no scanner is already installed, record that and continue with the remaining checks.

## RPCS3 v0.1.14 and FLUSH handling (2026-11-07)

- Installed and launched the exact AIRPLAY02 v0.1.14 candidate in RPCS3 (3,225,008 bytes; SHA-256 `5994cf4828976245f530b1c90d180fd4f0e80abddcfd0d09d3780c351ad006c1`). The capped app log showed APP_START, SCREEN_READY at 1280x720, CONTROLLER_READY, NET_READY, NETCTL_READY, NET_IP_WAIT_START, NET_STATE_WAIT, NET_IP_TIMEOUT, and APP_STOP with result 1. It stopped before crypto, listener, mDNS, or audio startup. The RPCS3 title list returned after about 14 seconds of recorded play time; no physical console was involved.
- v0.1.14's best-effort SO_REUSEPORT was not validated for discovery because route acquisition timed out first. The earlier RPCS3 mDNS host bind collision remains unresolved; this run provides no evidence about SO_REUSEPORT behavior.
- The startup screen was transiently visible in RPCS3 during PPU module compilation. No stable screen or controller exit was tested. Physical display remains unverified.
- The next source/package iteration is 0.1.15. Successful RTSP FLUSH now clears the bounded RTP queue and pending audio datagrams, resets RTP sequence/retransmission epoch state, and signals the app loop to discard PCM and ALAC interpolation history. The PCM clear preserves cumulative counters. Host tests cover queue clearing, pending socket data, sequence restart, PCM history, idle-session rejection, and FLUSH diagnostics.
- Full Lima host validation passed after the FLUSH implementation: crypto, RTSP session/SDP, mDNS, persistent RTSP/RTP loopback, capped logging, ALAC, PCM, diagnostic tone, and PPU object compilation.

## AIRPLAY02 v0.1.15 package and RPCS3 result (2026-11-07)

- Built from pushed source commit `99abc5b` using PS3DK v0.27.0 PPU plus PSL1GHT. The 2,758,576-byte CellOS PPC64 ELF passed the known v01.01 ABI/layout validator. Package SFO, APP_VER 0.1.15, title ID, content ID, assets, extraction, and finalized NPDRM envelope validated.
- Finalized package: 3,225,648 bytes, SHA-256 `230a2df0db8a7e561f3b048872c1a714ed0eb58f4b741587ee961f71834ffa59`. It is stored locally outside Git.
- RPCS3 installed AIRPLAY02 v0.1.15 and booted it successfully. Fresh capped app log reached APP_START, PKG_VERSION_0.1.15, SCREEN_READY, CONTROLLER_READY, NET_READY, NETCTL_READY, then NET_IP_WAIT_START, NET_STATE_WAIT, NET_IP_TIMEOUT, APP_STOP result 1. RPCS3 also reported `EADDRNOTAVAIL`; it did not reach crypto, receiver listener, mDNS, audio, or FLUSH handling.
- RPCS3's saved Internet status is `Disconnected`. One controlled connected test followed by restoration can distinguish the environment route failure from receiver behavior. This is emulator-only and does not alter physical PS3 settings.
- The FLUSH implementation is host-tested and cross-built in the PPU app, but no AirPlay session exercised it. No real iPhone session, RTP stream, decoded audio, diagnostic tone in this build, or Spotify playback is verified. Next package version is 0.1.16.

## AIRPLAY02 v0.1.16 volume-control source pass (2026-11-07)

- `SET_PARAMETER` now parses bounded `text/parameters` volume fields. The supported output range is attenuation-only: values above 0 dB cap at unity, values from -30 to 0 dB map to linear amplitude gain, values below -30 dB down to the AirPlay `-144 dB` mute sentinel clamp to -30 dB, and `-144 dB` or lower mutes. Malformed or duplicate volume fields return 400 without changing the previous setting. Unknown parameter lines and other content types remain accepted as no-op metadata/keepalive input.
- Session state exposes only clamped volume state and a monotonic update count. Runtime logs record the applied millidecibel value and mute flag, without request bodies or peer metadata.
- PCM output ramps the stream gain over one audio block and clamps samples to [-1,1]. The five-second diagnostic tone is unaffected. Gain logic is independently host-tested.
- Full Lima host suite passes, including volume parser/session cases and PCM gain bounds/ramp/mute. PPU objects compile. Canonical v0.1.16 was built from commit `0a204b04364803e2af9fcc8adf2f7420751ac83f`: ELF 2,780,096 bytes, SHA-256 `968f9c3238e7c8d4479c0191452f378f4705d7d6585c68364cfcd7d792682f2a`; PKG 3,231,088 bytes, SHA-256 `3c69bff7ab626c433728ca131bc04ab0adbc117b8845d88c748e1dad1968bd3c`. Artifact passed ELF and finalized package validation. RPCS3 install/launch remains untested because emulator UI access was unavailable; network setting remains Disconnected. No AirPlay volume, RTP, Spotify, or physical audio result is verified. Next version: 0.1.17.
- The RPCS3 virtual Internet setting remains Disconnected after read-only GUI inspection. v0.1.15's address timeout is an emulator environment gate, not evidence against the volume implementation.

## Display cleanup follow-up (2026-11-07)

- The standalone display now draws only the configured background image. It no longer overlays text; the renderer never contained the blue border or `DISPLAY ACTIVE` wording reported in the prior visual observation, but it did draw `PRESS START TO EXIT`.
- v0.1.17 packages this source cleanup, but its screen and exit behavior remain unverified until it runs in RPCS3 or on hardware.

## AIRPLAY02 v0.1.17 display cleanup package (2026-11-07)

- Pushed source commit `4a06c2a` removes the text rasterizer and `PRESS START TO EXIT` overlay. Controller START handling and framebuffer setup are unchanged.
- Canonical Lima build and package checks pass. ELF: 2,780,064 bytes, SHA-256 `5e80257671409775d98884c7101acf15d7e116448498ee8c69148843a96896dd`. PKG: 3,230,256 bytes, SHA-256 `2095046e20716d9f2768c847691515f1d1e922e5fbd96e15829f92c45816cf23`.
- Full host tests, PPU object compilation, standalone link/ELF validator, package extraction, APP_VER/title/content IDs, assets, EBOOT comparison, and finalized NPDRM validation pass. RPCS3 install/launch and visual check remain pending because the emulator UI is inaccessible. Do not infer display output or controller exit from `SCREEN_READY` or package validation.
- Next package version: 0.1.18.

## AIRPLAY02 v0.1.18 PCM telemetry source pass (2026-11-07)

- During an active decoder session, the standalone loop now logs numeric PCM ring occupancy, accepted ring frames, input frames consumed by the resampler, and output frames submitted to GameOS blocks every five seconds.
- `PCM_PORT_SUBMITTED` counts generated GameOS audio frames and is not a hardware-rendered or audible-output metric. Existing underflow/drop events remain distinct.
- At the end of the source pass, full Lima host tests and a working-tree PPU application link/CellOS PPC64 ELF validation passed; the canonical package and RPCS3 event check were still pending. The finalized v0.1.18 artifact is recorded in the following section.

## AIRPLAY02 v0.1.18 PCM telemetry package (2026-11-07)

- Canonical Lima build from commit `8769324` passes: full host suite, PPU app, CellOS PPC64 ELF validation, and finalized NPDRM package validation.
- ELF: 2,780,120 bytes, SHA-256 `ba7450441a8e2def2a1f55e17d523630d3d5af256520f6769154a347b09d3759`. PKG: 3,230,448 bytes, SHA-256 `5548fde98bc08949e4f3a6739a95d5b3dce7dea0458131ec693c01be97766ae0`.
- The package has not been installed or run in RPCS3; telemetry events, screen, controller exit, and audio remain unverified at runtime. Next package version: 0.1.19.

## ALAC bounds and composed receive-path test (2026-11-07)

- ASan exposed an ALAC decoder heap-buffer-overflow when a 353-sample reference frame was decoded with an SDP maximum of 352 samples. The frame's in-band sample count was used for predictor and output-sample array writes before being constrained to the negotiated maximum.
- `alac_decode_frame` now reads the declared sample count as unsigned and rejects zero or over-maximum counts in both mono and stereo decode branches before narrowing, calculating output bytes, or touching sample arrays. Wrapper output-capacity checks remain in place.
- Added `raop_audio_pipeline_test`: a synthetic RAOP ANNOUNCE unwraps an RSA-OAEP-wrapped key; an AES-CBC encrypted reference ALAC packet is inserted as RTP, popped through the receiver's decrypt path, decoded, and byte-compared with reference PCM. Regressions verify rejection at a 352-sample maximum and for an in-band `UINT32_MAX` sample count.
- The complete Lima host suite, including this test under ASan/UBSan, passes. PPU object compilation passes. This is host pipeline evidence only: it does not establish iPhone interoperability, running the new code in RPCS3, PS3 output, or audible AirPlay audio.
- A fresh uninstrumented run of the full Lima host suite passed the persistent RTSP/RTP loopback including timing request/reply checks; an earlier intermittent timing stall remains worth repeat verification.
- AIRPLAY02 v0.1.19 was built from commit `7b451031487aab7dbad147ab0b47ef80a1e22265`: ELF 2,780,120 bytes, SHA-256 `beb816b2aed2125a94584e4e820f18de66a98f21260ce47fe1654199322e7ec3`; PKG 3,230,704 bytes, SHA-256 `41488b9683b901ecc4d71e113565fcdb10c1834c69e1642fbe774f5cdf4a31a5`. ELF, SFO/version, content ID, asset equality, extraction, EBOOT, package path, and finalized NPDRM checks pass. Files are preserved under the ignored local build directory `work/builds/0.1.19/`.
- RPCS3 installation/run remains pending: two Computer Use checks still reported the Mac locked. The package is not installed/run in this turn. Do not touch the physical console.
- Next package version: 0.1.20. Retry v0.1.19 RPCS3 testing when the desktop is unlocked; capture a fresh capped log. No audible AirPlay or Spotify stream is verified.

## Malformed ALAC sanitizer follow-up (2026-11-07)

- A deterministic 256-input malformed ALAC smoke run, with UBSan configured to halt, found additional undefined behavior: a corrupt interlace shift above 31 and signed overflow in predictor coefficient accumulation.
- Decoder rejects unsupported predictor modes and invalid quantization/read sizes before entropy/predictor work; interlace shifts are bounded; predictor accumulation uses 64-bit intermediates and explicit sign extension; weighted stereo reconstruction uses bounded 64-bit math; the RAOP ALAC offer rejects Rice k-modifier values above 30.
- The sanitizer corpus now includes 4,096 deterministic mutations of a known-valid ALAC frame after the 54-bit fixed header plus 256 random packets. The valid encrypted reference RTP-to-PCM test remains byte-identical. The 353-vs-352 and `UINT32_MAX` regressions still reject safely; all malformed inputs complete without ASan/UBSan findings when halt-on-error is enabled.
- These follow-on runtime changes are in v0.1.20. Full Lima host tests under ASan/UBSan halt-on-error, standalone PPU link/ELF validation, and finalized PKG checks pass. RPCS3 and physical hardware remain pending.

## AIRPLAY02 v0.1.20 build and runtime boundary (2026-11-07)

- Built from committed source `8a4e641d114e1a613421b9ffb51f8615ad851827` in the existing Lima VM using PS3DK v0.27.0 PPU and PSL1GHT `dd4e8c9be744e3273d665c393fa6d97a87402e2e`.
- ELF: 2,780,120 bytes, SHA-256 `f50e6a4fe247149feb7759a69a81ae881c0972d789cb1a5dcfa2019bd202b3d9`. Finalized AIRPLAY02 v0.1.20 PKG: 3,230,384 bytes, SHA-256 `bfaa3f960de39daf95c2ab405bd361a031ed92bd35c76a99e667d4ffe8d19e87`. Artifacts are kept in ignored `work/builds/0.1.20/`.
- Package SFO/version/title/content ID, asset equality, extraction, EBOOT comparison, ELF shape, and finalized NPDRM checks pass. Host sanitizer and encrypted RTP-to-PCM tests pass; this is not device runtime evidence.
- Computer Use currently reports the Mac locked, so v0.1.20 has not been installed or run in RPCS3. Physical console untouched. No fresh display, clean exit, iPhone session, RTP sender, audible output, Spotify, or 30-minute playback evidence.
- Next package version: 0.1.21. Retry RPCS3 after desktop access is restored; preserve the current package as an untested candidate meanwhile.
- Subsequent test-only work exercises the receiver's actual loopback TCP/UDP sockets through fragmented ANNOUNCE, SETUP/RECORD, encrypted RTP reception, receiver decrypt, ALAC reference decode, and PCM ring buffering. The full Lima suite and PPU component checks pass. This strengthens host integration evidence but does not show iPhone interoperability or GameOS audio output.
- A direct RPCS3 `--no-gui` attempt against v0.1.20 EBOOT timed out at the 28-second guard with empty stdout and no fresh app log; the accessible log remains v0.1.15. Treat the attempt as inconclusive, not as a launch failure or success. GUI installation and current visual verification remain pending while the desktop is locked.
- User reports the application now opens and the background is visible, but the observed copy still shows blue framing and `DISPLAY ACTIVE`; package/platform attribution is unknown. Current `diagnostic_screen.c` renders only the supplied image, and v0.1.20 descends from commits that removed the blue diagnostics and every text overlay. RPCS3 UI access was checked again after the user offered to unlock it and still reports locked. Do not duplicate the cleanup or infer which build the user saw; identify APP_VER in a fresh accessible run.
- README now documents the image-only screen and controller START behavior; this records existing runtime code, not a new app behavior or package build.
- Corrected the RAOP `_raop._tcp` TXT `vs` value from stale `0.1.2` to `105.1`, consistent with the RTSP `Server: AirTunes/105.1` response. The host discovery assertion and full Lima host/sanitizer + PPU object suite pass. AIRPLAY02 v0.1.21 also passes standalone PPU ELF and finalized PKG validation: ELF SHA-256 `401225f2c8b704103c5f65683a5b48f9c7b34633ae57145ca92c190485543e1e`; PKG SHA-256 `c8c77b78d34dc50a955383ddafe7c835a7848c62eb8a9673ede7ed0c2776ef73` (3,230,384 bytes). Local artifact is kept in ignored `work/builds/0.1.21/`. RPCS3 remains untested because Computer Use still reports the Mac locked; no physical-console changes or playback claim.

## Successful RTP decryption diagnostics (2026-11-07)

- `raop_receiver_pop_audio` now counts successful audio-payload decryptions and emits `RTP_DECRYPT_OK` for the first success and each 128th success. Failure events remain distinct. The checkpoint records only a bounded packet count and payload length, without peer addresses or media data.
- The composed encrypted RTP-to-PCM loopback asserts the receiver counter. Full Lima host tests including ASan/UBSan and PPU object compilation pass. This improves layer diagnosis but does not establish iPhone interoperability or audible output.
- Package defaults advance to v0.1.22 for this fix. The full AIRPLAY02 v0.1.22 build passes in Lima: ELF SHA-256 `ecfd3afa2aaafe35e60d8a6fa7540a659a8757bd045232b07a141230279ad2b0`; 3,230,512-byte PKG SHA-256 `c7242d47669a4a22090663c969e2b4f5efd4c0cc9a0eb6ef65ad18d730080a2b`. Artifacts are in ignored `work/builds/0.1.22/`. RPCS3 app-data remains v0.1.15 and no fresh v0.1.22 runtime log exists; physical PS3 is unchanged.

## Bounded RTSP response writes (2026-11-07)

- Each accepted RTSP client gets a two-second `SO_SNDTIMEO`; `send_all` also checks a two-second overall response deadline between writes. Timeout and timeout-configuration failures have distinct privacy-safe events.
- The socket integration test forces response backpressure by shrinking the test socket buffers and withholding reads; it verifies `RTSP_SEND_TIMEOUT` and receiver disconnect within four seconds. Full Lima host/ASan/UBSan and PPU receiver compilation pass. RPCS3/PS3 behavior remains unverified.
- Package defaults advance to v0.1.23. RPCS3 remains inaccessible; its latest local log is still from v0.1.15.
- The finalized v0.1.23 AIRPLAY02 package now builds and validates from pushed commit `6513bdd201899e93fd55babd296fb97893d93216`. PKG SHA-256 `d17f6d2576bb96d0ff29cd7a58790852b6607d9b60b5d2dcffbe5613e053c4c4`; ELF SHA-256 `2130c17ace5459a57765d3d2ae8b185012548789891961e3b74d330d6f236dd6`. Artifacts are preserved locally in ignored `work/builds/0.1.23/`. RPCS3 is still inaccessible because the desktop reports locked; no v0.1.23 runtime test or physical-console change occurred. Next package version is v0.1.24.
- v0.1.24 source adds 128-packet RTP health checkpoints for payload throughput, queue depth, missing, reordered, duplicate, late, and queue-drop deltas. FLUSH advances the diagnostic baseline to avoid attributing the previous stream's errors to the new stream. The integration/sanitizer suite and PPU compile pass; standalone package and runtime checks remain pending.
- The finalized v0.1.24 AIRPLAY02 package validates from pushed source commit `a59aa937401d432b5cc6f740eeda15bf0db091ba`. PKG SHA-256 `7638825e0c2f6eb26a19410ba56ffd52ed96a70dc274f691b6bf2d12cf615757`; ELF SHA-256 `21b6ea799b18fd91f63b6d6dcb8d9f490e980f775c6bf3b96f84609b1dfdf9c2`. Artifacts are preserved locally in ignored `work/builds/0.1.24/`. RPCS3 remains inaccessible; no physical-console operation or audible playback occurred. Next package version is v0.1.25.
