# Test matrix

This matrix distinguishes portable host evidence and cross-build evidence from real hardware validation. Update rows only when the listed scenario is actually exercised.

**Demonstration target:** PS3 with Evilnat 4.93 CFW; Cobra 8.5 observed. Model not recorded. This configuration is context for future community testing and is not hardware compatibility evidence.

## Host and build checks

| Area | Evidence | Status |
|---|---|---|
| RTSP | Persistent bounded session, state transitions, malformed input, deadlines, loopback | Host tests pass |
| RTP | Header validation, peer filtering, sequence ordering, bounded queue, resend request/response validation | Earlier host run passed; latest direct receiver loopback run timed out waiting for the timing probe, while a traced run completed. Treat as unstable pending diagnosis. |
| Timing | Bounded timing probe, matching response and RTT diagnostics | Latest direct loopback run did not complete; trace-assisted run reached timing reply and later passed. Host scheduling/test-harness issue is unresolved; sender-clock playback mapping is not implemented |
| Crypto | Base64, AES, RSA session-key recovery and challenge helpers | Host vector tests pass; real sender interoperability untested |
| SDP/ALAC | Bounded format parsing and reference packet decoding | Host tests match the reference samples |
| PCM ring | Conversion, resampling, overflow and underflow bounds | Host tests pass |
| Audio diagnostic | Bounded 440 Hz stereo tone, five-second duration, conservative peak and fades | Host unit test; user reports hearing tone once on PS3; not independently verified |
| Discovery | RAOP response serialization and capability consistency | Host wire-format test passes for the 0.1.2 TXT version; PS3 LAN/iPhone discovery for this build untested |
| Diagnostics | Allowlisted RTSP methods/status/state, SDP numeric format, socket failures/readiness, RTP batch events; log cap and rollover | Host log redaction/cap/rollover checks pass; verbose receiver events cross-build; PS3 log retrieval for 0.1.2 not run |
| PPU/PRX | Cross-build and PRX structural checks | Build checks pass; console compatibility untested |
| Known-good ELF ABI | Parse preserved v01.01 decrypted ELF and modern PPU output; reject legacy v0.1.6 ELF layout | PASS: reference and PS3DK candidate both have CellOS `0x66`, PPC64 `ET_EXEC`, entry `0x10000000`, and reference PT_LOAD flags; legacy v0.1.6 has generic `0x5`/`0x6` flags and is rejected. The layout difference is proven; its causal relationship to RPCS3's loader error is still a hypothesis. |
| Runtime compatibility | Compare PSL1GHT runtime paths against hardware evidence | v0.1.2 (newer runtime) and v0.1.3 (PSL1GHT commit `6e565a70e927f55813babe86dc6f64149535f908`, 2020-11-25) both returned user-reported XMB `80028F21`. AIRTEST01 PS3DK-native app returned the same code. This does not isolate the cause. |
| Standalone package | Package inspection, extraction, and installed-file identity | v0.1.2: 3,225,712 bytes, SHA-256 `923aebf6eca5693b21b494a9a4225c36629f13e8b316b2825d5deb3763226ebf`. v0.1.3: 3,183,408 bytes, SHA-256 `98d8701ab018df882e30ba28f851a994c195d0a144dafe7c68c77d86d94126aa`. Original v0.1.4: 3,183,584 bytes, SHA-256 `0c1e3c74eea71f7b5829201673f4dd5211bcbf704ab00cb55e1b41cb87020ee4`; its package paths were rejected. Corrected-path v0.1.4 and v0.1.5/v0.1.6 candidates install in RPCS3 but fail at EBOOT loading; details below. |
| RPCS3 package launch | Install and boot AIRPLAY02, then reach app logging | v0.1.4 original: package parser rejected absolute entry paths. Corrected-path v0.1.4 (SHA-256 `7811cdb221765aaa9e6ac3e57bd875cb8b7409045e6f371ff7a2a7a2d0744a8f`) installed but failed on unsupported EBOOT format. v0.1.5 installed but failed `Invalid ELF OS ABI`. v0.1.6 installed after setting OS/ABI `0x66`, but failed `Invalid ELF type` before app startup. No app log or audio in these v0.1.4-v0.1.6 RPCS3 runs. |
| Native PS3DK launch control | Cross-build and package a minimal cellGcm/controller standalone app using a unique title ID | Built and staged locally as AIRTEST01 v0.1.0; read-only FTP later found matching installed SFO/EBOOT. User reports XMB `80028F21`; its fresh log was unavailable. Separate from AIRPLAY02 and parked. |
| Resident PCM output | Plugin audio output | Not implemented; candidate discards decoded PCM |

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
| DISPLAY-17 | Render the background without diagnostic overlays | Source change removes the remaining `PRESS START TO EXIT` text. Build/package and RPCS3 visual verification pending; no claim that a particular display capture has been observed. |
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
