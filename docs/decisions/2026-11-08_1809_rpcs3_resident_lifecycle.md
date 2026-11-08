# RPCS3 resident lifecycle verification

## Timestamp in EDT

2026-11-08 18:09 EDT

## Scope of the pass

Adjusted the emulator harness to load PS3DK's fake-signed SPRX, pass a narrow
diagnostic structure into the PRX, tolerate a missing log only for this
diagnostic run, and require lifecycle events plus module-count baseline checks.
All resident failure assertions now attempt stop, unload, and baseline
verification before the harness exits. Updated the resident test notes with
the observed RPCS3 boundary and artifacts.

## Why this pass was needed

The raw PRX failed before `module_start`, and the PRX previously returned
`SYS_PRX_NO_RESIDENT` when RPCS3 could not open its log file. That prevented
the requested repeated runtime lifecycle test.

## Files changed

- `experimental/resident/README.md`
- `experimental/resident/harness/CMakeLists.txt`
- `experimental/resident/harness/source/main.c`
- `experimental/resident/module.c`
- `experimental/resident/resident_diagnostics.h`
- `scripts/build-lima-resident-rpcs3-harness.sh`
- `docs/experimental/RESIDENT_OVERNIGHT_REPORT.md`
- `docs/memory.md`
- `docs/lessons-learned.md`
- `docs/scratchpad.md`
- `docs/decisions/2026-11-08_1809_rpcs3_resident_lifecycle.md`

## Architecture / logic changes

The harness now embeds `.fake.sprx` because RPCS3 rejected the raw `.prx` with
`0x80011148`. A shared diagnostics structure passed as `module_start` args
records startup stage, error, and atomic lifecycle flags. Only when that
valid structure is present may missing file logging be bypassed, allowing
RPCS3 lifecycle checks without changing default receiver startup behavior.
Worker readiness, worker completion, and module stop are exposed to the test
app. Package generation extracts and byte-compares the embedded SPRX.
One shared stop/unload helper handles both passing cycles and post-start
assertion failures, reducing the chance a failed check leaves a resident
module active during emulator testing.

## Runtime behavior changes

The diagnostic PRX can remain resident in RPCS3 while its file logger is
unavailable, provided the harness supplies the expected diagnostics block.
Normal module startup without that explicit test block continues to fail
closed when log creation fails. The resident still discards decoded PCM.

## Validation performed

- Full Lima host regression and sanitizer-oriented RAOP/audio tests passed.
- PPU PRX linked and `prx-gen check` passed: one load segment, 6,101
  relocations, one management export, and four import libraries.
- Harness SELF and `RPRX90001` PKG built; extracted `.fake.sprx` matched the
  current output byte for byte.
- RPCS3 0.0.43-20570: raw PRX load failed with
  `0x80011148 CELL_PRX_ERROR_UNSUPPORTED_PRX_TYPE` / “Failed to decrypt file”.
- Installed and ran the fake-SPRX package. Three cycles each reported
  `STARTED`, `LOG_UNAVAILABLE`, `RECEIVER_READY`, `STOP_RESULT=0`,
  `WORKER_STOPPED`, `UNLOAD_RESULT=0`, and module count six after unload.
- Rebuilt, reinstalled, and reran after adding cleanup on diagnostic assertion
  failures; the three normal cycles still passed with count six after unload.
- `git diff --check` passed before documentation finalization.

## Immediate results

RPCS3 lifecycle test passed for three clean cycles. The file logger is not
available in this RPCS3 run. No synthetic AirPlay session or audio output was
tested; no physical console was accessed.

## Known limitations / follow-ups

Audio is still discarded after ALAC decode. The app has not driven synthetic
RAOP packets through the PRX. The module-count check does not measure sockets,
all memory, or all system resources. CellAudio behavior and game coexistence
remain untested. Hardware readiness remains NOT READY.

## Risks introduced

The diagnostic argument is a test-only opt-in and must only be trusted when
its size and magic match. The RPCS3 fake-SPRX result must not be generalized to
retail/Evilnat/Cobra signing or loading. A missing file logger can hide module
events if the harness diagnostics are not captured.

## Decision record (embedded)

### Decision 1: Use fake-SPRX for RPCS3

- **Context:** Raw PRX load stopped at the loader with unsupported type.
- **Options considered:** Repeatedly vary package location; modify signing;
  use PS3DK fake SPRX as a local emulator container.
- **Why chosen:** The precise loader failure occurs before the module callback,
  while the fake-SPRX form is accepted and keeps hardware release artifacts
  separate.
- **Tradeoffs accepted:** Emulator execution validates the fake container and
  callback path only, not hardware signing or Cobra compatibility.

### Decision 2: Permit test-only no-log lifecycle

- **Context:** RPCS3 reports the resident log unavailable, blocking callback
  and cleanup observation through the file.
- **Options considered:** Ignore the test; alter global logging behavior; pass
  a validated diagnostic structure and make it an explicit opt-in.
- **Why chosen:** It permits observable lifecycle testing while leaving normal
  startup's required-log behavior intact.
- **Tradeoffs accepted:** This emulator run does not resolve production
  logging; test diagnostics have a small shared-memory ABI.

## Next recommended step

Probe `cellAudio` initialization and PCM-port submission in a separate
RPCS3-controlled audio experiment, then build a bounded resident playback path
only if that API surface works under the emulator. Keep hardware closed until
the output and safety gates are independently verified.
