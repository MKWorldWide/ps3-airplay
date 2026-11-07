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
| Runtime compatibility | Compare current PSL1GHT runtime against 2020 runtime | v0.1.2 using the current runtime and v0.1.3 using PSL1GHT commit `6e565a70e927f55813babe86dc6f64149535f908` (2020-11-25) both fail XMB launch with user-reported `80028F21`. The older runtime alone did not resolve the failure; neither result identifies the cause. |
| Standalone package | Package inspection, extraction, and installed-file identity | v0.1.2: 3,225,712 bytes, SHA-256 `923aebf6eca5693b21b494a9a4225c36629f13e8b316b2825d5deb3763226ebf`. v0.1.3: 3,183,408 bytes, SHA-256 `98d8701ab018df882e30ba28f851a994c195d0a144dafe7c68c77d86d94126aa`. Installed v0.1.3 SFO and extracted-package EBOOT match exactly. XMB launch fails with `80028F21`; the capped 1,817-byte log is unchanged from the earlier run. |
| Resident PCM output | Plugin audio output | Not implemented; candidate discards decoded PCM |

## PS3 and iPhone gates

| ID | Scenario | Status |
|---|---|---|
| DEPLOY-01 | FTP preflight, package staging, and downloaded-back hash verification | PASS. v0.1.2 is staged at `/dev_hdd0/packages/ps3-airplay-receiver.pkg`, 3,225,712 bytes; downloaded-back SHA-256 matches `923aebf6eca5693b21b494a9a4225c36629f13e8b316b2825d5deb3763226ebf`. Previous 01.01 PKG backup is verified locally. |
| DEPLOY-02 | Replace staged package with corrected NPDRM PKG and verify downloaded-back hash | PASS; 416,240 bytes, exact SHA-256 match |
| DEPLOY-03 | Replace staged package with screen-enabled 01.01 candidate and verify downloaded-back hash | PASS; 3,222,640 bytes, byte-identical to approved artifact |
| DEPLOY-04 | Install v0.1.2 and verify installed app identity against candidate | PASS: SFO reports AIRPLAY02 / 0.1.2; installed EBOOT (445,232 bytes) and SFO match the extracted v0.1.2 files by SHA-256 |
| DEPLOY-05 | Stage/install the v0.1.3 legacy-runtime candidate and verify launch | Upload PASS: versioned package staged at `/dev_hdd0/packages/ps3-airplay-receiver-0.1.3.pkg`; download-back matched size 3,183,408 bytes, SHA-256 `98d8701ab018df882e30ba28f851a994c195d0a144dafe7c68c77d86d94126aa`, byte-for-byte. Install PASS: SFO and extracted-package EBOOT match. Launch FAIL: user reports the same `80028F21`. Staged v0.1.2 remains available for rollback. |
| BUILD-03 | Cross-build visible diagnostic screen and controller START exit; inspect packaged image | PASS in Lima for 0.1.2; hardware behavior untested |
| HW-01 | Confirm tested recovery procedure and record a privacy-safe baseline | Partial: console remained reachable for read-only checks after the launch error; clean in-app exit and recovery from a new freeze remain unproven. Standalone app does not alter boot plugins. |
| HW-02 | Install/launch standalone app and exit cleanly | v0.1.3 FAIL at launch: user reports the same XMB error `80028F21`. Installed SFO reports AIRPLAY02 / 0.1.3 and matches the extracted package; installed EBOOT also matches. The 1,817-byte log is unchanged, so no new startup event is available. Display, tone, controller, and clean exit are unverified for v0.1.3. |
| HW-03 | Audible test audio at the system menu | Prior partial PASS only: user heard the five-second diagnostic tone in a run whose package identity was not established. v0.1.2 tone test BLOCKED by launch failure; normal AirPlay music not tested. |
| HW-04 | iPhone discovers receiver and completes AirPlay session | Prior partial result only: iPhone listed `ps3-airplay-diagnostic`; selecting it failed to connect. No v0.1.2 connection attempt; session not established. |
| HW-05 | Verify audible AirPlay playback and controls | NOT RUN |
| HW-06 | Reconnect after sender/network interruption | NOT RUN |
| HW-07 | Sustained standalone playback and resource stability | NOT RUN |
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
