# RPCS3 audio-thread fault address correction

## Timestamp in EDT

2026-11-08 17:58 EDT

## Scope of the pass

Corrected the RPCS3 crash-location interpretation in the current resident report, project memory, lesson record, and scratchpad. No runtime code changed.

## Why this pass was needed

The prior checkpoint mapped RPCS3's reported module offset directly onto the raw PRX `.text` section and named `sysThreadExit` as the fault location. That comparison did not account for relocation and was too strong.

## Files changed

- `docs/experimental/RESIDENT_OVERNIGHT_REPORT.md`
- `docs/memory.md`
- `docs/lessons-learned.md`
- `docs/scratchpad.md`
- `docs/decisions/2026-11-08_1758_rpcs3_fault_address_correction.md`

## Architecture / logic changes

None. The RPCS3 observation remains a fatal null-address execution fault in the named `ps3-airplay-audio` thread. The logged `0x001201c8` module location is retained as raw emulator evidence but is no longer assigned to a named PRX function.

## Runtime behavior changes

None.

## Validation performed

- Re-read the exact RPCS3 fatal-log line from the RPCS3 GUI log.
- Rechecked the PRX ELF section/symbol layout and confirmed a direct comparison of the emulator's relocated module location to the raw PRX `.text` virtual address is insufficient to prove a function mapping.
- `git diff --cached --check` passed for this correction. No runtime tests were rerun because no code or test changed.

## Immediate results

- PASS: The report now distinguishes the observed null-address fault from its unknown call site and root cause.
- PASS: Earlier `sysThreadExit` attribution is explicitly withdrawn in the current report and current project memory.
- UNTESTED: Whether the fault is caused by the resident, the RPCS3 implementation, a resource interaction, or an unload race.

## Known limitations / follow-ups

The exact code address must be resolved using a runtime-aware PRX load map or debug symbols from the exact fake-SPRX artifact. Continue instrumenting worker exit/join and retain the failure until repeated teardown passes.

## Risks introduced

No runtime risk. The earlier commit remains in branch history with its over-specific interpretation; this follow-up correction is committed on top and the current report is authoritative.

## Decision record (embedded)

- **Decision:** Stop assigning a named function to `0x001201c8` until relocation-aware mapping is available.
- **Context:** RPCS3 reports a relocated module location while the available static disassembly uses raw ELF section addresses.
- **Options considered:** Retain the syscall-stub guess; map it to the nearest raw offset; or record only the exact fatal thread and null target.
- **Why this option was chosen:** The available evidence supports the thread name, null target, and reported offset, but does not support a named-function mapping.
- **Tradeoffs accepted:** The failure remains less localized until the exact emulator load map or symbolization is available.

## Next recommended step

Capture or derive the fake-SPRX runtime load bias and symbol map, then rerun a minimal controlled shutdown with phase markers around worker exit and join. Do not advance to hardware testing until audio-enabled repeated unloads pass.
