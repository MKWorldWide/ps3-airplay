# Test matrix

## v1.0.0 release gate preparation (2026-11-08)

| Test | Result | Boundary |
|---|---|---|
| App/SFO version source | PASS: `release.json` reports app `1.0.0`, SFO `01.00`; regression test accepts next `1.0.1` / `01.01` and rejects malformed semantic versions. | Configuration regression only; the separately listed PKG test validates actual extracted SFO fields. |
| Release evidence verifier | PASS: synthetic debug-PKG header accepted when source/hash/check statuses match; mismatched hash and pending validation status were rejected. | Validates metadata and package envelope, not a real release package. |
| v0.2.8 Gate 0 hardware acceptance | PASS, user accepted: about 45 minutes, four XMB cycles, no reported PCM underflows, ALAC failures, RTP queue drops, concealed/skipped frames, or screen-presentation failures. | User-supplied summary; ~45 ms maximum service interval on XMB resume retained as a known hitch. No fresh raw-log audit in this pass. |
| Historical standalone launch, tone, and START exit | PASS for tested build label `0.30`: maintainer reported launch, the five-second tone, and START returning to XMB while idle; tone source caps peak amplitude at 5%. | Historical build, not v1.0.0 validation. The same report says controls stopped responding during the attempted AirPlay session; it does not prove streamed playback. |
| v1.0.0 exact-source host/sanitizer and PPU build | PASS: full release script completed from former commit `f3da352809f81daac5e3b61a35d7fc18362bf1cc`; its identical source tree is now `a1edbba176f31076eccd10d2c627916963cb5605`. | Cross-build and host tests only; evidence names the pre-rewrite SHA. Rebuild from exact current main tip for release. |
| v1.0.0 PKG and extracted SFO | PASS: 3,906,640-byte debug PKG, SHA-256 `ef8f0afdacef560e9f5825fa9539247f3eb17e7791282aa6790f086fa8b1288e`; SFO validator confirms AIRPLAY02, APP_VER/VERSION `01.00`, XMB title AIR PS3. | Candidate predates attribution rewrite and is not a main-tip rebuild. No PS3 was modified during build validation. |
| v1.0.0 development-unit transfer and install | PASS: FTP download-back matched the PKG SHA-256; installed PARAM.SFO and EBOOT matched the candidate payload byte-for-byte. | The install step did not launch the app; package identity only. Primary PS3 untouched. |
| v1.0.0 current hardware report | PASS, user-reported observation: maintainer says v1.0.0 is running beautifully on the development PS3. | Qualitative report only; no duration, log measurements, connection details, or separate START/PS-menu results were provided. This is not a measured soak or protocol test. |
| GitHub tag/release | BLOCKED | Source integration into current main and a fresh exact-main-tip build remain required. No tag or release created. |

## v0.2.0 candidate work (2026-11-07)

| Test | Result | Boundary |
|---|---|---|
| Startup tone absent from production renderer and link list | PASS by source/link inspection; receiver ELF built without the tone object or startup/completion events. The standalone tone component remains host-tested. | No current PS3/RPCS3 runtime check. |
| Production PCM ring 44.1 kHz to 48 kHz quality baseline | PASS structurally: six frequencies, 48,000 output frames each, zero underflows, zero dropped frames. Measured SNR: 92.30 dB (100 Hz), 54.64 dB (1 kHz), 26.78 dB (5 kHz), 15.03 dB (10 kHz), 8.47 dB (15 kHz), 4.17 dB (20 kHz). | Host synthetic tones only; indicates linear interpolation attenuation at high frequencies and does not isolate physical dropout causes. |
| App/package version and SFO version | PASS: AIRPLAY02, app marker `0.2.0`, SFO APP_VER `00.40`; finalized PKG validated. | Candidate only, not a stable 0.2.0 milestone claim. |
| Finalized package and extraction | PASS: 3,231,472 bytes; SHA-256 `924a01ef0ef812e1badb8e94011930f5187c4dba91b253e578e16ab72de4f29b`; five package entries, extracted EBOOT/SFO/icon/background checks passed. | Build and container validation only. |
| PS3 runtime | v0.2.0 installed and launched; user streamed a song and logs reached PCM submission. | User reports controller input failed; details below. No display or audible-quality confirmation. |

## v0.2.0 PS3 run (2026-11-07 local time)

| Test | Result | Boundary |
|---|---|---|
| Stage/install/launch exact package | PASS: FTP upload/download-back hash matched; webMAN installed AIRPLAY02 SFO 00.40, and installed EBOOT's first 450,944 bytes match the validated package EBOOT. | Installed file had 48 additional zero bytes; package staged copy and v0.1.39 rollback package remain. |
| Startup identity and tone removal | PASS: fresh log records APP_START and PKG_VERSION_0.2.0; tone start/completion events absent. | Log is capped and wrapped; original snapshots kept local and ignored. |
| Display and controller | Controller FAIL by user report: neither START nor PS button worked. | `PAD_HEALTH` only showed successful polling and pad 0 presence; no button mask existed. Display visibility was not confirmed. |
| AirPlay/decode/PCM | PASS to code-path boundary: RTSP RECORD, 23,040 successful decrypt count, ALAC frame decode, zero sampled decode failures, PCM submission. | User streamed a song; no explicit subjective quality confirmation. RTP gaps/concealments were logged; no PCM underflow/drop events in saved interval. |
| Clean recovery | PASS: webMAN exit logged SYSUTIL_EXIT_GAME, cleanup completion, and APP_STOP; status returned to XMB. | The webMAN command, not controller input, exited the app. |

## v0.2.1 controller-diagnostics source validation

| Test | Result | Boundary |
|---|---|---|
| Standalone PPU compile/link with changed poller | PASS with `-Wall -Wextra -Werror`. | Package not yet generated. |
| Full Lima host/sanitizer and PPU component suite | PASS, including RTSP/RTP integration, encrypted RTP-to-ALAC/PCM, ALAC mutation checks, PCM quality/gain, and PPU object checks. | Does not exercise physical controller input. |
| v0.2.1 finalized package | PASS: 3,231,920 bytes, SHA-256 `f45b2935213dc1cfa62cfc70f958fc7a240925979fbe7aa779822a1a4a8a2c50`; SFO 00.41; extracted payload and envelope validated. | Host/package boundary only. |
| v0.2.1 install and launch | PASS: installed SFO 00.41; EBOOT package payload prefix matched exactly with 48 zero padding bytes; webMAN launch produced fresh app-version log. | Historical run; no streamed audio in that run. |
| v0.2.1 input retest | INCONSISTENT: user reports START exited once, then START/PS stopped responding after relaunch. Latest log has no button transition or START exit; a PS menu-open callback occurs without a close callback. | Callback proves event delivery only; no visible menu confirmation. |
| v0.2.2 PPU build and full host suite | PASS: full Lima host/ASan/UBSan suite, PPU component objects, and standalone ELF validation. | Cross-build does not validate physical input. |
| v0.2.2 finalized package | PASS: 3,232,112 bytes, SHA-256 `44886cd6ce7bbeff8089bd2d5d752cce3b76ded01f36b214cfdaff057f6748cd`; AIRPLAY02 / SFO 00.42. | ELF, SFO, content ID, assets, extraction, SELF, and finalized package checks passed. |
| v0.2.2 install and launch | PASS: FTP download-back hash match; installed SFO exact; installed EBOOT equals package SELF plus 32 zero bytes; fresh `PKG_VERSION_0.2.2` log. | Installed and launched on PS3. |
| v0.2.2 controller retest: manual reboot/direct XMB launch | FAIL: user reports no buttons work, same as webMAN launch. Fresh log records `SYSUTIL_MENU_OPEN` but no close, no raw digital-mask transition, no START, and no app stop. | Manual launch does not distinguish or fix the issue. Callback delivery is not proof that the menu appeared. Console left under user control; isolate START-first then PS behavior on a fresh launch. |

## v0.2.3 controller START fallback candidate

| Test | Result | Boundary |
|---|---|---|
| START action uses normalized digital state | PASS: action condition consumes the same normalized bit that includes the raw `PAD_CTRL_START` fallback. Full Lima host/sanitizer and PPU builds passed. | Package v0.2.3 is installed; hardware test pending because remote launch returned to XMB without APP_START. |
| v0.2.3 package/deployment identity | PASS: 3,232,112-byte type-1 debug package, SHA-256 `756ac28147e98a37222279c607208639f157ddd4395bd2f4dfad8816ff4581b4`; FTP download-back matched. Installed SFO 00.43 and 451,632-byte EBOOT match. | Installation confirmed; launch and controller runtime not confirmed. |
| PS-button visible system menu | FAIL by user report on v0.2.2: PS pressed first, no menu appeared visibly. | v0.2.3 changes only the START action path and does not claim to fix PS-menu presentation. |

This matrix distinguishes portable host evidence and cross-build evidence from real hardware validation. Update rows only when the listed scenario is actually exercised.

## AIR PS3 v0.2.8 candidate gate

| Test | Result | Boundary |
|---|---|---|
| RTP delay simulation (50 ms through 1 s) | PASS: queue holds packets without forced resynchronization | Host sequence model, not a real scheduler pause |
| RTP five-second backlog | PASS: two bounded resyncs, 512 stale queued frames counted, queue returns to 113 live frames, no obsolete concealment | Host queue test; audio heard on hardware not verified |
| Asynchronous logging | PENDING | PPU build and hardware writer scheduling still required |
| RTSP stage timing | PENDING | Source instrumentation requires a fresh runtime log |
| Supplied artwork source/derivatives | PASS: source pixels and direct-resize checks; XMB icon 320x176; PIC1 1920x1080 | No physical XMB visual check |
| v0.2.8 PPU ELF and package | PENDING | Exact committed source revision is built by the Lima release gate |
| XMB resume, iPhone discovery, streaming, and audio quality | NOT TESTED | No console installation or launch was performed in this package task |

**Demonstration target:** PS3 with Evilnat 4.93 CFW; Cobra 8.5 observed. Model not recorded. This configuration is context for future community testing and is not hardware compatibility evidence.

## Host and build checks

| Area | Evidence | Status |
|---|---|---|
| RTSP | Persistent bounded session, state transitions, malformed input, deadlines, loopback | Host tests pass |
| RTP | Header validation, peer filtering, sequence ordering, bounded queue, resend request/response validation | Full Lima ASan/UBSan suite now verifies the bounded gap-wait/recovery/silence path and caps large discontinuities at 128 concealed frames. RPCS3 v0.1.38 encrypted 128-packet fixture had zero gaps; emulator loss-recovery behavior remains host-tested, not emulator-tested. |
| Timing | Bounded timing probe, matching response and RTT diagnostics | Latest uninstrumented full-suite loopback passed timing request/reply checks after one earlier intermittent stall. Sender-clock playback mapping is not implemented. |
| Crypto | Base64, AES, RSA session-key recovery and challenge helpers | Host vector tests pass; real sender interoperability untested |
| SDP/ALAC | Bounded format parsing and reference packet decoding | Host tests match reference samples; encrypted RTP-to-PCM integration passes; ASan/UBSan verifies negotiated sample-count bounds and 256 deterministic malformed inputs with halt-on-error |
| PCM ring | Conversion, resampling, overflow and underflow bounds | Host tests pass |
| Audio diagnostic | Bounded 440 Hz stereo tone, five-second duration, conservative peak and fades | Host unit test; user reports hearing tone once on PS3; not independently verified |
| Discovery | RAOP response serialization and capability consistency | Host wire-format test covers the advertised TXT records, including RAOP server version 105.1; PS3 LAN/iPhone discovery for this build untested |
| Diagnostics | Allowlisted RTSP methods/status/state, SDP numeric format, socket failures/readiness, RTP batch events; log cap and rollover | Host log redaction/cap/rollover checks pass; verbose receiver events cross-build; PS3 log retrieval for 0.1.2 not run |
| PPU/PRX | Cross-build and PRX structural checks | Build checks pass; console compatibility untested |
| Known-good ELF ABI | Parse preserved v01.01 decrypted ELF and modern PPU output; reject legacy v0.1.6 ELF layout | PASS: reference and PS3DK candidate both have CellOS `0x66`, PPC64 `ET_EXEC`, entry `0x10000000`, and reference PT_LOAD flags; legacy v0.1.6 has generic `0x5`/`0x6` flags and is rejected. The layout difference is proven; its causal relationship to RPCS3's loader error is still a hypothesis. |
| Runtime compatibility | Compare PSL1GHT runtime paths against hardware evidence | v0.1.2 (newer runtime) and v0.1.3 (PSL1GHT commit `6e565a70e927f55813babe86dc6f64149535f908`, 2020-11-25) both returned user-reported XMB `80028F21`. AIRTEST01 PS3DK-native app returned the same code. This does not isolate the cause. |
| Standalone package | Package inspection, extraction, and installed-file identity | v0.1.2: 3,225,712 bytes, SHA-256 `923aebf6eca5693b21b494a9a4225c36629f13e8b316b2825d5deb3763226ebf`. v0.1.3: 3,183,408 bytes, SHA-256 `98d8701ab018df882e30ba28f851a994c195d0a144dafe7c68c77d86d94126aa`. Original v0.1.4: 3,183,584 bytes, SHA-256 `0c1e3c74eea71f7b5829201673f4dd5211bcbf704ab00cb55e1b41cb87020ee4`; its package paths were rejected. Corrected-path v0.1.4 and v0.1.5/v0.1.6 candidates install in RPCS3 but fail at EBOOT loading; details below. |
| RPCS3 package launch | Install and boot AIRPLAY02, then reach app logging | v0.1.4 original: package parser rejected absolute entry paths. Corrected-path v0.1.4 (SHA-256 `7811cdb221765aaa9e6ac3e57bd875cb8b7409045e6f371ff7a2a7a2d0744a8f`) installed but failed on unsupported EBOOT format. v0.1.5 installed but failed `Invalid ELF OS ABI`. v0.1.6 installed after setting OS/ABI `0x66`, but failed `Invalid ELF type` before app startup. No app log or audio in these v0.1.4-v0.1.6 RPCS3 runs. |
| Native PS3DK launch control | Cross-build and package a minimal cellGcm/controller standalone app using a unique title ID | Built and staged locally as AIRTEST01 v0.1.0; read-only FTP later found matching installed SFO/EBOOT. User reports XMB `80028F21`; its fresh log was unavailable. Separate from AIRPLAY02 and parked. |
| Resident PCM output | Plugin audio output | Not implemented; candidate discards decoded PCM |

## Current emulator validation (2026-11-07)

| ID | Scenario | Result |
|---|---|---|
| RPCS3-01 | Install and launch v0.1.38 / SFO 00.38 from finalized PKG | PASS; installed EBOOT/SFO hashes match the validated extracted package. |
| RPCS3-02 | Image and startup tone | PASS in emulator log/screenshot: 1280x720 screen initialized, background visible, 440 Hz tone completed. RPCS3 output does not validate physical HDMI/flicker. |
| RPCS3-03 | Encrypted RAOP OPTIONS/ANNOUNCE/SETUP/RECORD, RTP, decrypt, ALAC, PCM submit | PASS for the bounded 128-packet fixture. Log: key unwrap complete; 44.1 kHz stereo; RTP decrypt success; ALAC frame; 61,440 output frames submitted. Zero sequence gaps, zero recovery, zero concealment, zero ALAC errors in this fixture. No acoustic confirmation. |
| RPCS3-04 | Keyboard START to clean app stop | PASS; USER_EXIT_START, CLEANUP_BEGIN, AUDIO_CLOSE_DONE, RECEIVER_CLOSE_DONE, APP_STOP. |
| RPCS3-05 | mDNS advertisement in RPCS3 | FAIL/unsupported boundary; logs report mDNS interface setup error 125. No iPhone discovery test in this run. |
| PS3-01 | Install and launch production v0.1.39 / SFO 00.39 | PASS by FTP/webMAN; installed SFO verified and webMAN reported AIRPLAY02 00.39 running. |
| PS3-02 | Startup readiness and diagnostic tone | Startup, screen initialization, controller, network, RTSP/RTP listeners, mDNS, audio, tone-start and tone-complete events present in fresh log. User-visible pixels and audible tone are NOT CONFIRMED. |
| PS3-03 | Same-LAN discovery and RTSP session | DNS-SD browse found a RAOP service. v0.1.39 log later reached recording state with encrypted RTP, confirming a session/data path; targeted DNS-SD resolve remains inconclusive. Earlier standalone OPTIONS probe EOF discrepancy remains separate/unresolved. |
| PS3-04 | RTP, decryption, ALAC and PCM submission | PASS to GameOS PCM block submission in the retained v0.1.39 log. User described AirPlay as working. Acoustic quality/Spotify identity not independently measured in this run. |
| PS3-05 | Remote exit, controller input and XMB status | webMAN remote exit PASS: SYSUTIL_EXIT_GAME, all resource cleanup, APP_STOP; webMAN status then showed XMB. User reports controller/PS input unresponsive. PAD_HEALTH confirms successful polling and port presence, but button transitions are not logged. Display/resume/relaunch remain NOT TESTED. |

## v0.1.39 build validation (2026-11-07)

| ID | Scenario | Result |
|---|---|---|
| BUILD-39-01 | Build production port-5000 CellOS PPC64 ELF from pushed commit `1a5520d` | PASS; 2,780,936 bytes; ELF validator confirms CellOS PPC64 executable with five PT_LOAD segments. |
| BUILD-39-02 | Finalize production AIRPLAY02 PKG, SFO 00.39 | PASS; 3,236,976 bytes; SHA-256 `9f67340331eaae9e5ecbe94adaee03a0201e79e652a6bf45fa14cf6922877840`. |
| BUILD-39-03 | Finalize RPCS3-only port-5001 PKG, SFO 00.39 | PASS; 3,236,976 bytes; SHA-256 `70ada4d72860f43680a7c81d331a10a71b0b5603a2471a2219493f66f7e271f5`. |
| BUILD-39-04 | Install/replay v0.1.39 in RPCS3 | PENDING; CUA timed out opening package install and then could not read RPCS3 AX state. Latest fresh runtime log remains v0.1.38; do not attribute its events to v0.1.39. |

## PS3 and iPhone gates

| ID | Scenario | Status |
|---|---|---|
| DEPLOY-01 | FTP preflight, package staging, and downloaded-back hash verification | PASS. v0.1.2 is staged at `/dev_hdd0/packages/ps3-airplay-receiver.pkg`, 3,225,712 bytes; downloaded-back SHA-256 matches `923aebf6eca5693b21b494a9a4225c36629f13e8b316b2825d5deb3763226ebf`. Previous 01.01 PKG backup is verified locally. |
| DEPLOY-02 | Replace staged package with corrected NPDRM PKG and verify downloaded-back hash | PASS; 416,240 bytes, exact SHA-256 match |
| DEPLOY-03 | Replace staged package with screen-enabled 01.01 candidate and verify downloaded-back hash | PASS; 3,222,640 bytes, byte-identical to approved artifact |
| DEPLOY-04 | Install v0.1.2 and verify installed app identity against candidate | PASS: SFO reports AIRPLAY02 / 0.1.2; installed EBOOT (445,232 bytes) and SFO match the extracted v0.1.2 files by SHA-256 |
| DEPLOY-05 | Stage/install the v0.1.3 legacy-runtime candidate and verify launch | Upload PASS: versioned package staged at `/dev_hdd0/packages/ps3-airplay-receiver-0.1.3.pkg`; download-back matched size 3,183,408 bytes, SHA-256 `98d8701ab018df882e30ba28f851a994c195d0a144dafe7c68c77d86d94126aa`, byte-for-byte. Install PASS: SFO and extracted-package EBOOT match. Launch FAIL: user reports the same `80028F21`. Staged v0.1.2 remains available for rollback. |
| DEPLOY-06 | Build isolated native PS3DK launch-control PKG | PASS locally: APP_VER 0.1.0, AIRTEST01, content ID `UP0001-AIRTEST01_00-0000000000000000`, 192,688 bytes, SHA-256 `7bb8e2fbc12cc3da8f4b91ff0ae85c4dcb105209dba5d45765b1787d4015b6d2`. |
| DEPLOY-07 | Upload approved native launch-control PKG as a new staged file and verify downloaded-back bytes | PASS: versioned AIRTEST01 PKG was staged and byte-verified. Later read-only retrieval shows AIRTEST01 is installed; user reports launch error `80028F21`. It is separate from AIRPLAY02. |
| BUILD-04 | Build AIRPLAY02 with 2020 PSL1GHT, route-first network address selection, bounded NetCtl fallback, and version-matched diagnostics | PASS locally: ELF64 big-endian PowerPC64; v0.1.4 PKG 3,183,584 bytes, SHA-256 `0c1e3c74eea71f7b5829201673f4dd5211bcbf704ab00cb55e1b41cb87020ee4`; SFO reports AIRPLAY02/0.1.4 and package extraction matches assets. No hardware validation. |
| BUILD-05 | Validate relative PKG paths and launch v0.1.5 in RPCS3 | Package installation PASS; launch FAIL before app startup with `Invalid ELF OS ABI`. Artifact SHA-256 `dfe2411ce0079d09fd84e7169865fad299dcb5f2759c2b8e0876fb2b7e5a8449`. |
| BUILD-06 | Patch PPU ELF OS/ABI to CellOS LV2 and launch v0.1.6 in RPCS3 | ELF header validation PASS (`EI_OSABI=0x66`); package install PASS; launch FAIL before app startup with invalid ELF type. Artifact SHA-256 `c71cf7ee0ec314496971ea32d831c05238746eada6796b017092b3873a2ecc33`. No in-app log, display, tone, or AirPlay test. |
| BUILD-07 | Build from committed PS3DK/Lima pipeline; assert known-good ELF layout; package, install, and launch in RPCS3 | ELF validation PASS; finalized v0.1.7 package installed in RPCS3; UI lists AIRPLAY02 0.1.7 and launch reached APP_START. Fresh log then shows RTSP_BIND_FAIL value 112, RECEIVER_OPEN_FAILED, APP_STOP. The RPCS3 network status was Disconnected; causal link is unproven. No display/tone/AirPlay result. Package size 3,225,904 bytes; SHA-256 `fc8899dd3f62cbd9678bf3f27bdb542770fe9516ad142d37235ea3f958e09674`. |
| BUILD-08 | Validate pre-finalization contents and finalized envelope; install and launch v0.1.8 in RPCS3 | Package extraction, APP_VER, content ID, assets, ELF, and finalized envelope validation PASS. RPCS3 installed v0.1.8 and fresh log reached APP_START, then RTSP_BIND_FAIL value 112, RECEIVER_OPEN_FAILED, APP_STOP. Network status observed Disconnected; causality unproven. No tone/display/AirPlay result. PKG: 3,225,904 bytes; SHA-256 `d614a27d38abc4e3ab1387b7ed3ad812aab461a167d781a80e87ea881d1c1f83`. |
| BUILD-09 | Move bounded route/NetCtl readiness before socket creation; build and test v0.1.9 | Lima host tests, ELF validation, package extraction, SFO and finalized envelope PASS. RPCS3 with network Disconnected logged NET_IP_TIMEOUT after the bounded five-second wait. With network Connected, it logged NET_IP_ROUTE_READY, then RTSP_BIND_FAIL value 112 and APP_STOP. A wildcard host listener occupied TCP 5000; causation is still unproven. |
| BUILD-10 | Build v0.1.10 with configurable RTSP port; test 5001 in RPCS3 | Host suite, ELF/package validation, SFO/content ID/assets PASS. RPCS3 installed/launched v0.1.10, then logged RTSP_LISTEN_READY=5001 and RTP_AUDIO/CONTROL/TIMING_READY=6000/6001/6002. MDNS_FAILED followed; audio initialized and tone completed. No AirPlay discovery/session. PKG: 3,225,904 bytes; SHA-256 `223ed38d55b680e185c8925ef311337de6ff01f87d300ad02ed3a44dde54de40`. |
| BUILD-11 | Remove blue diagnostic bands and DISPLAY ACTIVE text; build v0.1.11 | Lima host tests, ELF/package/SFO/envelope validation PASS. RPCS3 screenshot visibly shows only the background and white START-to-exit prompt. No physical display claim. |
| BUILD-12 | Add privacy-safe mDNS startup failure stage and errno; build v0.1.12 | Host suite and argument failure assertion PASS. ELF and finalized PKG validation PASS. RPCS3 reached RTSP 5001 and RTP 6000–6002, then logged `MDNS_FAILED value=4 bytes=112`; stage 4 is UDP bind, and PS3DK errno 112 is `EADDRINUSE`. Repeated with Lima stopped and RPCS3 networking Connected; same failure. The host system mDNS responder held wildcard UDP 5353; host bind probe also failed. No mDNS session or AirPlay audio. |
| BUILD-13 | Rebuild committed mDNS diagnostics as v0.1.13 on the canonical PS3DK/Lima pipeline | PASS: full Lima host suite, PPU object build, CellOS PPC64 ELF validation, NPDRM package extraction, SFO/content ID, assets, and finalized envelope. PKG 3,224,944 bytes; SHA-256 `f7d076e32520a552349ebb973894409d66fd429ff255d0919c66401fabe9f1e9`. RPCS3 install/launch and native PS3 test pending; no discovery or playback claim. |
| BUILD-14 | Try best-effort SO_REUSEPORT to share UDP 5353 with the host responder | Host socket probe, mDNS serializer, PPU/ELF and finalized v0.1.14 package validation PASS. RPCS3 installed/launch PASS; log reached SCREEN_READY/CONTROLLER_READY then `NET_IP_TIMEOUT` and `APP_STOP result=1` before crypto/listeners. SO_REUSEPORT discovery behavior was not exercised. Native PS3 not tested. |
| BUILD-15 | Invalidate buffered RTP and PCM after successful RTSP FLUSH | Full Lima host suite and PPU app build PASS. AIRPLAY02 v0.1.15 (source `99abc5b`) validated: 3,225,648 bytes, SHA-256 `230a2df0db8a7e561f3b048872c1a714ed0eb58f4b741587ee961f71834ffa59`; ELF, SFO, content ID, assets, extraction, and finalized envelope PASS. RTP queue/socket flush, zero-length UDP consumption, sequence epoch reset, PCM/resampler clear, metrics preservation, and idle-session rejection have host coverage. RPCS3 installed/booted, but route timeout prevented any RTSP FLUSH runtime test. |
| BUILD-16 | Apply RAOP volume and mute to PCM output | PASS: RTSP parser tests verify dB attenuation, `-144 dB` mute, lower/upper bounds, malformed/duplicate input rejection, and session reset. PCM gain tests verify bounded amplitude, block ramp, and mute. Full Lima host suite, PPU objects, standalone ELF and finalized NPDRM PKG checks pass. PKG 3,231,088 bytes; SHA-256 `3c69bff7ab626c433728ca131bc04ab0adbc117b8845d88c748e1dad1968bd3c`. RPCS3 UI install/launch and physical audio tests remain pending. |
| DISPLAY-17 | Render the background without diagnostic overlays | PASS for source and finalized v0.1.17 package; RPCS3 visual output and controller exit remain pending. The package has not been launched, so do not infer display behavior from its structure. |
| BUILD-17 | Build AIRPLAY02 v0.1.17 after removing all display text overlays | PASS: full Lima host suite, PPU objects, CellOS PPC64 ELF validator, package extraction/SFO/assets/content ID, and finalized NPDRM envelope. PKG 3,230,256 bytes; SHA-256 `2095046e20716d9f2768c847691515f1d1e922e5fbd96e15829f92c45816cf23`. RPCS3 install/launch and visual check pending; no runtime/display behavior inferred. |
| OBS-18 | Periodically log PCM ring occupancy and frame flow | PASS: full Lima host suite, PPU application link, CellOS PPC64 ELF validator, and finalized AIRPLAY02 v0.1.18 package checks. PKG 3,230,448 bytes; SHA-256 `5548fde98bc08949e4f3a6739a95d5b3dce7dea0458131ec693c01be97766ae0`. Source records numeric ring occupancy, accepted PCM frames, resampler input frames consumed, and frames submitted into GameOS audio blocks every five seconds during an ALAC session. RPCS3 event verification pending; counters are not evidence of audible output. |
| CODEC-01 | Reject malformed ALAC counts/fields and preserve the valid encrypted decode path | PASS: full Lima host suite under ASan/UBSan with halt-on-error rejected 353 samples at a 352 limit and `UINT32_MAX`; 4,096 seeded mutations of a valid ALAC frame plus 256 random packets completed; valid encrypted RTP-to-PCM matched the reference. PPU objects and standalone v0.1.20 package built and validated. Runtime sender/hardware test pending. |
| BUILD-19 | Build and inspect AIRPLAY02 v0.1.19 from the committed ALAC bounds fix | PASS: source `7b451031487aab7dbad147ab0b47ef80a1e22265`; CellOS PPC64 ELF 2,780,120 bytes (SHA-256 `beb816b2aed2125a94584e4e820f18de66a98f21260ce47fe1654199322e7ec3`); finalized PKG 3,230,704 bytes (SHA-256 `41488b9683b901ecc4d71e113565fcdb10c1834c69e1642fbe774f5cdf4a31a5`). Full Lima host/PPU/build/package checks pass. RPCS3 install/launch and physical PS3 tests not run; desktop is locked. |
| BUILD-20 | Build and inspect AIRPLAY02 v0.1.20 with malformed ALAC hardening | PASS: source `8a4e641d114e1a613421b9ffb51f8615ad851827`; CellOS PPC64 ELF 2,780,120 bytes, SHA-256 `f50e6a4fe247149feb7759a69a81ae881c0972d789cb1a5dcfa2019bd202b3d9`; finalized PKG 3,230,384 bytes, SHA-256 `bfaa3f960de39daf95c2ab405bd361a031ed92bd35c76a99e667d4ffe8d19e87`. SFO, content ID, assets, extraction, EBOOT equality, and finalized NPDRM validation pass. RPCS3 UI reports Mac locked; no runtime or physical hardware test. |
| E2E-01 | Exercise RTSP TCP → encrypted RTP UDP → decrypt → ALAC → PCM ring | PASS in Lima host integration: real receiver sockets, fragmented ANNOUNCE, RSA-wrapped session key, AES-encrypted RTP, reference ALAC PCM, and stereo PCM ring accepted 353 frames. This is a sender simulation, not iPhone or GameOS output. |
| RPCS3-03 | Attempt direct v0.1.20 EBOOT launch with RPCS3 `--no-gui` while the UI is locked | INCONCLUSIVE: process remained alive until the 28-second stop boundary, wrote no stdout, and did not change the app log (still v0.1.15). No `APP_START` evidence; the candidate was not installed. |
| DISPLAY-18 | Reconcile the user-reported blue frame / `DISPLAY ACTIVE` label with the current display source | SOURCE PASS: `diagnostic_screen.c` copies only the background; commits `46fca40` and `4a06c2a` removed the blue diagnostics and all text. v0.1.20 descends from that source. USER-REPORTED DISPLAY PASS for app opening/background visible, but reported overlays do not match the source lineage. Exact observed package/platform remains unknown; current RPCS3 desktop still reports locked, so artifact identity is unresolved. |
| DISCOVERY-21 | Correct stale RAOP `vs` server-version TXT value and keep it consistent with RTSP `Server` | PASS: host wire-format assertion verifies `vs=105.1`; v0.1.21 ELF/PKG validation also passes. RPCS3 runtime remains pending. |
| BUILD-21 | Build AIRPLAY02 v0.1.21 with corrected RAOP server version | PASS: PPU ELF and finalized NPDRM PKG validated; extracted AIRPLAY02/APP_VER/content ID, EBOOT, icon/background, package paths, and envelope all match. Lima host/ASan/UBSan + PPU component checks PASS. RPCS3 install/launch pending (desktop locked); physical PS3 untouched. |
| LOGGING-22 | Distinguish successful audio-payload decryption from RTP reception and decode | PASS: composed encrypted RTP-to-PCM loopback asserts the receiver's success counter; logs emit `RTP_DECRYPT_OK` at the first and each 128th successful packet. Host/ASan/UBSan and PPU component checks pass. |
| BUILD-22 | Build AIRPLAY02 v0.1.22 with decrypt-stage diagnostics | PASS: PPU ELF and finalized NPDRM PKG validated; extracted AIRPLAY02/APP_VER/content ID, EBOOT, icon/background, relative paths, and envelope all match. Lima host/ASan/UBSan + PPU component checks PASS. RPCS3 install/launch is pending; physical PS3 untouched. |
| RTSP-WRITE-23 | Bound RTSP response writes so a slow client cannot indefinitely stall receiver polling | PASS: host test shrinks socket buffers, stops reading responses, verifies `RTSP_SEND_TIMEOUT`, and confirms disconnect within four seconds; full ASan/UBSan suite and PPU receiver compile pass. PS3/RPCS3 runtime remains pending. |
| BUILD-23 | Build AIRPLAY02 v0.1.23 with bounded RTSP response writes | PASS in Lima from commit `6513bdd201899e93fd55babd296fb97893d93216`: PPU CellOS PPC64 ELF (2,780,120 bytes, SHA-256 `2130c17ace5459a57765d3d2ae8b185012548789891961e3b74d330d6f236dd6`) and finalized AIRPLAY02 PKG (3,230,768 bytes, SHA-256 `d17f6d2576bb96d0ff29cd7a58790852b6607d9b60b5d2dcffbe5613e053c4c4`). Package envelope, title ID, APP_VER, content ID, EBOOT and background asset validated. Artifacts preserved in ignored `work/builds/0.1.23/`. RPCS3 install/launch remains pending because the Mac is locked; physical PS3 untouched. |
| RTP-HEALTH-24 | Log packet loss, ordering, duplicate/late packets, queue drops, payload delta, and queue depth | PASS: Lima RTSP/RTP socket integration crosses the 128-packet checkpoint, forces one post-FLUSH gap, and asserts `RTP_RX_BATCH`, `RTP_QUEUE_DEPTH`, and `RTP_MISSING_BATCH`; full ASan/UBSan suite and PPU receiver compile pass. |
| BUILD-24 | Build AIRPLAY02 v0.1.24 with bounded RTP health checkpoints | PASS in Lima from commit `a59aa937401d432b5cc6f740eeda15bf0db091ba`: PPU CellOS PPC64 ELF (2,780,120 bytes, SHA-256 `21b6ea799b18fd91f63b6d6dcb8d9f490e980f775c6bf3b96f84609b1dfdf9c2`) and finalized AIRPLAY02 PKG (3,231,152 bytes, SHA-256 `7638825e0c2f6eb26a19410ba56ffd52ed96a70dc274f691b6bf2d12cf615757`). Package envelope, title ID, APP_VER, content ID, EBOOT and background asset validated. Artifacts preserved in ignored `work/builds/0.1.24/`. RPCS3 install/launch remains pending because the Mac is locked; physical PS3 untouched. |
| RPCS3-01 | Verify v0.1.14 startup and diagnostics in emulator | PASS for package install and entry into app startup; PARTIAL screen initialization (log reports SCREEN_READY at 1280x720, image transient during PPU compilation); FAIL for sustained receiver startup because local route timed out. No RTSP/RTP listener, mDNS, tone, AirPlay session, or audio playback. |
| RPCS3-02 | Verify AIRPLAY02 v0.1.15 launch and fresh diagnostics | PASS: package install and boot. Fresh app log: APP_START, PKG_VERSION_0.1.15, SCREEN_READY, CONTROLLER_READY, NET_READY, NETCTL_READY, NET_IP_WAIT_START, NET_STATE_WAIT, NET_IP_TIMEOUT, APP_STOP result 1. RPCS3 reports `EADDRNOTAVAIL`; no crypto, listener, mDNS, tone, AirPlay, or FLUSH-over-network test. |
| BUILD-03 | Cross-build visible diagnostic screen and controller START exit; inspect packaged image | PASS in Lima for 0.1.2; hardware behavior untested |
| HW-01 | Confirm tested recovery procedure and record a privacy-safe baseline | Partial: console remained reachable for read-only checks after the launch error; clean in-app exit and recovery from a new freeze remain unproven. Standalone app does not alter boot plugins. |
| HW-02 | Install/launch standalone app and exit cleanly | v0.1.3 FAIL at launch: user reports the same XMB error `80028F21`. Installed SFO reports AIRPLAY02 / 0.1.3 and matches the extracted package; installed EBOOT also matches. The 1,817-byte log is unchanged, so no new startup event is available. Display, tone, controller, and clean exit are unverified for v0.1.3. |
| HW-03 | Audible test audio at the system menu | Prior partial PASS only: user heard the five-second diagnostic tone in a run whose package identity was not established. v0.1.2 tone test BLOCKED by launch failure; normal AirPlay music not tested. |
| HW-04 | iPhone discovers receiver and completes AirPlay session | Prior partial result only: iPhone listed `ps3-airplay-diagnostic`; selecting it failed to connect. No v0.1.2 connection attempt; session not established. |
| HW-05 | Verify audible AirPlay playback and controls | NOT RUN |
| HW-06 | Reconnect after sender/network interruption | NOT RUN |
| HW-07 | Launch AIRTEST01, verify visible colors, START exit/auto-exit, and fresh local log | FAIL at launch per user: XMB `80028F21`; no new diagnostic log retrieved. Parked. |
| HW-08 | Sustained standalone playback and resource stability | NOT RUN |
| PRX-01 | Load/start/stop/unload diagnostic module with tested recovery | NOT RUN |
| GAME-01 | Test audio coexistence in a specific title | NOT RUN |

## Reporting template

```text
TEST: <matrix ID>
BUILD: <revision or checksum>
DEVICE: <broad model/software version; omit unique identifiers>
SENDER: <device and OS version>
RESULT: PASS | FAIL | BLOCKED
DURATION: <if applicable>
OBSERVATIONS: <concise and reproducible>
LOG: <redacted attachment, if useful>
```

Review reports and logs for console/account identifiers, network addresses, Wi-Fi names, credentials, and track metadata before sharing.

## Latest RPCS3 run (2026-11-07)

| Test | Result | Evidence and boundary |
|---|---|---|
| v0.1.24 startup with RPCS3 Internet disconnected | PASS to bounded failure | `NET_IP_TIMEOUT`, then `APP_STOP`; no receiver sockets opened. |
| v0.1.24 startup with RPCS3 Internet connected | FAIL at RTSP bind | NetCtl route acquired; TCP 5000 bind failed because the host port was already occupied. |
| v0.1.25 startup with emulator port override | PASS | Validated AIRPLAY02 package payload, APP_VER 0.1.25, RTSP TCP 5001 and RTP UDP 6000–6002 ready. Five-second tone was logged complete; audibility is unverified. |
| mDNS inside RPCS3 | FAIL | Multicast-interface setup failed (stage 6, error 125). Discovery is therefore not established by this run. |
| RTSP OPTIONS loopback | PASS | Local client received `RTSP/1.0 200 OK`; matching log recorded accept, OPTIONS, status 200, and peer close. This is not an iPhone or complete RAOP session. |
| Display and controller exit | NOT VERIFIED | RPCS3 desktop was locked; `SCREEN_READY` is initialization evidence only. No visible-pixel or START/XMB result is claimed. |
| RTP audio, ALAC, PCM output, Spotify | NOT RUN | No ANNOUNCE/SETUP/RECORD sender session and no streamed audio were tested in RPCS3. Physical PS3 was not touched. |

## RPCS3 ANNOUNCE diagnostic follow-up (2026-11-07)

| Test | Result | Evidence and boundary |
|---|---|---|
| Synthetic RTSP ANNOUNCE | BLOCKED | v0.1.25 log records OPTIONS 200 and then `RTSP_ANNOUNCE`; the sender timed out waiting for the ANNOUNCE response after three seconds. No later status or error event appeared before the emulator was stopped. The exact blocked operation is not yet proven. |
| v0.1.26 phase diagnostics | HOST PASS / RPCS3 PARTIAL | Host assertions pass. RPCS3 confirms SDP parse start/completion and offer fields but receives no response. Event names over 24 characters were silently filtered, so RSA-start absence is inconclusive. Detailed replay is below. |
| Sender probe | READY | The host utility can perform OPTIONS, ANNOUNCE, SETUP, RECORD, send 128 encrypted RTP fixture packets to UDP 6000, then TEARDOWN. This is synthetic test traffic, not iPhone interoperability. |


## RPCS3 v0.1.26 ANNOUNCE replay (2026-11-07)

| Test | Result | Evidence and boundary |
|---|---|---|
| v0.1.26 package validation | PASS | AIRPLAY02 package is 3,231,408 bytes, SHA-256 `82e1407b92a1218ec3085f57bd8f6c35e97255569b92a44249e14239108fc246`; ELF, SFO, content ID, extraction, assets, and finalized NPDRM envelope validated. |
| RPCS3 startup and listeners | PASS | Fresh app log records APP_START, NetCtl route, RTSP 5001, and UDP 6000–6002 readiness. |
| OPTIONS | PASS | Synthetic sender received 200; the app recorded `RTSP_STATUS=200`. |
| ANNOUNCE SDP | PASS | App recorded `ANNOUNCE_SDP_PARSE_START` and `ANNOUNCE_SDP_PARSE_DONE value=1`. |
| ANNOUNCE response | BLOCKED | Sender timed out waiting for the response. No short validation-completion or RTSP status event appeared. Longer Content-Type, RSA, and response event names were rejected by the 24-character logger limit, so those absences are inconclusive. The app was stopped by the harness without APP_STOP. Exact blocked operation remains unknown. |
| SETUP/RECORD/RTP | NOT REACHED | Sender stopped at ANNOUNCE; no audio fixture packets were sent. |
| Display/audio/hardware | NOT VERIFIED | Desktop remained locked; this replay cannot establish visible pixels, audible tone, iPhone compatibility, or physical PS3 behavior. |


## RPCS3 v0.1.27 ANNOUNCE validation replay (2026-11-07)

| Test | Result | Evidence and boundary |
|---|---|---|
| v0.1.27 package/build | PASS | AIRPLAY02 PKG 3,231,920 bytes, SHA-256 `0a3d1c409fb1044231256090cc3693da3e1aaa5abddd5bc6e00f29a9aa407b6c`; ELF and finalized package checks pass. Full Lima sanitizer suite and PPU objects pass. |
| RPCS3 startup / listeners | PASS | Fresh log records APP_START, NetCtl route, RTSP 5001, and RTP 6000–6002. |
| OPTIONS | PASS | Synthetic sender received 200. |
| ANNOUNCE offer parse | PASS | SDP parse returned 1; offer fields reported encryption type 1, RSA key, and AES IV (encoded flags value 1539, key data length 344). |
| ANNOUNCE validation / response | BLOCKED | No Content-Type-check, RSA-unwrapping, validation-complete, or RTSP-status event followed the offer summary. Sender timed out; exact stalled operation remains unknown. |
| RTP / audio | NOT REACHED | Sender aborted before SETUP/RECORD; no media packets were sent. |
| Display / physical PS3 | NOT VERIFIED | Desktop remained locked; physical PS3 and boot plugin configuration were not changed. |


## RPCS3 v0.1.28 replay and logger diagnosis (2026-11-07)

| Test | Result | Evidence and boundary |
|---|---|---|
| v0.1.28 package validation | PASS | AIRPLAY02 PKG 3,231,984 bytes, SHA-256 `0e3201647bb83b85c0df4e08dacc780c9daf1468c9d261e321c10be0f8fa9cef`; ELF, SFO, content ID, assets, extraction, and NPDRM checks pass. |
| RPCS3 startup/listeners | PASS | Fresh app log records startup, route, RTSP 5001, and RTP 6000–6002. |
| OPTIONS | PASS | Synthetic client receives 200. |
| ANNOUNCE | BLOCKED | SDP parse, validation start, and offer fields log. The client gets no response; short validation-completion event is absent. Newly added Content-Type/RSA/response names exceed the logger’s 24-character limit and are silently dropped. Their absence is inconclusive. |
| SETUP/RECORD/RTP | NOT REACHED | Probe aborts at ANNOUNCE timeout. |
| v0.1.29 logger fix | HOST PASS / RPCS3 PENDING | Event-name limit raised and a 33-character event persistence assertion passes in the full Lima suite; package/runtime replay pending. |

## v0.30 compatibility candidate

Release label 0.30 maps to PS3 SFO 00.30. Packaging uses the preserved v01.01 type-1 debug container. Older three-component SFO values are now rejected. Production RTSP remains 5000; hardware launch/display/exit and actual AirPlay playback require fresh evidence. See the current decision log and memory for validation boundaries.

## Recovered PS3 run (historical app label 0.30; corrected label 0.1.30)

| Check | Result | Evidence |
|---|---|---|
| Console app launch | PASS | Maintainer observation and fresh versioned startup log |
| Diagnostic tone | PASS | Heard by maintainer; tone completion logged |
| START while idle | PASS (user observation) | Returned to XMB; separate idle-run log was not preserved |
| PS-button menu | FAIL (user observation) | Did not show XMB; callback stage not instrumented |
| Discovery / route selection | PASS (user observation) | iPhone sees/selects receiver; mDNS ready in log |
| TCP / basic OPTIONS | PASS | Accepted client; several OPTIONS 200 responses |
| Full RAOP session | FAIL / NOT ESTABLISHED | Final OPTIONS has no response; no ANNOUNCE/SETUP/RECORD |
| RTP / ALAC / streamed music | NOT ESTABLISHED / NO MUSIC | No packet/decode evidence and user heard no music |
| START during sender attempt | FAIL (user observation) | No USER_EXIT_START event; last log is OPTIONS, so cleanup not implicated yet |
| Shutdown | Abnormal observed | Three-second power hold, three beeps; cause unknown |

Recovered log hash and provenance are recorded in memory and the hardware follow-up report; raw logs stay private.

## v0.1.31 RSA fix

| Check | Result |
|---|---|
| Host challenge/key recovery/RTP decrypt | PASS under ASan/UBSan |
| RPCS3 Apple-Challenge | PASS: 200 and Apple-Response, 54 ms |
| RPCS3 encrypted ANNOUNCE | PASS: unwrap success, 200, 26 ms |
| RPCS3 SETUP / RECORD / TEARDOWN | PASS: all 200 |
| RPCS3 RTP / target decode | PARTIAL: packet reception/loss inferred from resend requests; no target prebuffer/decode confirmation |
| PS3 package install | PASS: installed SFO/EBOOT match validated package |
| PS3 audible music / controller exit during sender session | PENDING maintainer observation |

See the v0.1.31 report for precise host-versus-target evidence boundaries.

## v0.1.32 controls and byte order

| Check | Result |
|---|---|
| Old live-app recovery | Normal remote quit logs all cleanup completions and APP_STOP 0; no reboot |
| Big-endian PPU golden PCM | PASS in RPCS3: bytes match FFmpeg fixture (353 samples) |
| Host ALAC regression | PASS under ASan/UBSan |
| RPCS3 paced stream | PASS: 256 decrypted packets, 90,368 PCM frames queued and consumed |
| RPCS3 display/input polling | Active heartbeat/pad health; no flip failure during run |
| PS3 installation | PASS: installed SFO/EBOOT match validated 0.1.32 package |
| Real PS/held-PS menu, idle/stream/disconnect START | PENDING maintainer |
| Real music and crackling resolution | PENDING maintainer |

The emitter used a paced diagnostic fixture; target output submission is not proof of audible playback.


## Latest hardware result: v0.1.33

| ID | Scenario | Status |
|---|---|---|
| HW-033-01 | Launch, background, diagnostic tone and XMB exit | Launch/background and clean exit observed by maintainer; background flickers after XMB resume. One short tone was heard; v0.1.33 source contains one five-second 440 Hz tone only. |
| HW-033-02 | Real iPhone AirPlay music | PASS for audible music, multiple songs by maintainer report. Playback is crackly/unstable and low fidelity; this is not an MVP quality pass. |
| HW-033-03 | Receive/decrypt/decode/output pipeline | Log segment confirms encrypted 44.1 kHz stereo RTP, ALAC frame decode and PCM submissions. It shows at least 1,792 packets in the visible receive batches, 106 RTP sequence gaps, and zero ALAC failures in sampled totals. Log wrapped; initial RTSP/SDP context and exact whole-session counts are missing. |
| HW-033-04 | PS menu resume | FAIL/inconsistent: earlier resume stuck at a loading view; subsequent resume returned to app but the background flickered. |
| BUILD-034 | Bounded resend queue and delayed recovery | Host/Lima validation PASS: preserves gaps queued during the send throttle, tracks multiple outstanding ranges, and accepts delayed retransmission packets. Full PPU app/package and physical behavior pending. |

## Follow-up live stream evidence (2026-11-07)

| Test | Result |
|---|---|
| Installed build identity | PASS: FTP copies of installed PARAM.SFO and EBOOT.BIN both byte-match local v0.1.33; neither matches v0.1.34. |
| v0.1.34 staged artifact | PASS: 3,236,016-byte staged PKG byte-matches local package; install remains pending. |
| RTP continuity on v0.1.33 | DEGRADED: 1,664 received packets, 88 sequence gaps, and five resend-recovery events in the retained live snapshot. |
| Decrypt and ALAC | PASS in sampled interval: encrypted 44.1 kHz stream and zero ALAC failures. This does not prove every frame was rendered. |
| PCM/output continuity | INCONCLUSIVE on v0.1.33: user reports audible crackle, but code counted intentional startup prebuffer silence as underflow; corrected telemetry is in v0.1.35. |
| XMB resume display | FAIL/inconsistent by user report: resumed image is dim/flickering with frozen loading indicator; a later relaunch showed a solid background. |
| Spotify source-quality causality | UNKNOWN: RAOP's negotiated 44.1 kHz ALAC does not identify whether Spotify delivered lossless FLAC to its AirPlay sender. |
| Supplied acoustic recording | INCONCLUSIVE: 22.485-second mono 48 kHz sample has no >10 ms interval below -50 dBFS or large waveform discontinuity, but its -40.7 dBFS RMS level limits detection of subtle crackle. The private recording is not in the repository. |
| PCM intentional-silence accounting | HOST TEST PASS: helper fills a block with silence without changing underflow counters; an active empty ring still increments underflow. v0.1.35 PPU build passes; physical behavior pending. |
| AIRPLAY02 v0.1.35 package | PASS: 3,236,016-byte finalized PKG validates with AIRPLAY02 / APP_VER 00.35 and five items; remote staged bytes match local candidate. Not installed because v0.1.33 remains active. |


## v0.1.35 live capture and v0.1.36 source test (2026-11-07)

| Test | Result |
|---|---|
| Installed identity | SFO PASS: AIRPLAY02 / 00.35. Log wrapped before its startup version event; installed EBOOT hash captured but not byte-compared with a local extracted v0.1.35 payload. |
| Staged package | PASS: v0.1.35, 3,236,016 bytes, matching recorded SHA-256. |
| RTP receive | DEGRADED: 2,048 received packets, 302 sequence gaps, 209 late packets in retained log. |
| RTP resend | DEGRADED: 58 requests and zero recovered packets in periodic totals; 12 sampled reject records lacked reason attribution. |
| ALAC | PASS in retained interval: zero decode failures. |
| PCM | FAIL in retained interval: 102,200 active-stream underflow frames across reported deltas; occupancy snapshots 5,654, 4,533, 1,037 frames. |
| Display/XMB resume | NOT ESTABLISHED by this log snapshot; no fresh user visual observation was recorded. |
| v0.1.36 source regression | PASS: bounded recovery of an in-window retransmission after tracking loss; full Lima ASan/UBSan suite and PPU component compilation pass. |
| v0.1.36 package/RPCS3/PS3 | PENDING. |


## v0.1.35 hardware baseline and v0.1.37 emulator follow-up (2026-11-07)

| Test | Result | Evidence |
|---|---|---|
| Physical baseline identity | PASS: v0.1.35 / SFO 00.35 | Prior read-only FTP PARAM.SFO validation. |
| Physical AirPlay continuity | DEGRADED | 34-second wrapped log: encrypted 44.1 kHz session, 2,560 received packets, 195 missing, 214 late, zero recoveries in retained checkpoints, and zero ALAC failures. |
| Physical PCM output | DEGRADED | 86,106 active-stream underflow frames (~1.79 seconds at 48 kHz); occupancy snapshots reached 0–11,234 frames. |
| v0.1.37 Lima host/sanitizer/PPU/package validation | PASS | Full source suite, sanitizer checks, PPC64 build and finalized production PKG validation completed in Lima. |
| v0.1.37 package identity | PASS | Production AIRPLAY02 package, SFO `00.37`, 3,236,528 bytes, SHA-256 `9ee7f1a419fea99a1fcbf40306159db100a665ae7c482f12376b1f18bd9104a3`. |
| RPCS3 installation and launch | PASS | Emulator-only package installed as SFO `00.37`; running window showed the background. |
| RPCS3 RTSP/RTP/decrypt/ALAC | PASS, bounded | Synthetic encrypted session returned 200 for OPTIONS/ANNOUNCE/SETUP/RECORD; key unwrap and an ALAC frame decode succeeded; TEARDOWN completed. Existing 128-packet RTP-to-PCM fixture passed. |
| RPCS3 discovery | NOT ESTABLISHED | RPCS3 logged an mDNS initialization failure; direct sender was used. |
| Static-screen flip behavior | PASS in emulator log; hardware pending | Repeated `SCREEN_FLIPS value=0` heartbeats after initial presentation; one screenshot showed the background. PS3 HDMI and resume behavior remain unverified. |
| Sustained playback and audible quality | NOT ESTABLISHED | The bounded emulator run is not a sustained or acoustic test. No new hardware listening result. |
| Physical v0.1.37 deployment | BLOCKED | Direct FTP and webMAN endpoint were unreachable at preflight; installed app and staged package directory were left untouched. |

## Physical v0.1.39 deployment (2026-11-07)

| ID | Scenario | Result |
|---|---|---|
| DEPLOY-39-01 | Stage production package under a new name and download-back verify | PASS; 3,236,976 bytes, SHA-256 `9f67340331eaae9e5ecbe94adaee03a0201e79e652a6bf45fa14cf6922877840`. |
| DEPLOY-39-02 | Install and verify installed title identity | PASS; webMAN install returned success and FTP-read installed SFO reports AIRPLAY02 / 00.39. |
| DEPLOY-39-03 | Launch and inspect fresh capped log | PASS to startup checkpoints; active app reported by webMAN. Log records tone completion path but does not prove the user heard it or saw a stable screen. |
| DEPLOY-39-04 | LAN DNS-SD and RTSP OPTIONS probe | PARTIAL; RAOP browse result found. TCP probe connected, receiver logged a sent 200 response, but probe client got EOF. Resolve and client-visible response remain unresolved. |

## v0.1.39 live stream and remote-exit follow-up (2026-11-07)

| ID | Scenario | Result |
|---|---|---|
| PS3-06 | Encrypted RTP/ALAC/PCM stream path | PASS to PCM submission: recording state 3, 44.1 kHz stereo, two 512-packet batches; successful decrypt and ALAC frame checkpoints; 240,128 GameOS audio frames submitted in each of two report intervals. Not an HDMI loopback measurement. |
| PS3-07 | Packet loss and recovery | Two receive batches logged 42 and 57 missing sequence numbers; periodic counters logged 65 and 47 recovered packets, 23 and 30 resend requests, 10 and 6 concealed frames, and zero skips/rejections. Counters are on different sampling boundaries and do not reconcile packet-by-packet. |
| PS3-08 | PCM/RTP queue and decoder health | RTP queue depth 97 then 93 / 256; PCM ring 12,426 then 12,512 / 16,384 frames. No ALAC failure or PCM underflow event in the retained active-stream window; earlier log was overwritten at cap. |
| PS3-09 | webMAN remote exit and post-exit check | PASS; HTTP 200 command, SYSUTIL_EXIT_GAME, cleanup through APP_STOP. Repeated FTP log download was byte-identical and read-only webMAN status showed XMB. No reboot or forced power-off. |
| PS3-10 | Controller button handling | FAIL on v0.2.2 manual reboot/direct launch: user reports no buttons work. 3,672-byte log SHA-256 `d51d7fb882e0ddb5dc60cae08449bb1aab4292c59c700ecb0fde5a1b051c0c9f`; pad presence and `SYSUTIL_MENU_OPEN` are logged, but no raw digital transition, START, or menu-close event. Visible menu response remains unconfirmed. |


## v0.2.4 PS-menu refresh and append-only diagnostics

| Test | Result | Boundary |
|---|---|---|
| PS-menu framebuffer refresh | PASS by source/PPU build only: candidate resumes paced flips while menu-open. | Hardware confirmation required; hypothesis only. |
| Append logs across launches | PASS: host regression reopens the same path and verifies both timestamped APP_START records and the first session's unique event remain. | Session cap is 16 KiB; total file grows by launch count. |
| Per-session cap | PASS: host regression verifies LOG_SESSION_CAPPED and rejects later writes without altering earlier session bytes. | Collect post-deploy log over FTP. |

## v0.2.4 build and deployment

| ID | Scenario | Result | Boundary |
|---|---|---|---|
| BUILD-24-01 | Full Lima host/sanitizer suite and PPU component build | PASS; protocol, encrypted RTP/ALAC/PCM loopback, mutation, PCM/gain, tone, and PPU checks completed. Append-only log regression passed. | Emulator/host tests do not validate the PS menu or physical audio. |
| BUILD-24-02 | Standalone ELF and package checks | PASS; CellOS PPC64 ELF, five load segments; AIRPLAY02, SFO 00.44, content ID, assets, extraction and debug PKG envelope validated. | Host build boundary only. |
| BUILD-24-03 | Package identity | PASS; 3,231,984 bytes, SHA-256 `a27d5c7b5d2d84f12fbb82ee4e164b9b50ccc825b5104b9b749f053d72a42bf7`; source `f870d823f15398691a6a38e6393f8d1376161bee`. | Artifact retained locally under ignored `work/builds/0.2.4/`. |
| DEPLOY-24-01 | Stage package and verify download-back | PASS; FTP download-back is byte-identical to local PKG. | New versioned filename; prior staged packages preserved. |
| DEPLOY-24-02 | Install and verify title payload | PASS; installed SFO exact 00.44; installed EBOOT exactly matches package SELF. | User later launched and reported runtime results below. |

| PS3-24-01 | PS menu, resume, and START | PASS: user reports PS menu opens and START exits to XMB. | Resume shows a dim screen with a frozen loading indicator; fix pending. |
| PS3-24-02 | AirPlay music | PASS to user-confirmed streamed music. | Brief stutters/cuts remain; XMB scrolling no longer appears to worsen them. |
| PS3-24-03 | Active-stream diagnostics | PASS to encrypted 44.1 kHz RTP decrypt, ALAC frame decode, and PCM submission; ALAC failures 0. A 512-packet batch reported 62 missing sequence numbers; RTP queue depth 79/256. | v0.2.4 reached its 16 KiB cap 37 seconds after APP_START, so logs do not cover the requested 10-15 minute run. |
| PS3-24-04 | Append preservation | PASS for observed bytes: two timestamped v0.2.4 APP_START records appear after the earlier log, whose 7,270 bytes match the prefix of the combined log. | Log growth stops at the current session cap; the long run has no newer telemetry. |
| PS3-24-05 | Passive 12-minute observation | PASS for non-interference: no pause, exit, restart, or install action was sent during the window; repeated FTP snapshots remained byte-identical after the cap. | Does not prove audio remained continuous throughout the interval; logger was capped 37 seconds into the active-stream session. |

## v0.2.5 candidate gate

| Test | Result | Boundary |
|---|---|---|
| Append-only 5 MiB cap regression | PASS; fills per-run 5 MiB limit, verifies LOG_SESSION_CAPPED and post-cap rejection, and confirms two timestamped launches plus earlier session data remain. | Host/sanitizer test; hardware writes still need post-deploy confirmation. |
| One-command release gate | PASS on source `9d9f60657f117daedc47c6b1bc3285a5982e5ae3`; host/ASan/UBSan, PPU component and standalone app build, ELF validation, package extraction and SFO 00.45 checks all pass. | Lima guest only. Initial run exposed missing PS3DEV export; wrapper was fixed and full gate rerun successfully. |
| v0.2.5 package identity | PASS; 3,232,048 bytes, SHA-256 `61e8310a6d4330bad1e4d64213365f9ba27de81a6fa14851a315f5b338894440`; ELF 2,754,776 bytes, SHA-256 `1ff2513b9823792aa4f202ddf700e161bee5f223e3ccc2fc19e53e39a3f1eead`. | Built from source `9d9f60657f117daedc47c6b1bc3285a5982e5ae3`; retained locally under ignored `work/builds/0.2.5/`. |
| Console install/launch | Install PASS: staged download-back matches local PKG; installed SFO exact 00.45; installed EBOOT matches package SELF followed by 48 zero bytes. User reports launching and streaming. | 30-minute soak in progress; retrieve final log and report runtime metrics after the window. |

## v0.2.6 candidate

- Host: synthetic 1,416-byte ALAC parse/decode/reference, encrypted clear-tail pipeline, gap splitting/bounded retries, malformed metadata reset, RTP jitter/wrap and ten-minute SRC conservation.
- Display: production presenter with deterministic clock/RSX mocks covers paced resume, return to static, and stalled-flip timeout. This is not HDMI/VSH validation.
- Hardware pending: PS menu opens, close/resume gives bright stable image without frozen overlay, START exits, repeat twice; then same-song AirPlay comparison with malformed/retry/concealment deltas. Keep safe-v0.2.4 rollback.

- v0.2.6 exact-source release gate PASS: encrypted 1,416-byte reference PCM, resend split/retry budgets, presenter mocks, all existing host/sanitizer tests, PPU ELF/NPDRM/PKG/SFO. Physical tests remain pending.


## v0.2.7 fidelity/resume candidate

| Check | Result | Boundary |
|---|---|---|
| Full Linux release gate / PowerPC / ELF-SELF-PKG | PASS | Exact packaged source39e918c; SFO00.47 |
|64tap SRC independent reference, sine/noise/impulse/music | PASS stated thresholds |20k reference SNR75.09dB; music63.75dB; fullband-noise25.35dB exposes transition limit |
| Phase continuity / stream restart / sample ratio | PASS |1/256/480 chunks exact;600 simulated seconds, no underflow |
| RTP retry/loss/reorder regressions | PASS |Existing bounded policy preserved; host fixtures only |
|250/350/500/750ms display guards, repeated/rapid menu/callback cycles | PASS |Default350ms mock settles under500ms, stall timeout retained |
| Audio service/read-index aggregator and cost probes | PASS host |Physical PS3 timing still unmeasured; maxima are observations |
| v0.2.7 physical receiver run | PARTIAL | Dev log confirms 30m03s app runtime, three RECORD sessions, RTP/decrypt/ALAC/PCM progress and clean START cleanup. 26,814 PCM underflow output frames; no acoustic measurement. |
| v0.2.7 physical XMB resume/display | NOT VERIFIED | No XMB menu/resume cycle in the captured playback session; visible restoration and flicker remain user-observation gates. |

## v0.2.8 reliability/branding candidate

| Check | Result | Boundary |
|---|---|---|
| RTP live-edge resynchronization and discontinuity recovery | PASS on host sanitizer tests | Synthetic queue/socket backlog; PS3 scheduler and acoustic recovery remain untested. |
| Exact-source Lima release gate | PASS on corrected source `9f7af0027b7c3e9470a1583bbfbf676f9525f72d`; host sanitizer/forensic, protocol/audio, PPU app, ELF, package and SFO checks passed. | Lima Ubuntu 24.04 x86_64 guest; hardware acceptance is separate. |
| PPU/ELF/SELF/PKG/SFO/artwork validation | PASS; PPC64 CellOS ELF 2,755,120 bytes; debug PKG 3,906,640 bytes; SFO `00.48`; AIRPLAY02 / `AIR PS3`; extracted assets byte-matched. | Candidate v0.2.8; package SHA-256 in engineering report. |
| Dev-unit installation identity | PASS; FTP download-back hash matched; installed SFO/EBOOT matched the package installation payload. | Verifies deployment identity, not successful launch or playback. |
| Dev-unit active stream telemetry | PARTIAL PASS per user-supplied sanitized summary: 10m41s; 69,632 RTP packets; 17 missing observations all recovered; no reported RTP drops, conceal/skips, PCM underflows, or ALAC failures. | Not a 30-minute run and not subjective audible-quality confirmation. Full aggregate record: `docs/reports/air_ps3_v028_reliability_branding.md`. |
| AirPlay request/control telemetry | PARTIAL: 330 OPTIONS/200, one stream setup, 18 volume updates, one FLUSH; repeated timing-packet rejects plus one accepted reply. | Investigate timing-packet compatibility; AirPlay volume events do not test DualShock controls. |
| Display, PS-menu resume, START exit and clean shutdown | NOT VERIFIED in the v0.2.8 capture; it contains no menu/resume events. | Requires direct user observation on hardware. |
| Human-confirmed continuous audio / 30-minute run / repeat reconnect | NOT VERIFIED for v0.2.8. | Required before stable receiver milestone. |
