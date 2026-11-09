# Structured Resident Diagnostics Journal

## Timestamp in EDT

2026-11-08 20:53 EDT

## Scope of the pass

Add a structured diagnostic channel from the resident PRX to the RPCS3 test
harness, add a repeatable host concurrency test, and document validation and
runtime limitations. No PS3 package was installed and no physical console was
contacted.

## Why this pass was needed

RPCS3 reports that the resident cannot open its configured file log. The old
shared diagnostics flags preserve lifecycle milestones, but do not preserve
individual events, timestamps, or logger failure details for later analysis.

## Files changed

- `experimental/resident/resident_diagnostics.h`
- `experimental/resident/module.c`
- `experimental/resident/harness/source/main.c`
- `experimental/resident/README.md`
- `tests/resident/resident_diagnostics_test.c`
- `scripts/test-resident-diagnostics.sh`
- `docs/memory.md`
- `docs/lessons-learned.md`
- `docs/scratchpad.md`
- `docs/experimental/RESIDENT_OVERNIGHT_REPORT.md`
- `docs/decisions/2026-11-08_2053_structured_resident_logging.md`

## Architecture / logic changes

The diagnostics ABI now embeds 128 fixed-size records. Producers atomically
reserve one unique slot, fill it, then publish its sequence with release
ordering. This supports resident startup, receiver, audio, and stop threads
without blocking them on file I/O. A fixed capacity bounds memory and work;
further events increment a dropped-record count.

The resident mirrors existing internal `log_event` calls into the journal,
including events when the PRX logger is unavailable, plus explicit records for
logger failures and audio PCM submission/drain. The RPCS3 harness drains only
committed records during event waits and teardown, writing them to its own
file and stdout. Its own events carry elapsed milliseconds; resident events
carry system monotonic microseconds. Fields exclude packet contents, keys,
headers, peer/device identifiers, and stream metadata.

## Runtime behavior changes

The experimental PRX writes diagnostic records into the memory region supplied
by the harness. The harness log now interleaves its lifecycle events with
resident event records and emits a per-cycle dropped-record count. Production
file-log behavior and the standalone receiver are unchanged.

## Validation performed

- `scripts/test-resident-diagnostics.sh` — PASS under AddressSanitizer and
  UndefinedBehaviorSanitizer; 128 concurrent records committed, validated,
  and overflow counted.
- Harness `-fsyntax-only` compilation against local mock API declarations —
  PASS.
- `python3 tests/resident/resident_container_test.py` — PASS, 4 tests.
- `bash -n scripts/build-lima-resident-rpcs3-harness.sh` — PASS.
- `git diff --check` — PASS.
- PPU build and fresh RPCS3 install/run — BLOCKED: no PS3DK compiler or
  resident Lima build endpoint is available in this session.

## Immediate results

The queue logic passes its host concurrency, publication-integrity, and
overflow checks. The harness source accepts the expanded diagnostics ABI in
the host syntax check. No fresh package exists, so PPU ABI compatibility,
RPCS3 record delivery, and log-file contents remain unverified.

## Known limitations / follow-ups

- Rebuild PRX and `RPRX90001` together because the shared diagnostics structure
  size changed.
- Install/run that fresh package in RPCS3 and verify ordered resident sequence
  records, timestamps, logger-unavailable evidence, and zero dropped records.
- The shared journal is finite and does not overwrite old records. Sustained
  high-volume events may overflow; the harness reports the exact drop count.
- Install-manager logs and package hashes still come from the host build/
  emulator log collection; this pass does not capture RPCS3 GUI installer logs
  into the in-app file.
- No resident RAOP network session, audible output, package install, or hardware
  test was performed.

## Risks introduced

- The diagnostics structure grows by roughly 7 KiB; both sides must be rebuilt
  from the same header, and PS3 PPU ABI behavior must be checked.
- Record publication uses compiler atomic builtins. The host test checks
  concurrent publication semantics, but PPU lowering and emulator memory
  behavior have not been exercised.
- Existing event cadence can fill the bounded journal during a long stream;
  overflow is surfaced but events after capacity are not retained.

## Decision record (embedded)

### Decision 1: use a bounded shared journal

- **Context:** The RPCS3 PRX file path is unavailable, while the harness can
  read the memory passed to `module_start`.
- **Options considered:** Add more summary flags; synchronously print every
  event from the PRX; use a fixed shared record journal.
- **Why chosen:** A journal preserves ordered per-event evidence without
  depending on PRX file I/O and without blocking real-time workers on stdout.
- **Tradeoffs accepted:** Fixed memory cost and explicit overflow rather than
  unbounded logging or overwriting evidence.

### Decision 2: retain privacy-safe event fields

- **Context:** Network diagnostics could accidentally expose stream or device
  details in logs that may later be shared.
- **Options considered:** Log raw protocol fields; log internal event names,
  numeric status/counters, and byte counts only.
- **Why chosen:** The latter supports lifecycle and pipeline diagnosis while
  excluding credentials, payloads, and identifiers.
- **Tradeoffs accepted:** Some failures need code-side event naming rather than
  inspecting raw packets.

### Decision 3: hold RPCS3 verification until a fresh build exists

- **Context:** The diagnostics structure changed size and an older package is
  still available locally.
- **Options considered:** Install the old package and treat its behavior as
  representative; wait for a synchronized PRX/harness build.
- **Why chosen:** An old binary cannot validate the new ABI or journal.
- **Tradeoffs accepted:** Runtime validation remains pending until the build
  endpoint is restored.

## Next recommended step

Restore the resident PS3DK/Lima build environment, rebuild the PRX and harness
from this branch, verify the package embeds the matching fake SPRX, then run
one RPCS3 test and inspect resident sequence ordering and drop count before
attempting the 50-cycle run.
