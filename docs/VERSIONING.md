# Versioning policy

AIRPLAY02 follows conservative pre-1.0 versioning. Increment the patch number for each distinct fix or small improvement that produces a new package: `0.1.4`, `0.1.5`, `0.1.6`, and so on. Never reuse a published or tested version number for different package contents. Record the exact package hash beside every result.

**Latest hardware-tested package:** AIRPLAY02 `0.2.4`, SFO `00.44`, built from `f870d823f15398691a6a38e6393f8d1376161bee`. PKG size 3,231,984 bytes; SHA-256 `a27d5c7b5d2d84f12fbb82ee4e164b9b50ccc825b5104b9b749f053d72a42bf7`. Installed SFO and EBOOT match the validated package. The user confirms PS opens the menu, START exits to XMB, and streamed AirPlay music works; resuming leaves a dim screen/frozen loading icon and short audio stutters remain. The 16 KiB run cap was reached in about 37 seconds.

**Known-safe functional baseline:** AIRPLAY02 `0.2.4` / SFO `00.44`. User confirms PS opens the system menu, START exits cleanly to XMB, and AirPlay streams music. Resume returns with a dimmed screen and frozen loading indicator; brief audio stutters remain. The 16 KiB per-launch diagnostic cap was reached about 37 seconds into the observed active-stream session. This is a functional rollback baseline, not a quality/stability release.

The immutable `safe-v0.2.4` Git tag points to packaged source commit `f870d823f15398691a6a38e6393f8d1376161bee`; it preserves the exact code used for the known-safe package hash above.

## v0.2.0 candidate package record

- Source: `b8bab2015551a7044dba26717611e524ed43f4ec` on `main` (startup tone removed; PCM ring quality baseline added).
- Artifact: `ps3-airplay-receiver-0.2.0.pkg`, AIRPLAY02, app log marker `0.2.0`, SFO APP_VER `00.40`.
- Size / SHA-256: 3,231,472 bytes / `924a01ef0ef812e1badb8e94011930f5187c4dba91b253e578e16ab72de4f29b`.
- ELF: 2,754,472 bytes, CellOS PPC64 executable, five PT_LOAD segments; SHA-256 `bc2c1f951624d0124f7771a50c0c3febe3d48c85cfd341691bc34cf4c5136875`.
- Build: existing Lima VM, PS3DK v0.27.0 PPU compiler 16.1.0, GNU ld 2.42, PSL1GHT revision `dd4e8c9be744e3273d665c393fa6d97a87402e2e`.
- Checks: full Lima host/sanitizer and PPU component suite; standalone PPU ELF validation; AIRPLAY02/SFO identity, content ID, package extraction, relative paths, icon/background equality, EBOOT comparison, and finalized NPDRM envelope PASS.
- Runtime: installed and launched on the demo PS3. User streamed a song; the log reached RTSP RECORD, RTP decrypt, ALAC decode, and PCM submission. User reports START and PS controls failed. webMAN exit completed cleanup and XMB was confirmed. Screen visibility and audible audio quality were not confirmed. Stable 0.2.0 acceptance remains open.
- The finalized PKG is preserved locally under ignored `work/builds/0.2.0/`.

## v0.2.1 controller-diagnostics candidate

- Source change: add rate-limited controller connect/disconnect, button-mask transitions, input errors, and named PS-menu callbacks. Restrict presence-mask iteration to the seven PSL1GHT controller ports to avoid undefined wide shifts.
- Source: `27da6c5cef4b41b58b672dc7e275b16667790b6c` on `main`.
- Artifact: `ps3-airplay-receiver-0.2.1.pkg`, AIRPLAY02, SFO APP_VER `00.41`.
- Size / SHA-256: 3,231,920 bytes / `f45b2935213dc1cfa62cfc70f958fc7a240925979fbe7aa779822a1a4a8a2c50`.
- ELF: CellOS PPC64, five PT_LOAD segments; SHA-256 `e875966c332610bb61e56f297b36eaaa69f74c190147c920d8c52a45ebfdc6c3`.
- Build: Lima, PS3DK v0.27.0 PPU compiler 16.1.0, GNU ld 2.42, PSL1GHT `dd4e8c9be744e3273d665c393fa6d97a87402e2e`.
- Checks: full Lima host/sanitizer suite, PPU component compile, standalone ELF validation, finalized package/SFO/content ID/extraction/EBOOT/assets/NPDRM checks PASS.
- Hardware: installed through webMAN and launched. The latest 10,658-byte log (SHA-256 `8fd3a3e922808cca9b5182149f0a1aed6c236770381dea2c7a2a59a64602e8e7`) records a single start, pad port presence, initial button mask 0, and a PS menu-open callback at 00:58:05Z. No later `PAD_BUTTONS`, `USER_EXIT_START`, or menu-close event appears. The user reports START exited the first launch, then neither START nor PS worked after reopening. Treat PS callback as delivery, not proof the menu appeared.
- Artifact is preserved in ignored `work/builds/0.2.1/`. This is a diagnostic candidate, not a controller fix or a stable milestone.

## v0.2.2 controller-input fallback candidate

- Source change: preserve PSL1GHT bitfield decoding and OR in the documented raw digital-button masks; log raw masks when they change and count positive pad input reports per heartbeat.
- Requested app label: `0.2.2`; SFO APP_VER: `00.42`.
- Source: `6151aae43b7ed26dfa940f090afbdd5b13ba53ac` on `main`.
- Finalized PKG: `ps3-airplay-receiver-0.2.2.pkg`, 3,232,112 bytes; SHA-256 `44886cd6ce7bbeff8089bd2d5d752cce3b76ded01f36b214cfdaff057f6748cd`.
- ELF: 2,754,776 bytes; SHA-256 `554c2580944bb0cd6d214f96b410c5d2c2395da609beeca9230455d2a3aa1bf6`. Extracted SFO is AIRPLAY02 / `00.42`; extracted EBOOT SHA-256 `b250051d8784922f696540cab7a5b672d7e85d651e1e6aa2df6144fec7e88fbf`.
- Build: Lima, PS3DK v0.27.0 PPU compiler 16.1.0, GNU ld 2.42, PSL1GHT revision `dd4e8c9be744e3273d665c393fa6d97a87402e2e`. Full host/sanitizer/PPU suite and ELF/SFO/package/NPDRM checks passed.
- Hardware: uploaded to a new versioned package path, downloaded back byte-identically, installed via webMAN, and launched. Installed SFO matches exactly; installed EBOOT matches the package payload plus 32 zero bytes.
- Controller result: user reports START and PS still do nothing. Latest 2,340-byte log (SHA-256 `010188b420884a0233e7cc04abfc17d8f0dcbf2c88aeee78a2107f38c6229f8f`) records `PAD_REPORTS=0` in the first two heartbeat windows. The PS menu-open callback fires, followed by two pad reports but no raw digital-mask change, no START event, and no menu-close event. Manual reboot/direct launch comparison completed with the same reported failure; do not issue remote launch/exit commands during user-controlled testing.
- Artifact is preserved under ignored `work/builds/0.2.2/`. The raw log remains private under ignored `work/hardware/`.

## v0.2.3 START fallback candidate

- Source: `5a04d93459611df730cc0b0f2d6f2afe84903b86` on `main`.
- Change: START exit now tests the normalized mask that combines PSL1GHT's `BTN_START` and raw `PAD_CTRL_START` representations.
- Package: `ps3-airplay-receiver-0.2.3.pkg`, AIRPLAY02, app marker `0.2.3`, SFO `00.43`, type-1 debug package.
- Size / SHA-256: 3,232,112 bytes / `756ac28147e98a37222279c607208639f157ddd4395bd2f4dfad8816ff4581b4`.
- ELF: 2,754,776 bytes, CellOS PPC64; SHA-256 `59f42ec05dc233c7450abfd2303f4fb929caf835dbc0d582e8164448b53b20fd`.
- Validation: full Lima host/ASan/UBSan suite, PPU component checks, standalone PPU build, ELF/SFO/content ID/assets/extraction/package checks passed.
- Deployment: FTP staged under a new filename and downloaded back with identical hash. webMAN installed while preserving the staged PKG. Installed SFO validates as 00.43; installed EBOOT is byte-identical to the 451,632-byte package SELF. A subsequent webMAN launch request returned HTTP 200 but left the console at XMB; log contained no new APP_START. Controller test pending; no runtime fix is claimed.

## Milestones

- **0.1.x — focused fixes and validation.** Each package changes one or a few clearly described items and carries a matching SFO and startup-log version. Failed candidates remain documented; the next distinct package increments the final number.
- **0.2.0 — stable diagnostic application.** The standalone app launches and exits reliably, provides actionable capped logs, and exposes basic settings and diagnostic toggles. This milestone does not claim working AirPlay audio.
- **0.3.0 — stable AirPlay receiver.** Discovery, connection, streamed audio, pause/resume, volume, reconnect, and sustained playback are verified on real hardware. RPCS3 and host tests are useful gates but do not replace console and iPhone evidence.
- **Later milestones — separately demonstrated capabilities.** Game-audio coexistence, resident-module operation, and other major features require their own feasibility and hardware evidence.

## Release rules

1. Choose the next version before building a new PKG; a failed package still consumes its version.
2. Keep `APP_VERSION`, SFO `APP_VER`, artifact filename, and startup log marker aligned.
3. Record source revision, package size, SHA-256, toolchain, checks run, and the exact failure boundary.
4. Reserve a minor milestone for a coherent capability boundary; do not advance minor versions for routine fixes.
5. Do not call a milestone stable until its acceptance tests in the roadmap pass at the stated hardware boundary.

## v0.1.16 package record

- Source: `0a204b04364803e2af9fcc8adf2f7420751ac83f` on `main`.
- Artifact: `ps3-airplay-receiver-0.1.16.pkg`, AIRPLAY02, APP_VER `0.1.16`.
- Size / SHA-256: 3,231,088 bytes / `3c69bff7ab626c433728ca131bc04ab0adbc117b8845d88c748e1dad1968bd3c`.
- ELF: 2,780,096 bytes, CellOS PPC64, five PT_LOAD segments; SHA-256 `968f9c3238e7c8d4479c0191452f378f4705d7d6585c68364cfcd7d792682f2a`.
- Build: Existing Lima VM; PS3DK v0.27.0 PPU compiler 16.1.0, GNU ld 2.42, PSL1GHT revision `dd4e8c9be744e3273d665c393fa6d97a87402e2e`.
- Package checks: SFO/version/title ID/content ID, icon/background, extraction, EBOOT comparison, and finalized NPDRM envelope PASS.
- RPCS3 installation/launch and physical hardware tests: pending. Do not infer volume behavior from host tests or ELF/package validation.
- At the time of this v0.1.16 record, the next package version was 0.1.17; that version is recorded below.

## v0.1.17 package record

- Source: `4a06c2a` on `main` (display cleanup: render the background without text overlays).
- Artifact: `ps3-airplay-receiver-0.1.17.pkg`, AIRPLAY02, APP_VER `0.1.17`.
- Size / SHA-256: 3,230,256 bytes / `2095046e20716d9f2768c847691515f1d1e922e5fbd96e15829f92c45816cf23`.
- ELF: 2,780,064 bytes, CellOS PPC64, five PT_LOAD segments; SHA-256 `5e80257671409775d98884c7101acf15d7e116448498ee8c69148843a96896dd`.
- Build: Existing Lima VM; PS3DK v0.27.0 PPU compiler 16.1.0, GNU ld 2.42, PSL1GHT revision `dd4e8c9be744e3273d665c393fa6d97a87402e2e`.
- Checks: full Lima host suite, PPU object compilation, standalone link/ELF validation, extracted SFO/title/content ID/assets, EBOOT equality, and finalized package envelope PASS.
- RPCS3 install/launch/display and physical PS3 tests: pending. Do not infer screen pixels, clean exit, discovery, or playback from the build.
- At the time of this v0.1.17 record, the next package version was 0.1.18; that version is recorded below.

## v0.1.18 package record

- Source: `8769324` on `main` (bounded periodic PCM pipeline counters).
- Artifact: `ps3-airplay-receiver-0.1.18.pkg`, AIRPLAY02, APP_VER `0.1.18`.
- Size / SHA-256: 3,230,448 bytes / `5548fde98bc08949e4f3a6739a95d5b3dce7dea0458131ec693c01be97766ae0`.
- ELF: 2,780,120 bytes, CellOS PPC64, five PT_LOAD segments; SHA-256 `ba7450441a8e2def2a1f55e17d523630d3d5af256520f6769154a347b09d3759`.
- Build: Existing Lima VM; PS3DK v0.27.0 PPU compiler 16.1.0, GNU ld 2.42, PSL1GHT revision `dd4e8c9be744e3273d665c393fa6d97a87402e2e`.
- Checks: full Lima host suite, PPU object compilation, standalone link/ELF validation, extracted SFO/title/content ID/assets, EBOOT equality, and finalized NPDRM envelope PASS.
- RPCS3 install/launch and emitted telemetry verification: pending. No physical hardware or audible playback evidence.
- The next unused package version after the v0.1.18 record was 0.1.19; its build record follows.

## v0.1.19 package record

- Source: `7b451031487aab7dbad147ab0b47ef80a1e22265` on `main` (ALAC negotiated sample-count bounds and composed encrypted RTP-to-PCM host test).
- Artifact: `ps3-airplay-receiver-0.1.19.pkg`, AIRPLAY02, APP_VER `0.1.19`.
- Size / SHA-256: 3,230,704 bytes / `41488b9683b901ecc4d71e113565fcdb10c1834c69e1642fbe774f5cdf4a31a5`.
- ELF: 2,780,120 bytes, CellOS PPC64, five PT_LOAD segments; SHA-256 `beb816b2aed2125a94584e4e820f18de66a98f21260ce47fe1654199322e7ec3`.
- Build: Existing Lima VM; PS3DK v0.27.0 PPU compiler 16.1.0, GNU ld 2.42, PSL1GHT revision `dd4e8c9be744e3273d665c393fa6d97a87402e2e`.
- Checks: full Lima host suite under ASan/UBSan, PPU component compilation, standalone link/ELF validator, package extraction, AIRPLAY02/APP_VER/content ID, asset equality, EBOOT equality, relative package paths, and finalized NPDRM envelope PASS.
- The package and ELF are preserved in the ignored local `work/builds/0.1.19/` directory. RPCS3 install/launch was not performed because Computer Use still reports the Mac desktop locked. No physical-console transfer/install, iPhone session, or audible playback is verified.
- Next unused package version: 0.1.20.

The standalone receiver `Makefile` and canonical Lima build/package scripts default to 0.1.26. For RPCS3, override `RTSP_PORT=5001` only when the host's TCP 5000 is occupied; keep production builds on 5000.

## v0.1.20 package record

- Source: `8a4e641d114e1a613421b9ffb51f8615ad851827` on `main` (ALAC malformed-bitstream bounds and arithmetic hardening).
- Artifact: `ps3-airplay-receiver-0.1.20.pkg`, AIRPLAY02, APP_VER `0.1.20`.
- Size / SHA-256: 3,230,384 bytes / `bfaa3f960de39daf95c2ab405bd361a031ed92bd35c76a99e667d4ffe8d19e87`.
- ELF: 2,780,120 bytes, CellOS PPC64, five PT_LOAD segments; SHA-256 `f50e6a4fe247149feb7759a69a81ae881c0972d789cb1a5dcfa2019bd202b3d9`.
- Build: Existing Lima VM; PS3DK v0.27.0 PPU compiler 16.1.0, GNU ld 2.42, PSL1GHT revision `dd4e8c9be744e3273d665c393fa6d97a87402e2e`.
- Checks: Full Lima host suite with ASan/UBSan halt-on-error, deterministic 256 malformed ALAC inputs, encrypted RTP-to-PCM golden test, PPU app link/ELF validation, SFO/version/title/content ID, assets, extraction, EBOOT comparison, and finalized NPDRM envelope PASS.
- Runtime: RPCS3 is not tested; current Computer Use reports the Mac desktop locked. No physical PS3 transfer/install, AirPlay sender session, or audible output is verified.
- The package, ELF, and build-info are preserved locally under ignored `work/builds/0.1.20/`. Next unused package version: 0.1.21.

## v0.1.21 package record

- Source: `3e529faac64b0b30989b955e01c991e63fdaeda8` on `main` (correct RAOP `vs` server-version TXT value).
- Artifact: `ps3-airplay-receiver-0.1.21.pkg`, AIRPLAY02, APP_VER `0.1.21`.
- Change: advertise `vs=105.1`, matching the RTSP `Server: AirTunes/105.1` header; the previous `vs=0.1.2` was stale package metadata.
- ELF: 2,780,120 bytes, CellOS PPC64, five PT_LOAD segments; SHA-256 `401225f2c8b704103c5f65683a5b48f9c7b34633ae57145ca92c190485543e1e`.
- Size / SHA-256: 3,230,384 bytes / `c8c77b78d34dc50a955383ddafe7c835a7848c62eb8a9673ede7ed0c2776ef73`.
- Build: existing Lima VM; PS3DK v0.27.0 PPU compiler 16.1.0, GNU ld 2.42, PSL1GHT revision `dd4e8c9be744e3273d665c393fa6d97a87402e2e`.
- Checks: RAOP TXT serializer assertion, full Lima host suite with ASan/UBSan, PPU component checks, standalone ELF validation/link, AIRPLAY02/APP_VER/content ID, asset equality, extraction, EBOOT comparison, relative package paths, and finalized NPDRM envelope PASS.
- Runtime: RPCS3 Computer Use reports the Mac is locked; no fresh v0.1.21 install/launch or matching log. Physical PS3 was not changed. No iPhone session or audible AirPlay playback is verified.
- The PKG, ELF, and build metadata are preserved in ignored `work/builds/0.1.21/`. Next unused package version: 0.1.22.

## v0.1.22 package record

- Source: `7b44acff578ce91d7cfbe963f1156f5157b23063` on `main` (bounded successful-decryption diagnostics).
- Artifact: `ps3-airplay-receiver-0.1.22.pkg`, AIRPLAY02, APP_VER `0.1.22`.
- Change: count successfully decrypted audio RTP payloads and log `RTP_DECRYPT_OK` for the first success and each 128-packet milestone. Failed decryptions retain `RTP_DECRYPT_FAILED`; the log contains only a bounded numeric count and payload byte count.
- ELF: 2,780,120 bytes, CellOS PPC64, five PT_LOAD segments; SHA-256 `ecfd3afa2aaafe35e60d8a6fa7540a659a8757bd045232b07a141230279ad2b0`.
- Size / SHA-256: 3,230,512 bytes / `c7242d47669a4a22090663c969e2b4f5efd4c0cc9a0eb6ef65ad18d730080a2b`.
- Build: existing Lima VM; PS3DK v0.27.0 PPU compiler 16.1.0, GNU ld 2.42, PSL1GHT revision `dd4e8c9be744e3273d665c393fa6d97a87402e2e`.
- Checks: full Lima host suite with ASan/UBSan, composed encrypted RTP-to-PCM test, PPU component compilation, standalone link/ELF validation, AIRPLAY02/APP_VER/content ID, asset equality, extraction, EBOOT comparison, relative package paths, and finalized NPDRM envelope PASS.
- Runtime: RPCS3 still has only the older v0.1.15 app log; the desktop is locked and no fresh v0.1.22 launch/log was obtained. Physical PS3 was not changed. No iPhone session or audible AirPlay playback is verified.
- The PKG, ELF, and build metadata are preserved in ignored `work/builds/0.1.22/`. The next package is v0.1.23.

## v0.1.23 source pass

- Change: configure a two-second `SO_SNDTIMEO` for each accepted RTSP client and enforce a two-second overall deadline across response writes. Timeout/setup failures are logged without peer details.
- Validation: full Lima host/sanitizer suite and PPU component compilation pass; the RTSP socket integration forces a slow-reader backpressure condition, verifies the timeout event, and confirms disconnect within four seconds.
- Full standalone ELF/PKG build and runtime validation pending. RPCS3 is still inaccessible while the desktop is locked.

## v0.1.23 package record

- Source: commit `6513bdd201899e93fd55babd296fb97893d93216` on `main`.
- Package: `AIRPLAY02`, version `0.1.23`, finalized NPDRM PKG, 3,230,768 bytes; SHA-256 `d17f6d2576bb96d0ff29cd7a58790852b6607d9b60b5d2dcffbe5613e053c4c4`.
- ELF: CellOS PPC64 executable, 2,780,120 bytes; SHA-256 `2130c17ace5459a57765d3d2ae8b185012548789891961e3b74d330d6f236dd6`.
- Checks: Lima PS3DK v0.27.0 + PSL1GHT build; ELF validation; finalized package envelope, AIRPLAY02 identity, APP_VER, content ID, EBOOT and embedded background validation pass. The full Lima host/sanitizer and PPU component suite passed in the source pass.
- Runtime: RPCS3 install/launch could not be performed because desktop access still reports the Mac locked. No fresh v0.1.23 runtime log, iPhone session, or audible AirPlay playback is verified. Physical PS3 was not changed.
- Artifacts are preserved in ignored `work/builds/0.1.23/`. Next package version: 0.1.24.

## v0.1.24 source pass

- Change: emit bounded RTP health checkpoints every 128 successfully parsed audio packets. Log received packet and payload deltas and queue depth; emit missing, reordered, duplicate, late, and queue-drop deltas only when nonzero. FLUSH resets the checkpoint baseline so old-stream errors are not reported as new-stream health.
- Validation: full Lima ASan/UBSan suite and PPU component compilation pass. The receiver loopback verifies a 128-packet checkpoint, queue depth, a post-FLUSH sequence gap, and the resulting one-packet missing delta. Standalone ELF/PKG build pending.
- RPCS3 remains inaccessible because desktop automation reports the Mac locked; no physical console or audible playback claim.

## v0.1.24 package record

- Source: commit `a59aa937401d432b5cc6f740eeda15bf0db091ba` on `main`.
- Package: `AIRPLAY02`, version `0.1.24`, finalized NPDRM PKG, 3,231,152 bytes; SHA-256 `7638825e0c2f6eb26a19410ba56ffd52ed96a70dc274f691b6bf2d12cf615757`.
- ELF: CellOS PPC64 executable, 2,780,120 bytes; SHA-256 `21b6ea799b18fd91f63b6d6dcb8d9f490e980f775c6bf3b96f84609b1dfdf9c2`.
- Checks: Lima PS3DK v0.27.0 + PSL1GHT build; ELF validator; finalized package envelope, AIRPLAY02 identity, APP_VER, content ID, EBOOT and background asset validation pass. The full Lima ASan/UBSan host suite and PPU component compile passed in the source pass.
- Runtime: RPCS3 install/launch is pending. The user reported unlocking the Mac, but the automation runtime still reports it locked. No fresh v0.1.24 app log, iPhone session, or audible playback is verified; physical PS3 was not changed.
- Artifacts are preserved in ignored `work/builds/0.1.24/`. Next package version: 0.1.25.

## v0.1.25 RPCS3 probe package

- Source revision: `a59aa937401d432b5cc6f740eeda15bf0db091ba`.
- Build: PS3DK v0.27.0 PPU + PSL1GHT; emulator-only `RTSP_PORT=5001`. This override is for the occupied host port in RPCS3; production default remains TCP 5000.
- AIRPLAY02 PKG: 3,231,152 bytes; SHA-256 `420971af3601d0a9813d4853f389f7e4ceb1c9f4a6077e27fcc3b81b292cbcd4`.
- ELF: 2,780,120 bytes; SHA-256 `848b698f2feaa832cddd88f1610452174a42018cdd0a126ca9d35a5abd724dde`.
- SFO APP_VER/title/content ID, CellOS PPC64 ELF, package structure, and finalized NPDRM envelope passed.
- RPCS3: app starts, RTSP/RTP sockets open, five-second tone completion is logged, and local OPTIONS gets 200. mDNS multicast-interface setup fails. Visual display, controller exit, audible tone, iPhone connection, RTP audio, and Spotify playback are unverified.
- Payload was staged directly from the validated extraction into local RPCS3 title data because its desktop installer UI was unavailable. Prior local title data is preserved for rollback. Physical console unchanged.
- Next package version: 0.1.26.

## v0.1.26 source pass

- Adds allowlisted session diagnostics around SDP parsing and RSA-wrapped session-key processing, and a reusable host-to-RPCS3 test mode that sends a synthetic RAOP session plus 128 encrypted ALAC RTP fixture packets.
- Full Lima host/ASan/UBSan suite, crypto/session diagnostic assertions, remote probe build, and PPU component compilation pass. The first v0.1.25 remote ANNOUNCE attempt stalled before reply; v0.1.26 runtime replay is pending.
- Standalone Makefile and canonical Lima build/package script defaults advance to 0.1.26. Keep the standard RTSP port at 5000; only RPCS3 test builds should override it to 5001 when needed.
- The package was subsequently built and replayed; see the v0.1.26 package record below. Next package version: 0.1.27.


## v0.1.26 package and RPCS3 replay

- Source revision: `6f8d6cb6f75e331d989419aaf51bd1d147704cad`; emulator-only RTSP port 5001.
- CellOS PPC64 ELF: 2,780,208 bytes, SHA-256 `eda789f1a6a7c3ab9ad3629c1d7acd0af14066f34e1cc3dac5bb198482d9210c`.
- AIRPLAY02 PKG: 3,231,408 bytes, SHA-256 `82e1407b92a1218ec3085f57bd8f6c35e97255569b92a44249e14239108fc246`. Package and payload validation passed. Local artifact is retained under ignored `work/builds/0.1.26/`.
- RPCS3 fresh log confirms startup, route, sockets, OPTIONS 200, and successful SDP parse for ANNOUNCE, followed by no response. Event names over 24 characters were rejected by the logger, so RSA unwrap status is inconclusive; SETUP/RECORD/RTP were not reached. mDNS still fails in the emulator. No screen, audible tone, iPhone, or physical-console result is claimed.
- Full Lima host/sanitizer and PPU component checks pass. The direct local title-payload update preserved prior app data for rollback. The GUI desktop remains locked.
- Next package version: 0.1.27.


## v0.1.27 package and RPCS3 replay

- Source revision: `eebd588f4d4e7baa8da95d5f4127b9155d90b858`; emulator-only RTSP port 5001.
- CellOS PPC64 ELF: 2,780,208 bytes, SHA-256 `e053dac6dfb801dd3f19d1ceef45d12e10d590052313f9ee0b27d5433e22c440`. AIRPLAY02 PKG: 3,231,920 bytes, SHA-256 `0a3d1c409fb1044231256090cc3693da3e1aaa5abddd5bc6e00f29a9aa407b6c`.
- Package, SFO, content ID, extraction, assets, and finalized NPDRM validation passed. Local artifact is under ignored `work/builds/0.1.27/`.
- RPCS3 confirms startup, route, sockets, OPTIONS 200, and SDP/offer parsing. Its last accepted session event identifies encryption type 1 plus key and IV fields. The event-name limit filtered longer validation/RSA/response events; only the missing short validation-completion event is informative. SETUP/RECORD/RTP, display, audible sound, iPhone, and physical console remain unverified. mDNS setup fails in RPCS3.
- v0.1.28 adds bounded Content-Type scanning, but its new long diagnostic event names were rejected by the log field limit. Next package version: 0.1.29.


## v0.1.28 package and RPCS3 replay

- Source: `4308bb120d0b86e065481a1175990de47acb4a24`; emulator-only RTSP port 5001.
- CellOS PPC64 ELF: 2,780,208 bytes, SHA-256 `0a6fd755b1150d42b0be5b5b11b34b59b426c85476b5847d4d8fc3ce957bbc6d`. AIRPLAY02 PKG: 3,231,984 bytes, SHA-256 `0e3201647bb83b85c0df4e08dacc780c9daf1468c9d261e321c10be0f8fa9cef`.
- Package validation, Lima ASan/UBSan suite, and PPU component compilation passed. RPCS3 starts and answers OPTIONS 200; ANNOUNCE SDP and fields parse, but no response arrives. New Content-Type/RSA/response event names were filtered by the 24-character logger limit, so their absence is inconclusive. The short validation-completion event is absent. No SETUP/RECORD/RTP or audio evidence.
- Local title payload was staged after preserving v0.1.27 data. Desktop remains locked; physical PS3 and boot plugins unchanged.
- v0.1.29 fixes the logger event-name limit and adds a regression test.

## v0.30 compatibility candidate

Release label 0.30 maps to PS3 SFO 00.30. Packaging uses the preserved v01.01 type-1 debug container. Older three-component SFO values are now rejected. Production RTSP remains 5000; hardware launch/display/exit and actual AirPlay playback require fresh evidence. See the current decision log and memory for validation boundaries.

## Hardware evidence after SFO correction

The historical 0.30 build starts and plays its tone on the PS3; START exits while idle (maintainer report). A real iPhone attempt ends in an unanswered OPTIONS request and no completed RAOP session. No streamed music. START and PS input become unresponsive during that attempt. Corrected release label: 0.1.30; SFO remains 00.30. Resolve the request-handling stall before extended playback tests.

## v0.1.31 / SFO 00.31

Disables incompatible Mbed TLS PPC64 assembly on the PS3 ABI and adds challenge/RNG/handler/cleanup checkpoints. Source 224f15d. Package 3,233,584 bytes, SHA-256 354dc5f33ef19c9342fee12efe21b70d2a7818d26660bc7ee439327675b4d60d. Installed payload verified. RPCS3 challenge and encrypted RTSP session complete; physical music/exit result pending.

## v0.1.32 / SFO 00.32

Installed from source e34cbce. Corrects ALAC host byte order; adds regular nonblocking presentation, input-first polling, bounded graphics cleanup and rate-limited diagnostics. PKG 3,234,544 bytes; SHA-256 fca5547fda865f22faff7099834d0c3a8e77e6658f955b5c3a4a941cfd7ac7b3. PPU golden PCM comparison and full-app paced RPCS3 RTP/decrypt/decode/PCM buffering pass. Physical XMB overlay, exit after disconnect and audible music await maintainer testing.


## v0.1.33 / SFO 00.33 hardware playback result

- Source `7352a77cbaa7d13d57ad93a2788bdc0ba26e4fd2`; AIRPLAY02 PKG 3,235,056 bytes, SHA-256 `aa3b456caa983b85b26fa63a43289b2a301a262a2d9d5f8748783897ddea79f5`. Installed EBOOT/SFO matched the candidate.
- Maintainer confirms multiple AirPlay Spotify songs were audible. Audio crackled and remained unstable/low fidelity; the display flickered after XMB resume. The retained hardware log confirms RTP/decrypt/ALAC/PCM activity, with 106 sequence gaps in the visible 1,792-packet window and zero sampled ALAC failures. The log wrapped and exact session-wide counts are unavailable.
- v0.1.34 / SFO 00.34 adds bounded pending/outstanding RTP resend state and less noisy aggregate logging. Source commit `f5f28c4c0e505722fc689947955da8f57fab1c9e`; validated 2,780,832-byte PPU ELF SHA-256 `e4e5367f41341018ebcf825a68a33880346dee3666aa7a0e0cd50da43bc9f4bc`, and 3,236,016-byte finalized PKG SHA-256 `8828e032fea57ab1ce4e0041f0c802c45940f3c7abe93caf3d9969fa9f01e80f`. Lima ASan/UBSan suite, PPU components/application link, SFO/ELF/content/package validation passed. PKG is staged on PS3 and byte-verified; latest installed SFO/EBOOT still match v0.1.33. v0.1.34 hardware behavior is not yet tested.

## v0.1.35 / SFO 00.35 candidate

Combines v0.1.34's bounded RTP resend-range recovery with valid PCM underflow accounting: intentional silence before decoder/prebuffer readiness no longer increments stream underruns, and session counters reset at recording boundaries. Source commit `51d3be9a0c737e7b11aa442defe90e69061051fb`; 2,780,888-byte CellOS PPC64 ELF SHA-256 `435c6f3e37832376f92d0d8fccc5c088880d0fc20f503235344e314d9990a497`. Finalized 3,236,016-byte AIRPLAY02 package SHA-256 `aba1d95124d85ff84c5d2442235968ad00b25e24871a3875f90dd7080ab74c9b`; SFO `00.35`. Lima host/sanitizer tests, PPU components/application build, ELF/SFO and package validation pass. Package is staged and byte-verified but not installed because v0.1.33 is still running. Hardware behavior is pending.


## v0.1.36 / SFO 00.36 candidate in progress

Live read-only evidence captured while AIRPLAY02 v0.1.35 was running: installed SFO 00.35; staged v0.1.35 package hash matched the recorded artifact. In the wrapped active-stream log, 2,048 RTP packets, 302 sequence gaps, 209 late packets, 58 resend requests, zero recovered packets, zero ALAC failures, and 102,200 valid PCM underflow frames were recorded. The receiver previously discarded a valid peer resend when its tracking range was absent, without distinguishing that case from a late/duplicate packet. Candidate source adds bounded untracked recovery inside the 256-packet receive window and reason-coded aggregate counters. Host/Lima tests and PPU component compilation pass. Standalone build/package and emulator/hardware tests remain pending; no clean-audio or display-resume claim.

## v0.1.37 stability candidate

- Release label `0.1.37`; PS3 SFO `00.37`; application ID `AIRPLAY02`.
- Includes v0.1.36's bounded in-window late RTP resend recovery and changes the static-screen renderer to submit only a menu-resume refresh instead of flipping continuously.
- Fresh console baseline remains v0.1.35 / SFO 00.35. v0.1.37 build, package hashes, RPCS3 replay, and hardware validation are pending.
- Production RTSP remains port 5000. Use port 5001 only for emulator testing when the host port conflicts, and label that package as an emulator variant.


## v0.2.4 PS-menu refresh and append-only diagnostics deployment

- Source changes: keep paced framebuffer flips active during `SYSUTIL_MENU_OPEN`, while leaving the static image idle otherwise; open the diagnostic log in append mode; cap each launch independently at 16 KiB with a `LOG_SESSION_CAPPED` marker. The first `APP_START` record in each run carries its UTC launch timestamp.
- Version: app/package `0.2.4`; SFO APP_VER/VERSION `00.44`.
- Source: `f870d823f15398691a6a38e6393f8d1376161bee` on `main`.
- Artifact: `ps3-airplay-receiver-0.2.4.pkg`, AIRPLAY02 app version `0.2.4`, SFO APP_VER/VERSION `00.44`.
- Size / SHA-256: 3,231,984 bytes / `a27d5c7b5d2d84f12fbb82ee4e164b9b50ccc825b5104b9b749f053d72a42bf7`.
- ELF: 2,754,776 bytes, CellOS PPC64 executable, five PT_LOAD segments; SHA-256 `bd492c726b12e503d268e00cf03dee82c2c3d6f2aac7fa8f4f07c85a68b2e8ae`.
- Build: existing Lima VM, PS3DK v0.27.0 PPU compiler 16.1.0, GNU ld 2.42, PSL1GHT revision `dd4e8c9be744e3273d665c393fa6d97a87402e2e`.
- Checks: full Lima host ASan/UBSan and PPU component suite; standalone ELF validation; package extraction, launch metadata, content ID, assets and debug NPDRM envelope PASS.
- Deployment: FTP upload/download-back hash matched; installed AIRPLAY02 SFO 00.44 matches exactly and installed EBOOT matches the package SELF exactly. Hardware-tested: PS menu opens, START exits, and AirPlay music streams; resume display is dim/frozen and audio stutters. A 12-minute passive observation elapsed, but the old 16 KiB cap stopped telemetry after 37 seconds.

## v0.2.5 diagnostic logging candidate

- Scope: increase the append-only per-launch cap from 16 KiB to 5 MiB; add a combined fail-fast Lima release gate and a regression that exercises the full cap without truncating earlier sessions.
- Version: app/package `0.2.5`; SFO APP_VER/VERSION `00.45`.
- Source: `9d9f60657f117daedc47c6b1bc3285a5982e5ae3` on `main`.
- Artifact: `ps3-airplay-receiver-0.2.5.pkg`, 3,232,048 bytes; SHA-256 `61e8310a6d4330bad1e4d64213365f9ba27de81a6fa14851a315f5b338894440`.
- ELF: 2,754,776 bytes; SHA-256 `1ff2513b9823792aa4f202ddf700e161bee5f223e3ccc2fc19e53e39a3f1eead`.
- Checks: combined release gate PASS: host/ASan/UBSan tests, full-cap append regression, PPU component/app build, CellOS ELF validation, AIRPLAY02 identity, SFO `00.45`, package extraction and debug NPDRM envelope.
- Preserve v0.2.4 as the known-safe functional rollback baseline while audio stutters and dim resume remain open.
- Package built and retained locally under ignored `work/builds/0.2.5/`; not transferred or installed while v0.2.4 playback was left undisturbed. Hardware validation pending.
