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
- The standalone screen candidate uses PSL1GHT RSX framebuffers, primes scanout using the PSL1GHT `videoTest` sample's spare-buffer-first pattern, and overlays a bright `DISPLAY ACTIVE` banner and controller START exit prompt over the public maintainer-provided background. This makes black/no-signal distinguishable from the intentionally dark image; visible output and orderly XMB return remain unconfirmed on hardware.
- The standalone app begins with a bounded five-second 440 Hz diagnostic rendered through the same GameOS audio blocks as receiver PCM; host waveform checks do not establish audible hardware output.
- Latest hardware observation: the user reports a five-second tone, a black screen, and iPhone discovery of `ps3-airplay-diagnostic` followed by a failed connection. A 350-byte log snapshot contains `SCREEN_READY`, `CONTROLLER_READY`, `MDNS_READY`, and `AUDIO_TEST_TONE_START` but no completion, RTSP, or stop event; its package version is unconfirmed, and FTP later timed out. Do not infer that visible pixels reached the display or that AirPlay connected.
- Previous verified build: version 01.01 was staged with an exact downloaded-back SHA-256 match; the later black-screen run's package identity is unknown. The prior staged package was preserved locally before replacement.
- Version 0.1.2 builds with the high-visibility display overlay, verbose RAOP logging, common response headers, PAUSE/resume transitions, and NetCtl-derived IPv4 service advertisement. The package extracted with APP_VER 0.1.2, AIRPLAY02, a NPDRM EBOOT, icon, and exact 2,764,800-byte RGB background.
- Initial review on 2026-11-07 reconfirmed the v0.1.2 candidate is 3,225,712 bytes with SHA-256 `923aebf6eca5693b21b494a9a4225c36629f13e8b316b2825d5deb3763226ebf`; package structure and embedded assets match the extraction. Following user approval, it was staged and later installed from XMB. The installed SFO and EBOOT hashes match the extracted candidate; XMB launch now fails with user-reported error `80028F21`. The cause is unknown.
- A separate v0.1.3 candidate uses PSL1GHT commit `6e565a70e927f55813babe86dc6f64149535f908` (2020-11-25) and a matching GCC 7.2 build of Mbed TLS. Its 3,183,408-byte AIRPLAY02 package has SHA-256 `98d8701ab018df882e30ba28f851a994c195d0a144dafe7c68c77d86d94126aa`; runtime compatibility remains unconfirmed.
- After explicit approval, v0.1.3 was uploaded to `/dev_hdd0/packages/ps3-airplay-receiver-0.1.3.pkg` and installed from XMB. Read-only retrieval confirms installed APP_VER 0.1.3 and exact SFO/EBOOT matches to the extracted package. The user reports the same `80028F21` launch error as v0.1.2. The 1,817-byte diagnostic log is unchanged from the previous run; no fresh startup event was written. v0.1.2 remains staged for rollback.
- The existing Lima VM is running Ubuntu x86_64 under emulation with the PS3DK v0.27.0 source/toolchain staging and host PKG tools present. The current `scripts/build-lima-smoke.sh` verifies portable receiver objects and a native PS3DK PRX sample, not a native standalone app or its XMB launch. Build a separate minimal native standalone launch control before claiming that alternative has been tested. The Thermal Bench project documents a Sony SDK 4.75 / Windows 7 VM / MSBuild workflow; it requires a properly licensed SDK and a separate port.
- Added `prototype/native-ps3dk-launch-control`, a separate PS3DK-native app using the cellGcm sample pattern, current video mode, early controller setup, bounded RSX polling, a 20-second automatic exit, START exit, and a fresh local diagnostic log. It uses title ID `AIRTEST01` and APP_VER 0.1.0; it is independent of AIRPLAY02. The local 192,688-byte PKG built and passed local package-list/SFO/content-ID and extracted-file checks; SHA-256 `7bb8e2fbc12cc3da8f4b91ff0ae85c4dcb105209dba5d45765b1787d4015b6d2`.
- After separate user approval, uploaded the native-control PKG as a new staged file at `/dev_hdd0/packages/ps3-airplay-native-launch-control-0.1.0.pkg`. Download-back was byte-for-byte equal at 192,688 bytes with the expected SHA-256. It has not been installed or launched; AIRPLAY02 remains untouched.
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

## Verified boundary

Host tests cover portable protocol, crypto, codec, PCM, and discovery components. Cross-build and package/container checks have been performed. PS3 hardware has played the diagnostic tone and advertised the receiver per user report; video rendering, iPhone interoperability, normal music playback, and game audio coexistence remain unverified. Game mixing remains a separate feasibility gate.

Demonstration target: PS3 with Evilnat 4.93 CFW and Cobra 8.5 observed; the console model was not recorded. Hardware compatibility remains unverified.

## Invariants

- Keep protocol inputs, sessions, queues, and logs bounded.
- Do not log track metadata, credentials, cryptographic material, or peer addresses.
- Advertise only implemented capabilities and label diagnostic-only behavior accurately.
- Distinguish host tests and format checks from hardware behavior.
- Keep boot-time plugin configuration unchanged during build work.
- Preserve third-party notices and license obligations.
