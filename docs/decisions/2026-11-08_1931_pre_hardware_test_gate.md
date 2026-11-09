# Pre-hardware resident test gate implementation

## Timestamp in EDT

2026-11-08 19:31 EDT

## Scope of the pass

Expanded the RPCS3 harness source to a 50-cycle audio/lifecycle stress plan with distinct test tones and multiple stop timings. Added a diagnostic flag for suppressing seeded PCM on selected cycles, a PS3DK SELF-container type validator with unit tests, and updated resident notes and readiness evidence.

## Why this pass was needed

The last emulator package ran only three cycles, used the same 440 Hz test signal for resident and foreground submissions, and contained only the emulator fake-SPRX container. More work is needed before a physical test can be considered.

## Files changed

- `experimental/resident/module.c`
- `experimental/resident/resident_diagnostics.h`
- `experimental/resident/harness/source/main.c`
- `experimental/resident/harness/CMakeLists.txt`
- `scripts/build-lima-resident-rpcs3-harness.sh`
- `scripts/validate-resident-containers.py`
- `tests/resident/resident_container_test.py`
- `experimental/resident/README.md`
- `docs/experimental/RESIDENT_OVERNIGHT_REPORT.md`
- `docs/memory.md`
- `docs/lessons-learned.md`
- `docs/scratchpad.md`
- `docs/decisions/2026-11-08_1931_pre_hardware_test_gate.md`

## Architecture / logic changes

The harness defaults to 50 load/start/stop/unload cycles and alternates through: no seeded resident PCM, stop immediately after resident PCM submission, stop after an explicit audio-worker drain event, and a longer foreground/resident dual-port submission. The resident diagnostic buffer now fills the existing bounded 16,384-frame SPSC queue. Resident and foreground tones use 440 Hz and 660 Hz. A test-only diagnostics flag can suppress the resident seed.

The new validator checks PS3DK SELF header type `0x00010001` for the signed SPRX and `0x80000001` for the emulator fake SPRX. The build script validates both outputs before packaging and retains its byte-for-byte check that the RPCS3 package embeds only the fake SPRX.

## Runtime behavior changes

The production RAOP path is unchanged. Only diagnostic module starts passed the diagnostics structure can suppress or seed test PCM. The expanded harness has not yet been built into a package or run in RPCS3.

## Validation performed

- `python3 tests/resident/resident_container_test.py`: four tests passed, covering valid distinct types, fake-as-production rejection, malformed headers, and identical containers.
- Host C syntax check for the updated harness passed with temporary declarations for the PS3 PRX loader APIs and existing audio mocks. This is a syntax check, not a PS3DK ABI or PPU build.
- `git diff --check`: passed.
- Ran the container classifier against retained ignored artifacts; their headers classify as signed SPRX and fake SPRX respectively. Those retained files are not a fresh production build from this source revision.
- PPU rebuild and RPCS3 stress run were unavailable: this environment has no PS3DK/Mbed TLS toolchain checkout, and prior Lima/SSH endpoints did not respond. The previous three-cycle RPCS3 result applies only to the earlier package.
- No physical PS3 was contacted.

## Immediate results

- Four host format-validator tests pass.
- Harness source now specifies 50 varied lifecycle cycles and distinguishable tone frequencies.
- No current package, fresh signed SPRX, resident RAOP run, captured audio, or 50-cycle emulator evidence was produced.

## Known limitations / follow-ups

- The production SELF type is a container-format distinction; it does not prove the Evilnat/Cobra loader accepts its key table. No resident hardware PKG is present.
- RPCS3 still reports `EADDRNOTAVAIL`; synthetic RAOP through the resident remains untested.
- No system audio capture or independent listening result is available. Two `cellAudio` ports accepting submissions would still not prove audible mixing.
- Module counts do not measure every socket or heap allocation.
- `sysThreadJoin` remains blocking after the worker signals cleanup completion. Its behavior if thread exit itself stalls is not bounded or proven by these tests.

## Risks introduced

The diagnostics structure gained a flags field and the test tone seed grew to the full PCM queue capacity. This is an experimental test ABI shared by the harness and resident build; both must be rebuilt together. The 50-cycle run may take over a minute due to its longer dual-port intervals and should be interpreted only if every cycle logs a pass and returns module count to baseline.

## Decision record (embedded)

### Decision

Make the stress harness use distinct tones and vary stop timing, defaulting to 50 cycles, while validating production and emulator SELF containers separately.

### Context

Three clean cycles covered one stop timing only. Identical tone frequencies made a future recording ambiguous. The existing package intentionally embeds a fake SPRX for RPCS3 and must not be confused with a hardware candidate.

### Options considered

- Keep the existing three-cycle harness and infer safety from earlier results.
- Add separate one-off test applications for every lifecycle state.
- Extend the existing harness with bounded modes and cycle count, and validate each container's declared SELF type.

### Why this option was chosen

The existing harness already exercises module load, ready, stop, unload, and module-list baseline. Extending it retains that reusable path while increasing repetition and varying audio stop timing. A compact SELF header validator provides a mechanical distinction without claiming loader compatibility.

### Tradeoffs accepted

The test-only diagnostics ABI changes alongside the PRX and requires a full rebuild. The 50-cycle source and container validator are not runtime evidence until rebuilt and rerun with the pinned toolchain. Distinct frequencies support future capture but do not create capture capability.

## Next recommended step

Restore access to the pinned PS3DK/Mbed TLS Lima environment, rebuild the PRX and `RPRX90001` package from this commit, verify fresh signed/fake SPRX hashes and embedded fake payload identity, and run the 50-cycle RPCS3 harness. Then establish a network-capable synthetic RAOP test and capture output before reconsidering physical hardware.
