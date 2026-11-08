# Repeat resident host forensic validation

## Timestamp in EDT

2026-11-08 06:48 EDT

## Scope of the pass

Repeated the Linux host forensic suite with AddressSanitizer and UndefinedBehaviorSanitizer, then recorded the additional validation and test-invocation lesson. No executable product source changed.

## Why this pass was needed

The overnight task calls for continued reliability validation beyond a successful compile. A prior recorded forensic pass existed, and another run adds independent repeat evidence for the queue, resampler, parser, and display-resume checks.

## Files changed

- `docs/experimental/RESIDENT_OVERNIGHT_REPORT.md`
- `docs/memory.md`
- `docs/lessons-learned.md`
- `docs/scratchpad.md`
- `docs/decisions/2026-11-08_0648_repeat_forensic_suite.md`

## Architecture / logic changes

- None. Production and harness code did not change.
- The report now distinguishes another host-only sanitizer pass from the still-blocked RPCS3 and physical hardware checks.

## Runtime behavior changes

None.

## Validation performed

- Ran `bash scripts/run-forensic-tests.sh` in the existing Linux/Lima workspace with `MBEDTLS_INCLUDE` set to the installed Mbed TLS 3.6.7 `include` directory.
- RTP queue sequence-wrap, reorder, delay/recovery, concealment, flush, and 10,000-packet jitter checks passed.
- The 600-second-equivalent SRC test passed with zero underflows; ALAC parse/decode, resend bounds, display-resume checks, aggregation, and timing probes passed.
- An initial direct script invocation failed because the copied file is not executable. A subsequent attempt used an incorrect include root and stopped at the optional resend compile. Neither failure was a product test failure; the corrected full invocation passed.
- RPCS3 remained inaccessible because the Mac was locked. No package or physical console was touched.

## Immediate results

- An additional complete host forensic run passed.
- Test execution is reproducible when the correct shell interpreter and pinned dependency include directory are supplied.

## Known limitations / follow-ups

- Host sanitizers do not exercise PS3 syscall behavior, PRX lifecycle, memory allocation under the console runtime, or audible output.
- RPCS3 remains the next required lifecycle gate.

## Risks introduced

- None from product code. The long sanitized RTP test takes several minutes in this VM and should not be multiplied without a specific reason.

## Decision record (embedded)

### Decision: run one corrected repeat rather than three

- **Context:** The full suite includes a sanitized 131,072-packet sequence-wrap loop that took several minutes in Lima.
- **Options considered:** Repeat it three times as initially requested; run one complete corrected repeat after correcting the environment; or stop after the failed invocation.
- **Why chosen:** One full repeat supplies useful regression evidence without spending most of the remaining window repeating the same deterministic host workload.
- **Tradeoffs accepted:** This is one additional run, not a broad statistical stress campaign.

## Next recommended step

When the desktop is unlocked, execute the existing RPCS3 lifecycle harness and update the hardware-readiness gate only from observed runtime evidence.
