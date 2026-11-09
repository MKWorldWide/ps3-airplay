# OpenPS3FTP reference artifact assessment

## Timestamp in EDT

2026-11-09 01:40 EDT

## Scope of the pass

Inspected public PS3 SPRX projects and added the OpenPS3FTP v5.4 source/release artifact as an offline comparison point. No source code, package, console, Cobra setting, or harness behavior was changed.

## Why this pass was needed

The resident and two diagnostic SPRXs fail in the hardware harness with `0x80010007` (`ENOEXEC`). A useful external reference needs a published binary, corresponding source, a reproducible build recipe, and a loader context that can be compared honestly.

## Files changed

- `docs/experimental/RESIDENT_SPRX_REFERENCE_RESEARCH_2026-11-09.md`
- `docs/memory.md`
- `docs/lessons-learned.md`
- `docs/scratchpad.md`
- `docs/decisions/2026-11-09_0140_openps3ftp-reference.md`

The downloaded release artifact is in ignored `build/reference/` and is not part of the repository change.

## Architecture / logic changes

No runtime or architecture changes. Research now distinguishes a VSH plugin loaded by CFW/HEN plugin management from a module intended for an application to load through `sys_prx_load_module`.

## Runtime behavior changes

None.

## Validation performed

- Inspected the OpenPS3FTP v5.4 GitHub release metadata and release asset list.
- Read the public `plugin/CMakeLists.txt`, `plugin/main.c`, and README build/deployment description.
- Downloaded the 125,296-byte release SPRX to ignored local research storage; checked its `SCE\0` prefix and outer flags word (`0x00010001`). Calculated SHA-256 `4010de387cf97b2559d5d29ca702db1f558ede033603866642608a9a7a352d92`.
- Compared the flags word with the existing production resident SPRX. This is an outer-header comparison only; the encrypted module was not decrypted and the signature was not authenticated.
- Reviewed PS3 Presence and sky-hook documentation to reject them as safe, known-working app-loader comparators: one requires Cobra startup integration and has limited hardware testing; the other is a skeleton.
- No console or RPCS3 run was performed.

## Immediate results

- OpenPS3FTP is a stronger public source-plus-binary example than the previously considered skeleton. Its source documents the PS3DK `ps3_add_sprx` pipeline and explicit module start/stop exports.
- Its plugin is intentionally VSH-hosted and uses VSH-facing functionality. It is not a drop-in reference for the AIR harness's app-context loader call.
- Its outer flags word matching the production resident does not establish loader compatibility or explain `ENOEXEC`.
- No known-good app-context reference SPRX was established in this pass.

## Known limitations / follow-ups

- The OpenPS3FTP release SPRX was not installed or executed; release availability and source documentation are not independent hardware proof.
- Full SELF authentication, decryption, import table comparison, and inner ELF inspection were not performed.
- Continue looking for a small app-context SPRX with public source and credible hardware load evidence, or produce a minimal reference with a documented alternate pipeline. Keep it separate from resident behavior.

## Risks introduced

No runtime risk. The ignored reference artifact must remain excluded from staging. Public documentation correctly states the toolchain and file size/hash but does not disclose local usernames, network addresses, or console identifiers.

## Decision record (embedded)

### Decision 1

- **Decision:** Keep OpenPS3FTP v5.4 as an offline structural/source reference only.
- **Context:** It has a public release SPRX and source using PS3DK, but the plugin runs in VSH and uses VSH APIs.
- **Options considered:** Install or manually load it as a hardware test; adapt the AIR harness to load it; inspect it offline and continue hunting.
- **Why chosen:** Offline inspection provides comparison value without changing the console or conflating VSH plugin loading with application module loading.
- **Tradeoffs accepted:** It cannot itself prove that the AIR resident should load through `sys_prx_load_module`.

### Decision 2

- **Decision:** Do not use PS3 Presence or sky-hook as the next hardware loader probe.
- **Context:** PS3 Presence modifies Cobra startup integration and states its hardware testing is limited; sky-hook is a placeholder skeleton.
- **Options considered:** Download and install either project; use their source as a loose structural reference; reject them for this loader experiment.
- **Why chosen:** The stated diagnostic boundary forbids persistent plugin activation, and neither candidate has sufficient evidence to isolate the loader failure.
- **Tradeoffs accepted:** This leaves fewer candidate artifacts but avoids unsafe or inconclusive tests.

## Next recommended step

Search for a released app-context SPRX with public source and explicit `sys_prx_load_module` load evidence; if none is credible, build a minimal app-context control module using an independently documented PRX pipeline and compare its complete module metadata before a manual test.
