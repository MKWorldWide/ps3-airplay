# Correct the standalone NPDRM package

**Timestamp:** 2026-11-07 00:12 EST

## Scope of the pass

Corrected the PKG creation method to bind `EBOOT.BIN` to the package content ID, included PSL1GHT's bundled default XMB icon, and recorded the observed launch failure and diagnostic evidence.

## Why this pass was needed

The first installed receiver failed to start with PS3 error `80029530`. The exact installed EBOOT matched the package payload, but no application startup log was created. The error is classified as an NPDRM format error, with a content-ID mismatch documented as a cause of a start failure.

## Files changed

- `scripts/package-lima-standalone-receiver.sh`
- `prototype/standalone-receiver/README.md`
- `docs/TEST_MATRIX.md`
- `docs/ROADMAP.md`
- `docs/lessons-learned.md`
- `docs/memory.md`
- `docs/scratchpad.md`
- `docs/reports/2026-11-07_first_hardware_test.md`
- `docs/decisions/2026-11-07_0012_fix_npdrm_package.md`
- `docs/decisions/README.md`

## Architecture / logic changes

The prior packaging script used generic `fself` without embedding the `CONTENTID` that the PKG builder used. PSL1GHT's CEX package rules call `make_self_npdrm` with the ELF, output filename, and exact content ID; the script now follows that contract and copies the SDK's default `ICON0.PNG` into the package root. Package extraction validation requires and compares `ICON0.PNG` alongside the existing SFO/EBOOT checks.

The first hardware failure is recorded as likely NPDRM content-ID mismatch, not confirmed until a corrected package launches. The PS3 error table documents `80029530` as `SCE_NP_DRM_ERROR_FORMAT` and notes content-ID mismatch as a possible start failure: [PS3 Developer Wiki error codes](https://www.psdevwiki.com/ps3/Error_Codes). PSL1GHT's own [PPU package rules](https://github.com/ps3dev/PSL1GHT/blob/master/ppu_rules) establish the correct `make_self_npdrm` flow.

## Runtime behavior changes

No receiver runtime code changed. A newly built package will have an NPDRM EBOOT bound to the matching package identifier and include a default XMB icon. The currently installed and staged package remains the previous version until a replacement is authorized and tested.

## Validation performed

- Read the installed `EBOOT.BIN` over FTP and compared it byte-for-byte with the EBOOT extracted from the first PKG; they match.
- Checked the expected log path after the failed launch; no `ps3-airplay.log` was present.
- Confirmed the local PSL1GHT rules call `make_self_npdrm <ELF> <EBOOT.BIN> <CONTENTID>` and use the SDK default icon.
- Rebuilt the PKG in Lima with `make_self_npdrm`; package listing and extraction passed, including `ICON0.PNG`, `PARAM.SFO`, and NPDRM SELF `EBOOT.BIN`.
- SFO metadata remained title `PS3 AirPlay`, ID `AIRPLAY02`, category `HG`, minimum system version `01.8000`; PKG content ID is `UP0001-AIRPLAY02_00-0000000000000000`.
- Corrected candidate size is 416,240 bytes; SHA-256 is `f1b17d07a05679030143e607c74b545592e92ad521135b860bc2aca677160cf2`.
- No console files were changed during this diagnostic pass.

## Immediate results

- The failure was not caused by FTP corruption.
- `80029530` is consistent with an NPDRM format/content-ID problem.
- The app did not produce a startup log; this suggests failure before application logging, but cannot rule out a file-open failure.

## Known limitations / follow-ups

- Obtain approval to replace the staged PKG with the corrected candidate; the currently installed title also needs reinstall/update to test it.
- The old PKG is still installed and staged on the console.
- Present the corrected package name, digest, destination, and overwrite conflict before replacing the staged PKG.
- Installation, application launch, tone audibility, logging, iPhone discovery, and AirPlay playback remain unverified for the corrected package.

## Risks introduced

- No new console risk; this pass changes only local packaging logic and documentation.
- A package can pass structural validation and still fail hardware launch, so runtime remains a separate gate.

## Decision record

### Decision: Use the PSL1GHT NPDRM signer with the package's exact content ID

- **Context:** The first package installed but failed at launch with error `80029530`; installed EBOOT matched the transferred artifact.
- **Options considered:** Change audio/runtime code; rebuild only the PKG envelope; generate `EBOOT.BIN` using the PS3 toolchain's NPDRM signer bound to the package content ID.
- **Why this option was chosen:** PSL1GHT's own CEX package rules require `make_self_npdrm` and supply the package content ID, aligning the EBOOT, PARAM.SFO title ID, and PKG.
- **Tradeoffs accepted:** A new package digest requires review and explicit approval before replacing the staged file.

## Next recommended step

Provide the corrected package name, digest, destination, and existing-file conflict and obtain approval to replace the staged PKG. Then guide XMB reinstall and observe launch behavior.
