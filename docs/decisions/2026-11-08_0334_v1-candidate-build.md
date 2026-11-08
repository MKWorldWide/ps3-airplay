# v1.0.0 candidate build and SFO verification

**Timestamp:** 2026-11-08 03:34 EST

## Scope of the pass

Built the committed AIR PS3 v1.0.0 candidate in Lima, verified the generated PKG and extracted SFO, and updated release status documentation. No PS3, RPCS3, tag, or GitHub Release was changed.

## Why this pass was needed

The release gate required exact-source package evidence and explicit confirmation that the package carries the requested app and SFO versions.

## Files changed

- `docs/RELEASE_GATE_1.md`
- `docs/TEST_MATRIX.md`
- `docs/memory.md`
- `docs/scratchpad.md`
- `docs/reports/air_ps3_v1_0_0_candidate_build.md`
- `docs/decisions/2026-11-08_0334_v1-candidate-build.md`

## Architecture / logic changes

No runtime or build logic changed. The exact-source release workflow produced a type-1 debug package from `release.json`; extracted SFO fields were independently checked with the project validator.

## Runtime behavior changes

None. A build artifact was produced; no device was launched.

## Validation performed

- Existing Lima full release gate: host ASan/UBSan suite, PPU crypto/receiver build, CellOS ELF validation, package extraction/envelope, metadata, and artwork checks passed.
- `python3 scripts/validate-ps3-sfo.py work/builds/1.0.0/PARAM.SFO 01.00 'AIR PS3'`
- `python3 scripts/test-release-config.py`
- `python3 scripts/test-release-evidence.py`
- `python3 scripts/verify-release-evidence.py --package ... --evidence ... --expected-source f3da352809f81daac5e3b61a35d7fc18362bf1cc --expected-sha256 ef8f0afdacef560e9f5825fa9539247f3eb17e7791282aa6790f086fa8b1288e`
- Package size/hash independently matched Lima evidence.

## Immediate results

- App version is `1.0.0`.
- Extracted SFO `APP_VER` and `VERSION` are both `01.00`.
- AIRPLAY02 PKG is 3,906,640 bytes; SHA-256 `ef8f0afdacef560e9f5825fa9539247f3eb17e7791282aa6790f086fa8b1288e`.
- Candidate is technically validated at the host/build boundary, not at a fresh runtime boundary.

## Known limitations / follow-ups

The built source commit is on the preserved feature branch rather than current `origin/main`; integrate and rebuild from the exact main tip before release staging. Physical hardware behavior remains untested by this pass.

## Risks introduced

No code risk introduced. Treating the branch artifact as the public release would violate the exact-main-tip gate; release staging remains blocked.

## Decision record

### Decision: Preserve app version `1.0.0` and use SFO `01.00`

- **Context:** User requires version `1.0.0` and SFO `1.00`; this project's SFO field convention is fixed-width `XX.YY`.
- **Options considered:** Store `1.00` literally or use the project's two-digit fixed-width form `01.00`.
- **Choice:** Use `01.00`, consistent with the checked-in mapping and PS3 SFO validation.
- **Tradeoff:** The stored form has a leading zero, while retaining the requested numeric value and established metadata format.

### Decision: Do not stage or publish this candidate

- **Context:** Exact source build passed, but its source revision is not the current main tip.
- **Options considered:** Stage a release from the feature branch or leave the candidate local until main integration and a rebuild.
- **Choice:** Keep the gate blocked and preserve the candidate for review.
- **Tradeoff:** Release is deferred; exact main-tip reproducibility and release policy remain intact.

## Next recommended step

Integrate the reviewed source into `main`, then rerun the complete release gate from the resulting exact `origin/main` commit.
