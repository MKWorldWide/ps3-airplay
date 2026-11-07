# Test matrix

This matrix distinguishes portable host evidence and cross-build evidence from real hardware validation. Update rows only when the listed scenario is actually exercised.

**Demonstration target:** PS3 with Evilnat 4.93 CFW; Cobra 8.5 observed. Model not recorded. This configuration is context for future community testing and is not hardware compatibility evidence.

## Host and build checks

| Area | Evidence | Status |
|---|---|---|
| RTSP | Persistent bounded session, state transitions, malformed input, deadlines, loopback | Host tests pass |
| RTP | Header validation, peer filtering, sequence ordering, bounded queue, resend request/response validation | Host tests pass |
| Timing | Bounded timing probe, matching response and RTT diagnostics | Host loopback tests pass; sender-clock playback mapping is not implemented |
| Crypto | Base64, AES, RSA session-key recovery and challenge helpers | Host vector tests pass; real sender interoperability untested |
| SDP/ALAC | Bounded format parsing and reference packet decoding | Host tests match the reference samples |
| PCM ring | Conversion, resampling, overflow and underflow bounds | Host tests pass |
| Audio diagnostic | Bounded 440 Hz stereo tone, five-second duration, conservative peak and fades | Host unit test; user reports hearing tone once on PS3; not independently verified |
| Discovery | RAOP response serialization and capability consistency | Host wire-format test passes for the 0.1.2 TXT version; PS3 LAN/iPhone discovery for this build untested |
| Diagnostics | Allowlisted RTSP methods/status/state, SDP numeric format, socket failures/readiness, RTP batch events; log cap and rollover | Host log redaction/cap/rollover checks pass; verbose receiver events cross-build; PS3 log retrieval for 0.1.2 not run |
| PPU/PRX | Cross-build and PRX structural checks | Build checks pass; console compatibility untested |
| Standalone package | Package inspection and extraction checks | Prior 01.01 was staged and hash-verified; 0.1.2 builds/extracts locally, not transferred |
| Resident PCM output | Plugin audio output | Not implemented; candidate discards decoded PCM |

## PS3 and iPhone gates

| ID | Scenario | Status |
|---|---|---|
| DEPLOY-01 | FTP preflight, package staging, and downloaded-back hash verification | PASS; first 2,560,368-byte package staged and verified |
| DEPLOY-02 | Replace staged package with corrected NPDRM PKG and verify downloaded-back hash | PASS; 416,240 bytes, exact SHA-256 match |
| DEPLOY-03 | Replace staged package with screen-enabled 01.01 candidate and verify downloaded-back hash | PASS; 3,222,640 bytes, byte-identical to approved artifact |
| BUILD-03 | Cross-build visible diagnostic screen and controller START exit; inspect packaged image | PASS in Lima for 0.1.2; hardware behavior untested |
| HW-01 | Confirm tested recovery procedure and record a privacy-safe baseline | NOT RUN |
| HW-02 | Install/launch standalone app and exit cleanly | Unresolved: user reports tone, black screen, and forced shutdown; a log reports `SCREEN_READY` but visible output and exact build identity are unconfirmed; 0.1.2 awaits exact transfer approval |
| HW-03 | Audible test audio at the system menu | Partial PASS; user heard the five-second diagnostic tone; normal AirPlay music not tested |
| HW-04 | iPhone discovers receiver and completes AirPlay session | Partial: iPhone lists `ps3-airplay-diagnostic`; selecting it fails to connect; session not established |
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
