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
| Audio diagnostic | Bounded 440 Hz stereo tone, five-second duration, conservative peak and fades | Host unit test; PS3 audibility untested |
| Discovery | RAOP response serialization and capability consistency | Host wire-format test passes; LAN/iPhone discovery untested |
| PPU/PRX | Cross-build and PRX structural checks | Build checks pass; console compatibility untested |
| Standalone package | Package inspection and extraction checks | Host packaging checks pass; console install/launch untested |
| Resident PCM output | Plugin audio output | Not implemented; candidate discards decoded PCM |

## PS3 and iPhone gates

| ID | Scenario | Status |
|---|---|---|
| DEPLOY-01 | FTP preflight, package staging, and downloaded-back hash verification | PASS; first 2,560,368-byte package staged and verified |
| HW-01 | Confirm tested recovery procedure and record a privacy-safe baseline | NOT RUN |
| HW-02 | Install/launch standalone app and exit cleanly | FAIL on first package (`80029530`); corrected PKG validated locally, replacement/launch pending |
| HW-03 | Audible test audio at the system menu | NOT RUN |
| HW-04 | iPhone discovers receiver and completes AirPlay session | NOT RUN |
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
