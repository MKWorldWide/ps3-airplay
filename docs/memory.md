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
- The standalone screen uses PSL1GHT RSX framebuffers and primes scanout using the PSL1GHT `videoTest` sample's spare-buffer-first pattern. v0.1.11 removes the blue bands and `DISPLAY ACTIVE` label; it shows the public background with a white controller START exit prompt. RPCS3 visibly renders it, while physical HDMI output and orderly XMB return remain unconfirmed.
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

## Invariants

- Keep protocol inputs, sessions, queues, and logs bounded.
- Do not log track metadata, credentials, cryptographic material, or peer addresses.
- Advertise only implemented capabilities and label diagnostic-only behavior accurately.
- Distinguish host tests and format checks from hardware behavior.
- Keep boot-time plugin configuration unchanged during build work.
- Preserve third-party notices and license obligations.
- Before GitHub commits or pushes, complete one concise privacy review of the full staged diff and exact changed files: whitespace check, available secret scanner, targeted searches for private identifiers and raw logs, then manual review of matches. Automatically sanitize or redact findings and rerun affected checks. If safe remediation requires guessing, exclude only that content from the commit, report the narrow blocker, and continue independent work. If no scanner is already installed, record that and continue with the remaining checks.
