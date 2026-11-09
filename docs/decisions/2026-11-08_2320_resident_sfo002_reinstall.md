# Resident harness SFO 00.02 reinstall

**Timestamp (EDT):** 2026-11-08 23:20

## Scope of the pass

Advanced the isolated `RPRX90002` harness SFO version fields to `00.02`,
rebuilt and validated the package, installed it on the development PS3, and
retrieved the fresh harness log over read-only FTP.

## Why this pass was needed

The user requested correction and reinstall with SFO version `00.02` after the
previous `00.01` hardware run still failed at resident module load.

## Files changed

- `experimental/resident/harness/sfo.xml`
- `docs/experimental/RESIDENT_OVERNIGHT_REPORT.md`
- `docs/memory.md`
- `docs/lessons-learned.md`
- `docs/scratchpad.md`
- `docs/decisions/2026-11-08_2320_resident_sfo002_reinstall.md`

Generated packages, binaries, and console logs are local ignored artifacts in
`build/resident-hardware/` and are not included in Git.

## Architecture / logic changes

No PRX logic, signing method, imports, or module loader code changed. Only the
two SFO version values were advanced. The hardware package continues to embed
the production signed SPRX; the emulator fake SPRX was not substituted.

## Runtime behavior changes

The installed test title now advertises SFO `APP_VER=00.02` and
`VERSION=00.02`. The harness app launches. The resident SPRX still fails to
load on its first cycle with `0x80010007` (`ENOEXEC`), before `module_start`.
The harness then stops and closes its foreground audio resources successfully.

## Validation performed

- Rebuilt with `scripts/build-lima-resident-hardware-harness.sh` in the pinned
  Linux PS3DK v0.27.0 environment.
- Host crypto, RAOP/RTSP, encrypted RTP/ALAC, PCM, resampling, and sanitizer
  checks in the build script passed.
- `prx-gen check` passed: one PT_LOAD, 6,300 relocations, one management export,
  and the expected five import libraries.
- Extracted the package with the PS3DK `pkg` tool; `sfo inspect --json`
  confirmed `TITLE_ID=RPRX90002`, `APP_VER=00.02`, and `VERSION=00.02`.
- Confirmed the package contains exactly one signed SPRX, byte-identical to the
  freshly built production SPRX.
- Installed the package and independently parsed the console-side SFO; it
  matches `RPRX90002`, `00.02`, `00.02`.
- Retrieved the installed SPRX and confirmed SHA-256
  `9fae9ed955c023aca2077654625510e80766f899c9eaa4998cad4af5f1588817`.
- Launched the harness and retrieved a fresh run log: run ID `1258700164`,
  `LOAD_FAILED=-2147418105`, `HARNESS_FAIL=1`; audio port stop, close, and quit
  each returned zero.
- Package SHA-256:
  `c070380805764adfef94d10ede277c240e84db6379bfd8c81ccbcbb92370fa94`.
- Console log SHA-256:
  `8501b5c10846ac2a12ee2a9d97621682dae59ff5c57c85a6d651e7f5e985784f`.

## Immediate results

- The application launch/SFO version gate passes at `00.02`.
- The production SPRX loader gate remains failed at cycle one.
- No resident worker, receiver networking, resident PCM, audible output, or
  unload cycle was exercised.

## Known limitations / follow-ups

The `ENOEXEC` subcause is not established. The current evidence is consistent
with a target SELF signing/key compatibility issue, a PRX format or relocation
issue, or target import resolution. The console error alone does not distinguish
these possibilities. Hardware readiness remains BLOCKED. Next isolate the
toolchain with the smallest safe diagnostic PRX and compare its loader result
before changing production signing or relocation behavior.

## Risks introduced

The update only changes a separately titled diagnostic harness. No production
title, published package, physical standalone receiver, or Cobra setting was
changed. A future speculative signing or format change could make the module
less compatible and must be validated against a minimal test module first.

## Decision record (embedded)

### Keep the production signed SPRX in the hardware package

- **Decision:** Retain the freshly signed production SPRX in `RPRX90002`.
- **Context:** The production module was rejected, while the `.fake.sprx` is
  intended for RPCS3 and is not valid evidence for this hardware gate.
- **Options considered:** Substitute the RPCS3 fake container; retain and
  diagnose the exact signed hardware candidate.
- **Why chosen:** Preserve the user's hardware-test constraint and ensure the
  test exercises the actual candidate.
- **Tradeoffs accepted:** The current target rejection remains visible and
  blocks lifecycle/audio tests.

### Do not guess at the loader fix

- **Decision:** Record the repeat `ENOEXEC` and defer binary/signing changes
  until the failing layer is isolated.
- **Context:** Package SFO, install, and launch now pass, but the same module
  load result persists.
- **Options considered:** Change SELF flags, imports, or relocation format by
  guess; build a minimal isolation module and compare behavior.
- **Why chosen:** The syscall error does not identify the subcause, so guessing
  could create a misleading candidate or damage format compatibility.
- **Tradeoffs accepted:** The resident remains not hardware-ready.

## Next recommended step

Build and validate a minimal signed diagnostic PRX with the same PS3DK toolchain
and compare its load/start/stop/unload result on target. Use the result to
separate signing/container acceptance from resident-specific format, relocation,
and import failures before the next resident package install.
