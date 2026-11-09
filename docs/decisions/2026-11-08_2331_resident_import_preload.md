# Preload resident import modules in the harness

**Timestamp (EDT):** 2026-11-08 23:31

## Scope of the pass

Updated the diagnostic app to preload the network system modules imported by
the resident before asking GameOS to load the SPRX. Added matching cleanup,
rebuilt the `00.02` package, installed it, and captured the target log.

## Why this pass was needed

The resident imports symbols from `sys_net` and `cellNetCtl`. The harness had
not loaded those modules before calling `sys_prx_load_module`, while the
resident's own module-start routine cannot load them until after PRX import
resolution.

## Files changed

- `experimental/resident/harness/CMakeLists.txt`
- `experimental/resident/harness/source/main.c`
- `docs/experimental/RESIDENT_OVERNIGHT_REPORT.md`
- `docs/memory.md`
- `docs/lessons-learned.md`
- `docs/scratchpad.md`
- `docs/decisions/2026-11-08_2331_resident_import_preload.md`

The package, executable, and raw console log are local ignored artifacts under
`build/resident-hardware/`.

## Architecture / logic changes

The harness links `cellSysmodule` and calls `cellSysmoduleLoadModule` for
`CELL_SYSMODULE_NET` and `CELL_SYSMODULE_NETCTL` before recording its PRX-cycle
baseline. It tracks successful references and unloads them in reverse order
on normal completion or each harness error exit. The resident implementation
and production SPRX did not change.

## Runtime behavior changes

The harness now logs network-module preload and unload return codes. On target,
both preloads returned zero, raising the pre-cycle module count from the prior
run's baseline to 11. The resident load still failed with `0x80010007`
(`ENOEXEC`) at cycle one. Both harness-held module references and foreground
audio were released successfully afterward.

## Validation performed

- Ran `scripts/build-lima-resident-hardware-harness.sh` in Lima using pinned
  PS3DK v0.27.0 and Mbed TLS 3.6.7; host protocol/audio/sanitizer checks passed.
- Rebuilt PPU resident and harness. `prx-gen check` passed and confirmed the
  same production SPRX structure and five import libraries.
- Extracted the package and inspected the SFO: title `RPRX90002`,
  `APP_VER=00.02`, `VERSION=00.02`.
- Confirmed the package has one signed SPRX. Installed package EBOOT SHA-256
  matches the package-extracted EBOOT; installed SPRX SHA-256 remains
  `9fae9ed955c023aca2077654625510e80766f899c9eaa4998cad4af5f1588817`.
- Installed and launched the updated diagnostic title. The fresh log records
  successful NET and NETCTL preload and unload, then `LOAD_FAILED=-2147418105`
  and `HARNESS_FAIL=1`.
- Package SHA-256:
  `c96644d84c3e7b7b365f9a1a84342b624dd111c6557c9055ab49ed0a174431b1`.
- Fresh console log SHA-256:
  `c74c095a9dd0dd3e8a7769ad0fa36168fe1cc8f74fdf37ffd04a9dbb6288f1f9`.

## Immediate results

The harness dependency-order bug is fixed and verified on target. Missing
`sys_net`/`cellNetCtl` preloads are not a sufficient explanation for the
resident loader rejection. The app and package run correctly at SFO `00.02`,
but the resident still does not reach `module_start`.

## Known limitations / follow-ups

The remaining `ENOEXEC` source is unknown. SELF signing/key compatibility,
resident PRX header/relocation details, and target import resolution remain
possible areas; this run does not distinguish them. No resident worker, RAOP
session, PCM submission, audible output, or module unload was tested. Hardware
readiness remains BLOCKED. Isolate the loader with a minimal signed PRX before
changing signing or relocation behavior.

## Risks introduced

The change affects only the separately titled diagnostic harness. The network
modules remain held while the resident is under test, and harness-owned
references are explicitly released. No production app, release artifact,
standalone receiver, or Cobra setting was changed.

## Decision record (embedded)

### Preload the resident's import modules in the harness

- **Decision:** Load `CELL_SYSMODULE_NET` and `CELL_SYSMODULE_NETCTL` before
  calling `sys_prx_load_module`.
- **Context:** The resident imports from these libraries but its own module
  startup loads them only after the loader has resolved the import table.
- **Options considered:** Assume GameOS will implicitly load them; preload them
  from the harness and test the result.
- **Why chosen:** Explicit preload makes import availability testable and keeps
  module loading order correct for a user-loaded PRX.
- **Tradeoffs accepted:** The harness now has explicit system-module lifetime
  management and must release references on all exits.

### Keep the loader failure open after the experiment

- **Decision:** Do not label the resident fixed when the retry still returns
  `ENOEXEC`.
- **Context:** Both dependency preloads and their unloads return zero, while the
  resident load result is unchanged.
- **Options considered:** Attribute the error to the missing preloads; test the
  exact loader result and retain an unresolved status.
- **Why chosen:** Target runtime evidence disproves the preloads as a sufficient
  fix, and the syscall does not name the deeper cause.
- **Tradeoffs accepted:** The candidate remains blocked pending a minimal-PRX
  isolation test.

## Next recommended step

Build a minimal signed PRX with only management exports and no external imports,
then test load/start/stop/unload on the target. If that passes, add imports in
small groups to isolate the incompatible library or relocation pattern.
