# Public stable release and feature plan

**Status checked:** 2026-11-08
**Published release:** AIRPLAY02 app `1.0.0` / SFO `01.00`, XMB title `AIR PS3`, RAOP name `AirPS3`
**Repository:** `MKWorldWide/ps3-airplay` is public, `main` is the default branch, and [v1.0.0 is published](https://github.com/MKWorldWide/ps3-airplay/releases/tag/v1.0.0). This document retains the publication gates for reproducibility and future versions.

This release stays focused on the standalone AirPlay 1 receiver. Now Playing, extra controller transport controls, and visual settings remain later work.

## Gate 0 — User accepted v0.2.8 hardware run

The user reports the installed AIRPLAY02 v0.2.8 / SFO `00.48` development unit ran for about 45 minutes and completed four XMB open/close cycles. The latest summarized log showed no PCM underflows, ALAC failures, RTP queue drops, concealed/skipped frames, or screen-presentation failures. A maximum audio-service interval of about 45 ms occurred during XMB resume; the user accepted that brief resume hitch for Gate 0. This is user-supplied summarized evidence, not a fresh independent raw-log audit. The audio service interval is not a subjective sound-quality measure, and callbacks/counters do not prove every visual or audible detail.

Earlier v0.2.8 evidence remains in its engineering report and version history, but the newer 45-minute user summary supersedes the old 10m41s note for Gate 0 duration. Never combine separate test units or runs. The known earlier multi-second stall/full-RTP-queue event remains a risk requiring bounded host fault-injection evidence; do not infer it away from a clean soak summary.

## Gate 1 — v1.0.0 publication (completed)

### Identity and build

- App semantic version `1.0.0`; PS3 SFO `APP_VER` and `VERSION` `01.00`.
- Next package mapping: `1.0.1` / `01.01`, then `1.0.2` / `01.02`; no leading-zero semantic versions such as `1.0.01`.
- Preserve `AIRPLAY02`, `AIR PS3`, `AirPS3`, and the user-approved artwork.
- `release.json` is the version source of truth. Lima builds must pass the host/sanitizer suite, PPU/ELF checks, SFO identity, package extraction, asset comparison, and debug-PKG envelope validation. A generated local candidate is not a public release or a PS3 deployment.
- Candidate metadata records full source SHA, pinned toolchain, package size/hash, ELF hash, SFO identity, and extracted payload hashes. GitHub automation consumes only that exact candidate and fails closed on mismatch.

### Distribution and safety

- The current PKG is a type-1 debug/homebrew package. Installation is intended for a compatible CFW homebrew environment; retail firmware installation is not promised.
- The receiver is standalone. No boot plugin, resident SPRX, LV2 patch, firmware change, or game-audio mixing is included.
- The supplied artwork is public at the maintainer's explicit direction. Third-party software/key notices must accompany source and package distribution; the AirPort Express RAOP compatibility key is not a project credential and is separately attributed in `THIRD_PARTY_NOTICES.md`.
- Do not include private device/network identifiers, raw logs, local build folders, or private agent instructions in release assets.
- Installation, normal START exit, removal, rollback, and recovery are documented in `INSTALLATION.md`.

### Publication gate

The prepublication gate and completed release evidence are recorded in `RELEASE_GATE_1.md`. AIR PS3 v1.0.0 is now published at [GitHub Releases](https://github.com/MKWorldWide/ps3-airplay/releases/tag/v1.0.0). The draft-staging helper verified candidate evidence before creating the tag/draft; manual workflow `42468858833` passed exact source, asset, checksum, and package-evidence validation before publication. Both workflow jobs use `contents: write`: validation performs GET-only access to draft release assets, and the separate publish job changes the draft state.

## First public stable receiver acceptance — v1.0.0

- Completed: rebuild v1.0.0 from one committed source revision and record hashes/evidence.
- Completed: verify exact artifact identity and the debug package's CFW-only scope; keep host and hardware results separate.
- Preserve Gate 0 as user-accepted evidence. The maintainer now reports v1.0.0 is running beautifully on the development PS3; no duration, measurements, log, or individual playback/control checks accompanied that report. Do not extend it into a soak-test claim.
- Retain the accepted resume hitch as a known polish item; avoid claiming flawless XMB composition or zero latency.
- Confirmed release notes, installation/removal/recovery steps, exact notices, artwork authorization, and the published asset set before publication. Post-publication PKG and `SHA256SUMS.txt` download verification passed.

## Phase 2 — Now Playing metadata and artwork

- First capture actual metadata behavior from each target sender; distinguish Spotify/iPhone behavior from protocol capability in general.
- Support bounded title, artist, album, and artwork parsing with UTF-8/text sanitization, missing-field fallbacks, maximum image dimensions/bytes, decode limits, and cache eviction.
- Handle stream changes and missing/corrupt artwork without blocking RTSP, RTP, or PCM service. Keep metadata work asynchronous and memory bounded.
- Add fixtures for supported metadata formats and sender-specific tests. Demonstrate audio remains stable under frequent metadata/artwork changes.
- Acceptance: correct displayed metadata/art for tested senders, graceful fallback for unsupported senders, no audio stalls or unbounded memory growth.

## Phase 3 — DualShock playback controls

- Keep local DualShock volume/mute distinct from the existing AirPlay `SET_PARAMETER` gain path; define gain bounds, mute semantics, ramping, and visible feedback.
- Investigate play/pause/previous/next only where sender controls are available and honored. Expose sender capability or document when a control is unavailable; do not promise Spotify behavior without testing it.
- Define control mapping, focus/navigation, button-conflict policy, idle/streaming behavior, and a way to reach exit/settings.
- Acceptance: controls work on the physical controller in repeated idle and streaming sessions, with no regressions to PS-menu, resume, START exit, or audio servicing.

## Phase 4 — Display settings and ambient mode

- Provide selectable full-screen dimming: off, 50%, and 95%; allow adjustment toward 90% if 95% proves too dark on hardware. Apply dimming consistently to artwork, text, controls, and background.
- Offer adaptive album-derived color, solid backgrounds, and user-provided wallpapers; include invalid-image fallback and strict decoded-image/memory bounds.
- Offer three font choices, seven layout positions, and small/medium/large artwork sizing.
- Offer transitions off/fast/smooth and a configurable idle timeout for ambient Now Playing view. Visual effects must not interrupt audio.
- Persist validated settings with safe defaults and recovery from corrupt or unsupported data. Controller navigation must remain operable.
- Respect GameOS/VSH ownership: never draw over system UI or assume the app can choose XMB composition. Test PS-menu/resume after each renderer change.
- Acceptance: all choices persist, invalid input falls back safely, and repeated XMB cycles restore a visible responsive screen without audio interruption.

## Phase 5 — Package and GitHub Release checklist

Before publishing a downloadable release from the already-public repository:

1. Build from a clean exact commit in the pinned Lima toolchain; run host sanitizer/regression suite and PPU/ELF/SELF/PKG/SFO checks.
2. Extract the exact package and verify title ID, version, content ID, EBOOT, icon, background, and artwork; publish PKG and ELF hashes.
3. Check code/package version alignment and create release notes describing tested hardware/software scope and known limitations.
4. Include install, exit, uninstall, rollback, and recovery instructions plus the tested CFW/PKG format requirements.
5. Audit all release files/assets for secrets, local paths, personal/device/network identifiers, raw logs, and unlicensed content. Inspect the full diff and exact release assets; preserve public-safe architecture, generic firmware, and demonstration details where useful.
6. Review all reachable branches, tags, history, LFS/submodules, issues/comments, pull requests, discussions, wiki, workflows, packages, and linked release surfaces for private information. A clean current tree does not sanitize old history or other GitHub surfaces.
7. Verify the remote/repository/default branch and release target. Keep existing repository name, public visibility, and `main` identity unchanged. Release publication is a separate action from roadmap planning.

## Exclusions

AirPlay video/mirroring, Spotify credentials or Spotify Connect, resident SPRX installation, firmware changes, boot-plugin changes, LV2 modifications, and universal in-game audio mixing are outside this receiver roadmap. Game coexistence requires a separate feasibility decision after standalone playback is stable.
