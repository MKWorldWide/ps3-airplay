# Independent forensic audit and bounded payload reproduction

Timestamp: 2026-11-07 23:06:41 EDT (UTC-04:00); client local time is EST (UTC-05:00).

## Scope and reason

Investigate v0.2.5 audio cuts using a preserved hardware log and unchanged production code. No production behavior changed.

## Files changed

- `docs/memory.md`
- `docs/lessons-learned.md`
- `docs/scratchpad.md`
- `docs/reports/ps3_airplay_sol61_forensic_audit.md`
- `docs/reports/assets/sol61_audit_timeline.svg`
- `scripts/analyze-hardware-log.py`
- `scripts/run-forensic-tests.sh`
- `tests/observability/hardware_log_analysis_test.py`
- `tests/protocol/forensic_rtp_test.c`
- `tests/protocol/forensic_resend_test.c`
- `tests/protocol/forensic_payload_test.c`
- `tests/protocol/fixtures/README.md`
- `tests/protocol/fixtures/alac-noise-352.packet`
- `tests/protocol/fixtures/alac-noise-352.pcm`
- `docs/decisions/2026-11-07_2306_sol61_forensic_audit.md`

## Architecture and runtime behavior

Added offline numeric log analysis, a relative-time SVG, synthetic full-entropy ALAC fixtures and targeted host reproductions. Production runtime, artifacts and console execution were preserved.

## Validation and immediate results

Existing host suite in isolated guest directory passed, including sanitizer crypto/session/UDP/ALAC/PCM/cap tests. `bash scripts/run-forensic-tests.sh` passed local RTP/SRC/analyzer/payload cases; Linux private resend-policy reproduction passed. Existing RTP queue tests passed under ASan/UBSan. Installed artifacts matched retained v0.2.5 package. Raw hardware snapshot retained with SHA-256; no console stop/install/reboot.

Valid 1,416-byte ALAC is rejected by production 1,400-byte limits; copied-source 1,536-byte experiment decodes byte-exact PCM. >16 resend omission reproduced. Hardware completed intervals contain 6,886 silence frames; no physical-loss attribution established.

## Decisions

- Decision: isolate audit on a separate worktree/branch. Context: user requests independent investigation and production preservation. Options: edit main or isolate. Chosen isolation protects ongoing work; tradeoff is later integration.
- Decision: retain raw evidence privately and publish aggregate findings/synthetic fixtures. Options: raw logs or sanitized report. Aggregates preserve privacy; detailed packet tracing remains local.
- Decision: reproduce boundary defect in copied sources only. Options: immediate deploy, production edit or controlled reproduction. Chosen reproduction establishes exact PCM result without hardware regression; deployed correction remains follow-up.
- Decision: prioritize payload validity before broad timing changes. Context: deterministic failure for valid input. Options: larger timeout, rewrite engine or fix demonstrated boundary. Small bound/negotiation work has strongest evidence; actual hardware contribution still requires telemetry.

## Limitations and risks

No new RPCS3 run, real output capture, high-jitter end-to-end simulator or final clean-exit test. Logs do not embed commit identity or oversize counts. The offline analyzer cannot reconstruct unsampled packet ages. Synthetic PCM is deterministic noise, not user audio. No new runtime risks introduced; wrong counter interpretation remains a reporting risk documented in the report.

## Next recommended step

Implement a coherent payload-size/negotiation policy and numeric oversize counters in a separate development pass, then test encrypted incompressible frames before a controlled hardware comparison. Keep safe-v0.2.4 intact.
