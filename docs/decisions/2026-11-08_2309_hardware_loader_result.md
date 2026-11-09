# Hardware harness confirms SFO and exposes SPRX loader failure

## Timestamp in EDT

2026-11-08 23:09 EDT

## Scope of the pass

Install the corrected `RPRX90002` package with SFO `00.01`, launch its
verbose harness on the development PS3, retrieve its log read-only, and
compare the installed module hash with the fresh signed build.

## Why this pass was needed

The first package's SFO version was `0.1.0`; the user reported the same
launch error as an earlier invalid-version case and directed that the SFO
version be `00.01`. The corrected build needed an on-console check.

## Files changed

- `docs/experimental/RESIDENT_OVERNIGHT_REPORT.md`
- `docs/memory.md`
- `docs/lessons-learned.md`
- `docs/scratchpad.md`
- `docs/decisions/2026-11-08_2309_hardware_loader_result.md`

## Architecture / logic changes

No source or PRX architecture changed in this pass. The corrected package
replaces the earlier diagnostic app under the same isolated title ID. The
prior four-file install was backed up to ignored local build output before
removal. No release app, Cobra option, or automatic loading setting changed.

## Runtime behavior changes

The `RPRX90002` app starts and writes its harness log with the corrected SFO.
Its first resident module load is rejected by the target loader before the
PRX reaches `module_start`; the harness stops immediately and closes its own
foreground audio port.

## Validation performed

- Extracted package SFO — PASS: `TITLE_ID=RPRX90002`, `APP_VER=00.01`,
  `VERSION=00.01`.
- Transferred corrected PKG — PASS: console copy SHA-256 matched local
  `54d5d299c6831a18e7c0474aac3577a8a02de443a7a3b98e03f64d242b9010b9`.
- Installed SFO retrieved from the console — PASS: both version fields are
  `00.01`.
- Installed signed SPRX — PASS: SHA-256
  `9fae9ed955c023aca2077654625510e80766f899c9eaa4998cad4af5f1588817`,
  exactly matching the fresh production build.
- Harness launch/log — PASS: `HARNESS_START`, module baseline, audio port
  initialization, and first-cycle events were recorded.
- `sys_prx_load_module` — FAIL: `LOAD_FAILED value=-2147418105`, or
  `0x80010007`; the pinned PS3DK errno mapping identifies this as `ENOEXEC`.
- Harness audio cleanup — PASS: port stop/close and `cellAudioQuit` returned
  zero.
- 50-cycle resident lifecycle, RAOP receive, resident PCM, audible output,
  and Cobra compatibility — UNTESTED; the PRX did not load.

## Immediate results

The SFO launch issue is resolved enough for the test application to start and
write a log. The actual hardware SPRX loader gate now fails with `ENOEXEC`.
The harness stops on cycle one and performs its own audio cleanup. The full
console log is retained locally under ignored build output; no raw log or
network identifiers are committed.

## Known limitations / follow-ups

- `ENOEXEC` does not identify the precise subcause. Candidate areas include
  SELF/SPRX format, signing/key-table expectations, and PRX loader constraints;
  none is confirmed as the cause yet.
- The signed SPRX has passed `prx-gen check`, but that validates structure and
  import metadata, not Evilnat/Cobra acceptance.
- The harness did not observe a post-unload module count because the loader
  rejected the module before an ID was returned.
- The installed title remains available for reproducing the error. The
  RPCS3-only fake SPRX was not substituted.

## Risks introduced

- The diagnostic title remains installed with a loader-rejected module package.
  It has no autoload or Cobra plugin configuration and is not active as a
  resident plugin.
- Repeated install or launch attempts will not resolve the load failure without
  an evidenced format/signing correction.

## Decision record (embedded)

### Decision: stop after first module-load failure

- **Context:** The harness returned `ENOEXEC` on cycle one; its repeated
  start/stop test cannot meaningfully proceed if the module cannot load.
- **Options considered:** Keep retrying all 50 cycles; treat package install as
  success; stop and preserve the loader error for targeted investigation.
- **Why this option was chosen:** Repeating an identical rejected load adds no
  evidence and does not test shutdown; preserving the exact failure makes the
  next diagnostic actionable.
- **Tradeoffs accepted:** Repeated lifecycle evidence remains unavailable
  until the first load succeeds.

### Decision: retain the signed SPRX artifact and not substitute the fake

- **Context:** The explicit hardware gate requires the production-compatible
  signed SPRX; the RPCS3 fake has a different container type.
- **Options considered:** Repackage the emulator fake to see whether CFW loads
  it; investigate the signed SPRX rejection; stop hardware attempts.
- **Why this option was chosen:** The current evidence points to the signed
  module loader boundary, and swapping container types would change the test
  conditions without authorization.
- **Tradeoffs accepted:** Hardware runtime testing is blocked pending a
  reviewed, evidence-based loader fix.

## Next recommended step

Trace `0x80010007` from the target `sys_prx_load_module` path against the
PS3DK-generated SELF/SPRX metadata, import library versions, and Evilnat
4.93 loader expectations. Produce a targeted candidate and structural proof
before another hardware install attempt.
