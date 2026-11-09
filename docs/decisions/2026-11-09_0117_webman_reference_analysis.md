# webMAN MOD SPRX reference analysis and 00.04 loader result

**Timestamp:** 2026-11-09 01:17 EST (America/New_York)

## 1. Scope of the pass

Downloaded and inspected the public webMAN MOD 1.47.50 installer package and source build settings offline. Matched the released full SPRX SELF header against the AIR PS3 resident and the two 00.04 signed probes. Recorded the fresh 00.04 hardware log result in the report and working notes.

## 2. Why this pass was needed

The prior loader analysis identified SELF revision `0x000A` as a difference from the PS3DK resident (`0x0001`) but had not tested whether that field alone was sufficient. The 00.04 harness run provides the controlled result; a public working plugin supplies a reference artifact and build configuration.

## 3. Files changed

- `docs/experimental/RESIDENT_SPRX_REFERENCE_RESEARCH_2026-11-09.md`
- `docs/experimental/RESIDENT_OVERNIGHT_REPORT.md`
- `docs/memory.md`
- `docs/lessons-learned.md`
- `docs/scratchpad.md`
- `docs/decisions/2026-11-09_0117_webman_reference_analysis.md`

## 4. Architecture / logic changes

No application code or runtime behavior changed. The research note records the release source/tag, artifact hashes, SELF header fields, upstream signing/build settings, plugin import context, and boundaries of the comparison. It also records that both `0x000A` probes failed in the same loader path, so the revision alone is not a sufficient correction.

## 5. Runtime behavior changes

None. The public package was never installed or run. The previously installed diagnostic `00.04` harness was launched and its log retrieved; all probe attempts failed before module start, and cleanup returned module count to baseline.

## 6. Validation performed

- Retrieved the official GitHub release asset and verified its size and SHA-256.
- Extracted the package to ignored local research output and enumerated the four webFTP SPRX variants.
- Parsed the full SPRX SELF header and independently parsed the resident and 00.04 probe headers.
- Read the source tag’s `_Make_all.sh`, `_Make_pkg.sh`, `makefile`, and full-build flags.
- Retrieved the fresh 00.04 console log read-only; confirmed six `ENOEXEC` results, unchanged module counts after failure, successful audio/network cleanup, and baseline module count 9.
- Confirmed downloaded package, extracted SPRXs, helper scripts, and console logs remain ignored under `build/`. The complete staged diff was reviewed; targeted searches found no private paths, network identifiers, credentials, raw logs, or generated artifacts. No `gitleaks` or `trufflehog` scanner is installed.

## 7. Immediate results

- PASS — public reference package and source tag identified; full SPRX and package hashes recorded.
- PASS — 00.04 harness cleanup returned the module count from 11 to 9.
- FAIL — all three loads of each diagnostic probe returned `0x80010007` (`ENOEXEC`).
- BLOCKED — 50-cycle resident lifecycle test did not pass its probe gate and did not run.
- UNTESTED — resident startup, RAOP reception, PCM submission, and audible output.
- UNTESTED — reference plugin installation or execution; analysis remained offline.

## 8. Known limitations / follow-ups

The SELF header is readable, but this pass did not decrypt or cryptographically validate the embedded ELF. The reference plugin uses a different toolchain and VSH/Cobra plugin context. The load failure root cause remains unknown. The reported hard power-button shutdown is not correlated with the captured 00.04 run, which logged cleanup at 157 ms.

Next step: check availability of a compatible Sony CELL SDK PRX toolchain, build a tiny no-import probe with the reference link/sign pipeline, and only then compare or test it through the isolated loader harness. Any further physical-console experiment remains manual and diagnostic.

## 9. Risks introduced

No runtime or source changes. The downloaded package and helper artifacts are local ignored research files. Do not install the reference webMAN package or alter Cobra/boot-plugin settings as part of this loader comparison.

## 10. Decision record (embedded)

### Decision 1: Use the official GitHub release package as the binary source

- **Context:** Brewology linked the project and listed a 4.93 build, but its direct download endpoint returned HTTP 403.
- **Options considered:** Stop at public source/build scripts; retry Brewology; use the project’s official GitHub release asset.
- **Why chosen:** The official tag provides a reproducible release asset and matching source tag. No mirror or console-side copy was needed.
- **Tradeoffs accepted:** The Brewology listing is a cross-reference, not the origin of the analyzed bytes.

### Decision 2: Treat SELF revision `0x000A` as insufficient, not irrelevant

- **Context:** Both scetool-signed 00.04 probes use `0x000A`, yet all six load attempts failed.
- **Options considered:** Treat the revision mismatch as fixed; reject it as irrelevant; keep it as one dimension in a larger loader-contract investigation.
- **Why chosen:** The hardware result falsifies revision alone as a sufficient fix but does not reproduce the reference’s complete toolchain, signing metadata, or VSH/Cobra loader context.
- **Tradeoffs accepted:** Root cause remains open; no production resident changes are justified by this result alone.

### Decision 3: Do not install the reference plugin

- **Context:** webMAN is a persistent VSH/Cobra-facing plugin suite, while the task is to inspect its build/container characteristics.
- **Options considered:** Install and activate it on the development console; extract and analyze offline.
- **Why chosen:** Offline extraction answers the structural research question without changing console plugin state.
- **Tradeoffs accepted:** Runtime acceptance of the reference SPRX was not independently retested in this task.

## 11. Next recommended step

Check for a compatible Sony CELL SDK PRX toolchain and build a minimal no-import module with the same PRX link and scetool signing steps. Keep the resident unchanged; use an isolated diagnostic title only after the artifact is structurally reviewed.
