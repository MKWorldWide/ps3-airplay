# Integrate and validate the v1.0.0 main release candidate

**Timestamp:** 2026-11-08 04:17 EST

## Scope of the pass

Fast-forwarded reviewed release work into `main`, ran the full release pipeline from that exact source revision in the existing Lima VM, recorded package evidence, reconciled GitHub issues, and updated the prepublication gate and test matrix. No application code or console state changed.

## Why this pass was needed

The prior package evidence named a pre-attribution source revision and was not eligible for release staging. The public repository also needed an accurate release state and milestone closure based only on documented hardware evidence.

## Files changed

- `docs/RELEASE_GATE_1.md`
- `docs/TEST_MATRIX.md`
- `docs/README.md`
- `docs/RELEASE_NOTES-v1.0.0.md`
- `docs/ROADMAP.md`
- `docs/PUBLIC_RELEASE_PLAN.md`
- `docs/memory.md`
- `docs/scratchpad.md`
- `docs/lessons-learned.md`
- `docs/reports/air_ps3_v1_0_0_main_build_checkpoint.md`
- `docs/decisions/2026-11-08_0417_v1-exact-main-build.md`

## Architecture / logic changes

No runtime or build-system changes. Release documentation now distinguishes pre-attribution artifacts from the package bound to the integrated `main` SHA. The test matrix preserves host/build versus hardware evidence boundaries. GitHub issues #1 and #2 were closed against historical launch/tone/idle-exit observations; #3 through #6 remain open with missing acceptance evidence described in comments.

## Runtime behavior changes

None. The exact-main package was built and inspected only. No PS3 or RPCS3 operation was performed.

## Validation performed

- Fast-forward merge from the reviewed feature branch; pushed the resulting `main` without rewriting history.
- Ran `scripts/release-lima-standalone-receiver.sh` from an archive stamped with the exact main SHA.
- Version mapping and evidence verifier tests passed.
- Host ASan/UBSan, crypto, RTSP/RTP, ALAC, PCM/resampler, timing, logger, and display-resume tests passed.
- PPU crypto/receiver build and CellOS PPC64 ELF validation passed.
- Artwork, SFO identity, package extraction, relative paths, type-1 debug envelope, SHA256SUMS, and evidence checks passed.
- Reconciled issue tracker statuses and comments.

## Immediate results

- Source SHA: `38f13ba2b3255352a9957160c3d075e5426cd069`.
- Package: `ps3-airplay-receiver-1.0.0.pkg`, 3,906,640 bytes, SHA-256 `1244be0bb7ef1e6c8565a2a9f06e82bf7d43ad7d10f570fa1ad30dc394bee259`.
- SFO `APP_VER`/`VERSION` are `01.00`, title ID is `AIRPLAY02`.
- Issues #1 and #2 are closed; #3–#6 remain open.

## Known limitations / follow-ups

The build is not physical-hardware validation. The user report for v1.0.0 remains qualitative and is not tied to this exact-main package hash. Issue #3 repeated RTSP sessions, #4 30-minute audible playback/reconnect, #5 resident module feasibility, and #6 two-hour/Wi-Fi/game coexistence remain open. A final documentation commit changes the source SHA; repeat the full release build from that exact final main revision before staging.

## Risks introduced

None to application runtime. Release links are in place but should be verified after the GitHub Release is published. Artifact publication must wait until exact assets and all release surfaces are reviewed.

## Decision record

### Decision: Use a fast-forward integration and rebuild the exact main revision

- **Context:** Main was an ancestor of the reviewed release branch; prior evidence used an old commit SHA.
- **Options considered:** Rewrite the package evidence to the new SHA or rerun the pinned build.
- **Choice:** Fast-forward main and run the complete build from a source archive stamped with the exact new SHA.
- **Tradeoffs accepted:** Lima emulation is slower, but the package and evidence are bound to the release commit rather than inferred from tree similarity.

### Decision: Close only milestones with explicit hardware evidence

- **Context:** Historical records describe app launch, a five-second diagnostic tone, and START returning to XMB while idle; later v1.0.0 feedback is qualitative.
- **Options considered:** Leave every issue open or close only the bounded launch/tone milestones.
- **Choice:** Close #1 and #2 with comments limiting the evidence to the historical build; keep #3–#6 open.
- **Tradeoffs accepted:** The project has fewer stale baseline tasks without overstating current release or AirPlay acceptance.

## Next recommended step

Finish the exact candidate and public-surface privacy review, commit and push the release evidence, rebuild from that final main SHA, then create a draft and publish only through the validated GitHub workflow.
