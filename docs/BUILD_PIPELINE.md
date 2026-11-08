# Standalone PS3 build and package pipeline

## Canonical path

Use the existing x86_64 Lima guest with the pinned PS3DK v0.27.0 PPU toolchain, the recorded PSL1GHT checkout, and the matching Mbed TLS PPU library. The legacy GCC 7.2/binutils 2.22 script is an ABI comparison experiment only; it is not the release build path.

The v01.01 reference decrypted by RPCS3 is an ELF64 big-endian PPC64 `ET_EXEC`, CellOS OSABI `0x66`, entry `0x10000000`. Its first four `PT_LOAD` flags are `0x400005`, `0x600006`, `0x400004`, `0x600006`, followed by the PS3-specific zero-sized segment. The v0.1.6 legacy-linker ELF had the generic first flags `0x5` and `0x6`. Their shared layout is evidence that the PS3DK path preserves the old artifact's executable contract; it does not prove the complete v01.01 build or runtime is reproducible.

## Required per-build release gate

Run one command for every standalone package so host regressions, PPU component checks, the standalone ELF build, and package validation cannot be accidentally skipped:

```sh
export PS3DK_SOURCE="$HOME/ps3-airplay-host/ps3-sdk-src-v0.27.0"
export SOURCE_REVISION="$(git rev-parse HEAD)"
export APP_VERSION=0.2.5
export SFO_VERSION=00.45
bash scripts/release-lima-standalone-receiver.sh
```

The release gate requires a clean tracked worktree and a full matching source revision. It runs `build-lima-crypto.sh`, `build-lima-standalone-receiver.sh`, generates the runtime RGB background from `assets/air-ps3/artwork-source.png`, validates `ICON0.PNG` and `PIC1.PNG` as direct resizes of that source, then runs `package-lima-standalone-receiver.sh` with fail-fast behavior. The image conversion requires Pillow (`python3-pil` on Ubuntu). The packager includes project-managed artwork rather than the generic SDK icon and verifies extracted asset equality. Keep build outputs and package files under ignored `work/`; never deploy a package that failed any stage. Record the exact source revision and output hash. The first physical launch remains a separate hardware test.

AIR PS3 packaging defaults are `XMB_TITLE="AIR PS3"`, `APP_VERSION=0.2.8`, and `SFO_VERSION=00.48` for this candidate. The RAOP DNS-SD instance uses the existing hardware-derived unique prefix followed by `@AirPS3`; the hardware address itself is not committed. The existing title/content ID remain unchanged.

The release path is:

1. Run `scripts/release-lima-standalone-receiver.sh` for the combined fail-fast gate; its first stage runs `scripts/build-lima-crypto.sh` for portable protocol/audio tests and PPU object checks.
2. Export the pinned toolchain paths, exact source revision, and next unused `APP_VERSION`.
3. Run `scripts/build-lima-standalone-receiver.sh`. It builds Mbed TLS, clean-builds the receiver, and validates the resulting ELF against the v01.01 ABI/layout reference. It emits a build manifest with the source revision, compiler/linker versions, PSL1GHT revision, and ELF hash.
4. Run `scripts/package-lima-standalone-receiver.sh`. It verifies the ELF again, creates the NPDRM SELF and matching SFO/content ID, builds the package under a relative content-ID directory, extracts and checks the unfinalized package (including APP_VER and asset bytes), then validates a type-1 debug package matching v01.01. Finalized NPDRM format is an explicit optional mode. The bundled `pkg` reader cannot extract finalized NPDRM type `0x80000001`, so extraction is deliberately checked before finalization. The script refuses to overwrite a versioned package or intermediate.
5. Install and launch only in RPCS3 for the first gate. Keep physical console deployment separate and require the established recovery procedure and user authorization.

Example, with paths adjusted to the local Lima guest:

```sh
export PS3DK_SOURCE="$HOME/ps3-airplay-host/ps3-sdk-src-v0.27.0"
export PS3DEV="$PS3DK_SOURCE/stage/ps3dev"
export SOURCE_REVISION="<full SHA of the exact source commit copied into Lima>"
export APP_VERSION=0.1.30
export SFO_VERSION=00.30
# Optional emulator-only probe when the host's port 5000 is occupied.
export RTSP_PORT=5001
bash scripts/build-lima-crypto.sh
bash scripts/build-lima-standalone-receiver.sh
bash scripts/package-lima-standalone-receiver.sh
```

Keep the generated package, build manifest, and test logs outside Git. Record package SHA-256, size, source revision, and emulator outcome in a sanitized report. Never treat matching ELF headers or package extraction as proof of PS3 launch, visible display, AirPlay connection, or audible music.

## Current evidence and limits

- The preserved AIRPLAY02 v01.01 package and the modern PS3DK candidate share the validated ELF header and PS3 `PT_LOAD` flag pattern; they are not byte-identical.
- The v0.1.6 legacy runtime build fails the new layout validator, directly exposing a real ABI/layout regression in that path. This discrepancy is not yet proven to be the sole reason RPCS3 rejected v0.1.6.
- The PS3DK PPU compiler is GCC 16.1.0 in the current Lima toolchain; compiler and linker versions are captured per build.
- v0.1.7 (source revision `95f5924`) installed in RPCS3 and reached `APP_START`, `SCREEN_READY`, controller/network module initialization, and crypto initialization. It then logged RTSP bind error value `112`, `RECEIVER_OPEN_FAILED`, and `APP_STOP`. This is a successful loader/startup boundary followed by a network/socket failure; it does not establish display output or AirPlay operation.
- The RPCS3 network configuration was observed disconnected during this run. The relationship to bind error 112 is plausible but unproven; do not change emulator network settings without authorization. Physical PS3, visible display, clean XMB exit, iPhone RAOP, and streamed-audio results remain separate tests.

# v0.1.8-v0.1.13 follow-up

- v0.1.8 was built from the corrected PS3DK/Lima pipeline. It installed and reached `APP_START` in RPCS3, then failed RTSP bind with error value 112. RPCS3's network status was observed as disconnected; causation is unknown. The app stopped before receiver readiness.
- v0.1.9 moves the existing bounded local IPv4 readiness check ahead of receiver socket creation. In RPCS3, this produced `NET_IP_TIMEOUT` while networking was disabled and `NET_IP_ROUTE_READY` after enabling networking; the RTSP bind still failed with value 112.
- A wildcard host TCP listener occupied 5000 during the connected run. This is a port-conflict hypothesis, not a proven cause. The v0.1.10 build input permits a controlled test on 5001; the normal RTSP port remains 5000, and mDNS publishes the actual listener port.
- v0.1.10 bound RTSP on 5001 and RTP UDP on 6000-6002 in RPCS3, then logged `MDNS_FAILED`. Audio initialized and the diagnostic tone completed. No AirPlay discovery/session or music was established.
- v0.1.11 visibly rendered the supplied background and `PRESS START TO EXIT` in RPCS3 without blue bands or `DISPLAY ACTIVE`. It still logged `MDNS_FAILED` without a substage. The screen result is emulator evidence; the user's report separately confirms the app opens.
- v0.1.12 adds a numeric mDNS failure stage and errno to the privacy-safe `MDNS_FAILED` event. RPCS3 logged stage 4 (UDP bind) and errno 112 (`EADDRINUSE`). A second run with Lima stopped still failed: the host's system mDNS responder owns wildcard UDP 5353, and a wildcard bind probe returned `EADDRINUSE`. RPCS3 mDNS cannot bind on this host; this says nothing about native PS3 port availability.
- v0.1.13 was built from pushed source revision `70fa6654bd2d092a5f674dd526c05a2e5ccd4703` with the normal PS3 RTSP port 5000. All Lima host tests, PPU objects, ABI validation, PKG extraction, SFO/content-ID, asset equality, and finalized envelope checks passed. The package is 3,224,944 bytes; SHA-256 `f7d076e32520a552349ebb973894409d66fd429ff255d0919c66401fabe9f1e9`. Its RPCS3 and physical-console launch gates remain pending.
- v0.1.14 includes the best-effort SO_REUSEPORT request. The PS3DK header defines the option, and the PPU build compiled it. A host socket probe bound UDP 5353 and received multicast while the system mDNS responder remained active. RPCS3 installed and launched v0.1.14, but its fresh app log reached `NET_IP_TIMEOUT` before receiver startup; mDNS sharing was not exercised in the emulator. The physical PS3 remains untested for this change.
- v0.1.14 was built from source revision 6d9f28d0f079c445b96203695ea1912e8deed887. The 3,225,008-byte finalized PKG passed ELF, content ID, SFO, assets, and envelope validation; SHA-256 `5994cf4828976245f530b1c90d180fd4f0e80abddcfd0d09d3780c351ad006c1`.
- v0.1.15 was built from source revision `99abc5b`, including bounded RTSP FLUSH propagation through RTP and PCM buffering. The 2,758,576-byte CellOS PPC64 ELF validated against the v01.01 reference. The finalized PKG is 3,225,648 bytes; SHA-256 `230a2df0db8a7e561f3b048872c1a714ed0eb58f4b741587ee961f71834ffa59`. Package extraction, AIRPLAY02/APP_VER, content ID, assets, and finalized envelope checks passed.
- RPCS3 installed and booted v0.1.15. Its fresh log reached APP_START, SCREEN_READY, CONTROLLER_READY, NETCTL_READY, then `NET_IP_TIMEOUT` and APP_STOP result 1. RPCS3's own network layer reported no available address. No receiver sockets, mDNS, tone, or FLUSH-over-RAOP behavior was reached. The emulator's Internet setting is currently Disconnected; connected-route retesting is tracked separately from physical PS3 deployment.
- v0.1.16 was built from pushed source revision `0a204b04364803e2af9fcc8adf2f7420751ac83f`, including `SET_PARAMETER` volume/mute and a bounded PCM gain ramp. PS3DK v0.27.0 PPU + PSL1GHT produced a 2,780,096-byte CellOS PPC64 ELF, SHA-256 `968f9c3238e7c8d4479c0191452f378f4705d7d6585c68364cfcd7d792682f2a`; compiler 16.1.0, linker 2.42, PSL1GHT revision `dd4e8c9be744e3273d665c393fa6d97a87402e2e`. Final AIRPLAY02 package: 3,231,088 bytes, SHA-256 `3c69bff7ab626c433728ca131bc04ab0adbc117b8845d88c748e1dad1968bd3c`. ELF, APP_VER/SFO, content ID, five package items, background/icon assets, pre-finalization extraction, and finalized NPDRM envelope passed.
- v0.1.17 was built from pushed source revision `4a06c2a`. It removes the remaining text overlay from the standalone display. The 2,780,064-byte CellOS PPC64 ELF passed the ABI validator (SHA-256 `5e80257671409775d98884c7101acf15d7e116448498ee8c69148843a96896dd`). The 3,230,256-byte finalized AIRPLAY02 package passed SFO/APP_VER, content ID, asset equality, pre-finalization extraction, and NPDRM envelope checks (SHA-256 `2095046e20716d9f2768c847691515f1d1e922e5fbd96e15829f92c45816cf23`). Full Lima host tests and PPU object checks passed.
- v0.1.17 has not been installed or launched in RPCS3. The UI remains unavailable, so its visual output and runtime behavior are untested. The saved Internet setting was not changed.
- v0.1.18 adds five-second numeric PCM telemetry for ring occupancy, frames queued/consumed, and frames submitted to GameOS audio blocks. Full Lima host tests, PPU objects, the canonical 2,780,120-byte CellOS PPC64 ELF, and finalized package checks pass. ELF SHA-256 `ba7450441a8e2def2a1f55e17d523630d3d5af256520f6769154a347b09d3759`; 3,230,448-byte PKG SHA-256 `5548fde98bc08949e4f3a6739a95d5b3dce7dea0458131ec693c01be97766ae0`. Block submission is not audible-output evidence.
- v0.1.18 has not been installed or launched in RPCS3. UI access remains unavailable, no emulator setting changed, and no runtime log event is verified.
- The historical v0.1.21 note above is superseded by the v0.1.25 RPCS3 probe below. Every generated package consumes its version even when it fails a test.

## RPCS3 route and RTSP probe (2026-11-07)

- The exact v0.1.24 package payload was copied into RPCS3 app data after preserving the prior title data. With RPCS3 Internet disconnected, the app reached its bounded `NET_IP_TIMEOUT` and stopped. After the authorized local emulator setting was changed to Connected, NetCtl obtained a route but TCP 5000 could not bind because that host port was occupied.
- The existing build override `RTSP_PORT=5001` was used for v0.1.25. The app log confirmed RTSP 5001 and RTP UDP 6000–6002 readiness. mDNS failed at interface setup (failure stage 6, error 125), before any service could be confirmed discoverable.
- A localhost RTSP `OPTIONS` probe received `200 OK`; the matching app log recorded `RTSP_CONNECTED`, `RTSP_OPTIONS`, `RTSP_STATUS=200`, and `RTSP_PEER_CLOSED`. The diagnostic tone start and completion were logged. No audible output was observed, and no iPhone session or audio stream was tested.
- v0.1.25 package: 3,231,152 bytes, SHA-256 `420971af3601d0a9813d4853f389f7e4ceb1c9f4a6077e27fcc3b81b292cbcd4`. Build revision `a59aa937401d432b5cc6f740eeda15bf0db091ba`; build manifest records port 5001. The extracted package payload—not the GUI PKG installer—was used to update the local RPCS3 title data; the previous title files were preserved locally for rollback.
- Do not change the production default from port 5000 based on this emulator-only conflict. The emulator's multicast-interface failure and the host's occupied port are environment constraints that need separate treatment from PS3 behavior.
- The desktop remained locked, so the renderer's output and controller START exit could not be visually checked. The physical PS3 and its system/plugin settings were not touched.
- Next package: v0.1.26.

## ANNOUNCE stall and phase logging (2026-11-07)

- A Lima host sender connected to the running v0.1.25 RPCS3 app through the local host bridge. `OPTIONS` received 200 and `ANNOUNCE` reached the receiver's method log, but no RTSP response arrived within the probe's three-second deadline. The app log ended its protocol trace at `RTSP_ANNOUNCE`; the exact blocked operation is unknown.
- Added a session diagnostic callback for privacy-safe SDP parse and RSA key-unwrapping start/completion events. Added an opt-in remote mode to `raop_audio_pipeline_test` that sends the shared synthetic RAOP fixture through OPTIONS/ANNOUNCE/SETUP/RECORD and then sends 128 encrypted RTP packets.
- Full Lima host/sanitizer suite, assertions for diagnostic callback behavior, and PPU component compilation pass. A prior run exposed that `raop_receiver_test` read only 4 KiB from a log capped at 16 KiB; its test buffer now uses the configured cap, and the complete suite passes.
- The v0.1.26 package and RPCS3 replay are recorded below; the production RTSP default remains unchanged at 5000.


## v0.1.26 RPCS3 replay

- Built from pushed source `6f8d6cb6f75e331d989419aaf51bd1d147704cad` with `RTSP_PORT=5001`; the normal production default remains 5000.
- The 2,780,208-byte CellOS PPC64 ELF passed ELF validation (SHA-256 `eda789f1a6a7c3ab9ad3629c1d7acd0af14066f34e1cc3dac5bb198482d9210c`). The finalized AIRPLAY02 v0.1.26 PKG passed SFO, content ID, extraction, asset equality, and NPDRM checks: 3,231,408 bytes, SHA-256 `82e1407b92a1218ec3085f57bd8f6c35e97255569b92a44249e14239108fc246`.
- Full Lima host/ASan/UBSan tests and PPU object compilation pass from this exact source snapshot. RPCS3 started, obtained a route, opened RTSP 5001/RTP 6000–6002, and answered OPTIONS 200. For ANNOUNCE it logged SDP parse start and success, then no further session event or response arrived within the sender deadline. The RSA-start event name exceeded the logger’s 24-character limit, so its absence is inconclusive.
- The receiver payload was copied into RPCS3 local title data after preserving the prior AIRPLAY02 title directory; this was a direct local payload update, not GUI PKG installation. Prior title data and the capped log are preserved locally. Desktop CUA still reports locked. No physical PS3 changes were made.
- v0.1.27 source adds events around ANNOUNCE field classification, validation completion, and response generation to isolate the remaining boundary. The exact stall cause is still unknown.


## v0.1.27 RPCS3 replay and v0.1.28 follow-up

- v0.1.27 was built from pushed source `eebd588f4d4e7baa8da95d5f4127b9155d90b858`, using emulator-only `RTSP_PORT=5001`. CellOS PPC64 ELF: 2,780,208 bytes, SHA-256 `e053dac6dfb801dd3f19d1ceef45d12e10d590052313f9ee0b27d5433e22c440`. Finalized AIRPLAY02 PKG: 3,231,920 bytes, SHA-256 `0a3d1c409fb1044231256090cc3693da3e1aaa5abddd5bc6e00f29a9aa407b6c`.
- v0.1.27 package validation, host/ASan/UBSan suite, and PPU checks passed. RPCS3 fresh log confirms app/network/listener readiness, mDNS failure at stage 6/error 125, and OPTIONS 200. ANNOUNCE parsed successfully and logged field flags value 1539 with 344 bytes of wrapped-key data, then produced no validation-complete event or response. The probe did not reach RTP.
- The exact stalled expression is not yet proven. v0.1.28 replaces the Content-Type `strlen` call with a bounded length scan, logs its start/result, and asserts the result in the host session test. This makes the parse boundary explicit and avoids relying on an unbounded scan of a fixed-size parsed header field.


## v0.1.28 RPCS3 replay and v0.1.29 logger correction

- v0.1.28 source `4308bb120d0b86e065481a1175990de47acb4a24`, emulator RTSP 5001. CellOS PPC64 ELF: 2,780,208 bytes, SHA-256 `0a6fd755b1150d42b0be5b5b11b34b59b426c85476b5847d4d8fc3ce957bbc6d`. Finalized AIRPLAY02 PKG: 3,231,984 bytes, SHA-256 `0e3201647bb83b85c0df4e08dacc780c9daf1468c9d261e321c10be0f8fa9cef`.
- Package and host/ASan/UBSan + PPU checks passed. RPCS3 starts, opens sockets, and answers OPTIONS 200. ANNOUNCE logs SDP success, validation start, and offer flags; the sender receives no response. The app log does not contain the new Content-Type/RSA/response diagnostics because their event names exceed the logger’s 24-character limit and are silently rejected. Absence of those events is not evidence about which operation ran. The short `ANNOUNCE_VALIDATE_DONE` event is also absent.
- The remote sender did not reach SETUP/RECORD/RTP. Desktop CUA remained locked; no screen/audio observation or physical console change occurred.
- v0.1.29 increases the permitted event-name length, grows the formatted line buffer accordingly, and tests persistence of a 33-character diagnostic event. Replay .29 before drawing conclusions about ANNOUNCE internals.

## v0.30 hardware-launch compatibility pass

- Installed v0.1.29 SFO and EBOOT match the production-port package byte-for-byte. Maintainer confirms installation completes and Game entry fails with 80028F21. The 1,817-byte log remains identical to the old tone run; no fresh startup evidence.
- Concrete metadata defect: APP_VER and VERSION contained three-component release labels. PS3 version fields require XX.YY; preserved v01.01 uses 01.01. Release v0.30 uses APP_VER/VERSION 00.30 and retains 0.30 in app logs/filename.
- Packaging defaults to type 0x00000001 matching the preserved v01.01 container; optional finalized mode remains available. NPDRM content ID, title ID, icon/background, runtime source and ABI contract remain intact. Package format and SFO are changed together for a baseline-compatible candidate, so a future success cannot isolate their individual effects.
- Deferred unfinished RNG instrumentation to a private local patch; no protocol behavior changes in this pass. The new SFO validator rejects the actual v0.1.29 metadata and accepts the original v01.01 SFO.
- Provenance fix: builds require a matching full checkout revision or stamped source archive. Prior exported source files match main, but the older external build manifest carried an incorrect full SHA. Matching source content does not make that old manifest accurate.
- Build, upload, installation, visible image, tone, START exit and AirPlay remain separate gates. Hardware launch fix is a candidate until the maintainer confirms it.

## Current release identity

APP_VERSION controls the package filename and application logging; SFO_VERSION controls APP_VER/VERSION. Keep both explicit for every release. The 0.30 artifact was actually tested on hardware before the maintainer corrected its release label; preserve that historical identity in reports. Build from a committed archive stamped with its full SHA. Current logs append and use UTC-timestamped APP_START markers; the per-launch cap must be large enough for the planned test duration.

## v0.2.5 release gate result

- Combined `scripts/release-lima-standalone-receiver.sh` passed from clean source revision `9d9f60657f117daedc47c6b1bc3285a5982e5ae3`, app version `0.2.5`, SFO `00.45`.
- The first run found a missing `PS3DEV` export between the app and package stages. The wrapper now derives it from `PS3DK_SOURCE`; the full rerun reached and passed package validation.
- Full host ASan/UBSan suite and PPU component tests passed. The 5 MiB append test reached its cap and retained the earlier launch record.
- Standalone ELF: 2,754,776 bytes, SHA-256 `1ff2513b9823792aa4f202ddf700e161bee5f223e3ccc2fc19e53e39a3f1eead`.
- Debug PKG: 3,232,048 bytes, SHA-256 `61e8310a6d4330bad1e4d64213365f9ba27de81a6fa14851a315f5b338894440`; AIRPLAY02, SFO `00.45`, package extraction and NPDRM envelope validated.
- FTP upload/download-back matched the local PKG. Installed SFO is byte-exact; installed EBOOT is the package SELF plus a 48-byte zero pad. Subsequent v0.2.5 launch and 33-minute telemetry are documented in the forensic audit.

## v0.2.2 controller raw-mask build and hardware test

- Source `6151aae43b7ed26dfa940f090afbdd5b13ba53ac` adds PSL1GHT raw digital-mask fallback, mask-change logging, and positive pad-report counts per heartbeat. App version is `0.2.2`; SFO APP_VER/VERSION are `00.42`.
- Full Lima host/sanitizer tests, PPU component suite, standalone app build, and package validators passed. PPU ELF: 2,754,776 bytes, SHA-256 `554c2580944bb0cd6d214f96b410c5d2c2395da609beeca9230455d2a3aa1bf6`. Finalized PKG: 3,232,112 bytes, SHA-256 `44886cd6ce7bbeff8089bd2d5d752cce3b76ded01f36b214cfdaff057f6748cd`.
- Uploaded via FTP to a new filename, downloaded back byte-identically, installed via webMAN, and launched. Installed SFO is exact; EBOOT matches the packaged SELF followed by 32 zero bytes. Fresh log records `PKG_VERSION_0.2.2`, listener, mDNS and audio readiness.
- User reports START and PS still do nothing. The log has zero input reports in two 10-second windows, then a sysutil menu-open callback and two reports without a raw digital-mask change, START event, or menu-close. The callback does not prove the PS menu was visible. User will reboot and launch manually; wait for the resulting log before any further console command.

## v0.1.31: PS3 RSA arithmetic ABI fix

Saved RPCS3 v0.1.29 replay contains a PPU fatal invalid-memory read at 0x000a0b40, mapped by the exact ELF's addr2line to mbedtls_mpi_core_mul. Disassembly and upstream bn_mul.h show PPC64 assembly using ld/std on pointer variables, while the surrounding compiler-generated code uses 32-bit lwz/stw pointers. PS3 uses a 64-bit-register / 32-bit-pointer ABI; this assembly path is incompatible.

The PS3-specific Mbed TLS user config disables MBEDTLS_HAVE_ASM only for that ABI. Both PPU library build scripts and standalone compilation use the same config. Portable C bignum arithmetic preserves RSA blinding and system RNG; no cryptographic checks are removed. Added Apple-Challenge/signature/RNG phase events, total RTSP-handler milliseconds, system-menu callback event numbers, and cleanup phases. START exits the current loop immediately once detected. Version 0.1.31, SFO 00.31. Host crypto and RPCS3 replay results follow in the artifact report; PS3 outcome requires maintainer observation.

## v0.1.32: foreground controls, display cadence and ALAC byte order

Maintainer reports tone, AirPlay selection and crackling instead of music, then unresponsive START/PS after disconnect. FTP timed out during the open app, but HTTP retrieved a 5,122-byte log (SHA-256 baa8abdb0645a24707a70071d83dd670f52efb8827d73eca71c2664d3b58eeb3). It contains LOG_WRAPPED and 83 idle PCM_UNDERFLOW events; session/startup records are gone. The live loop is still servicing audio, so this is not evidence of a fully stalled main thread. A single documented webMAN normal quit produced SYSUTIL_EXIT_GAME, all cleanup completion events and APP_STOP 0; no reboot was sent.

Concrete defect: ALAC host_bigendian was fixed at zero despite the PS3 being big endian. Set from compiler byte order so decoded PCM remains little endian as required by pcm_ring. Controls/sysutil callbacks move before network service; pad reports are zero-initialized and only nonempty reports are used. Decode work is limited to four packets per iteration. Static-background scanout now receives nonblocking flips at about 60 Hz for the system compositor; this is a hardware overlay hypothesis requiring confirmation. RSX startup/close waits are bounded to 500 ms; on close timeout process teardown reclaims retained allocations, avoiding freeing memory still in use by RSX. Added heartbeat/pad/menu diagnostics and first-decode entry event; idle underflow spam is suppressed and recording underruns aggregate every five seconds. Version 0.1.32 / SFO 00.32.

## v0.1.33: menu resume and classic SDP encryption

Maintainer reports PS-button menu now opens, but resuming freezes its loading view, background flickers, and AirPlay still crackles. User could reopen XMB and quit. Retrieved saved log: 13,202 bytes, SHA-256 01358cd853dab686380e2f574f273a501cbf925e88e94aeccef1b2131f80cb78. It wrapped under decode-error spam (164 retained ALAC_DECODE_FAILED events, payload length 32), then clean TEARDOWN, draw/menu open-close events, and every cleanup completion through APP_STOP 0. The earlier negotiation fields are lost, so this does not prove the sender's encryption attributes.

Concrete protocol gap: key setup previously required a=et:1, although classic RAOP infers encryption from paired rsaaeskey/aesiv without et (also the maintained Shairport Sync behavior). Infer encryption only for a complete pair when et is absent; respect explicit et and reject incomplete pairs. Regression checks missing-et key recovery and correct plaintext, plus partial-pair rejection.

Display candidate restores the standard GPU-side wait-flip command ordering after flushing each flip. Retain the bounded RGB image to refresh both framebuffer images following MENU_CLOSE; reset the flip deadline after the compositor releases control. Do not apply a normal-frame timeout while the system menu owns presentation. Controller/event handling remains input-first. Rate-limit failure and decode-attempt logs; repeat stream-rate/encryption/error totals every five seconds to preserve negotiation context. Version 0.1.33 / SFO 00.33. Hardware resume/flicker/music result remains pending.


## v0.1.34: bounded RTP gap recovery

The v0.1.33 hardware log shows repeated RTP sequence gaps while ALAC decode and PCM generation remain active. v0.1.34 keeps up to 16 pending resend ranges so the 50 ms request interval cannot discard a gap, and tracks 16 outstanding ranges for one second so delayed responses remain eligible for insertion. FLUSH, TEARDOWN, and disconnect clear pending and outstanding state. Packet/decode checkpoints are reduced to every 512 packets; five-second totals report sequence gaps and resend outcomes. The validated finalized v0.1.34 PKG is 3,236,016 bytes, SHA-256 `8828e032fea57ab1ce4e0041f0c802c45940f3c7abe93caf3d9969fa9f01e80f`; staged on hardware but not installed. Source inspection then found PCM underflow telemetry included intentional silence during prebuffer. v0.1.35 / SFO 00.35 retains the RTP recovery changes and only counts underflows after decoder/prebuffer readiness. Lima host/sanitizer suite, PPU application build, ELF/SFO and finalized package validation pass. ELF: 2,780,888 bytes, SHA-256 `435c6f3e37832376f92d0d8fccc5c088880d0fc20f503235344e314d9990a497`. PKG: 3,236,016 bytes, SHA-256 `aba1d95124d85ff84c5d2442235968ad00b25e24871a3875f90dd7080ab74c9b`; staged and byte-verified but not installed. Wait for the current title to exit before hardware testing.

## v0.2.3 controller START fallback build and deployment

- Source commit `5a04d93459611df730cc0b0f2d6f2afe84903b86` connects the normalized START mask to the existing clean-exit action. App marker `0.2.3`; SFO `00.43`.
- Existing Lima guest: 11 GiB free before build. Full `scripts/build-lima-crypto.sh` host/ASan/UBSan suite passed, including RTSP/RTP loopback, decrypt/ALAC/PCM, mutation smoke, PCM quality and gain checks, and PPU component objects.
- `scripts/build-lima-standalone-receiver.sh` produced a 2,754,776-byte CellOS PPC64 ELF, SHA-256 `59f42ec05dc233c7450abfd2303f4fb929caf835dbc0d582e8164448b53b20fd`, with PS3DK v0.27.0 PPU 16.1.0, GNU ld 2.42, and PSL1GHT `dd4e8c9be744e3273d665c393fa6d97a87402e2e`.
- `scripts/package-lima-standalone-receiver.sh` produced a type-1 debug package with five items, AIRPLAY02 identity, matching content ID, SFO APP_VER/VERSION `00.43`, and validated extraction. PKG size 3,232,112 bytes; SHA-256 `756ac28147e98a37222279c607208639f157ddd4395bd2f4dfad8816ff4581b4`.
- The initial package invocation stopped because `PS3DEV` was not exported into the new shell; the second invocation then required the generated background asset. We reused the existing 2,764,800-byte generated RGB asset only after confirming the JPEG source SHA matched this checkout. The completed package passed all validators.
- FTP upload to a new versioned path and download-back comparison passed. webMAN installed while retaining the staged PKG. Installed SFO validates as 00.43; installed 451,632-byte EBOOT matches the package SELF exactly.
- A webMAN `play.ps3?AIRPLAY02` request returned HTTP 200 but the console remained at XMB, and no new `APP_START` appeared. Launch and START-first hardware validation remain pending. Do not infer controller behavior from package installation.

## v0.2.6 / SFO 00.46

Exact source `bca0f94901acdd7eb97f3b824e372f53beee7ee3`; full release gate PASS, including payload/recovery and mocked production display tests. PKG 3,233,072 bytes, SHA-256 `472915766a1b950f44558259a3583b5931c0bb3f8d3a8b0e5f1a044715927ca6`. Built locally; no deployment. See `docs/reports/2026-11-07_v026_payload_resume.md` for acceptance boundary.
