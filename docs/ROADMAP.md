# AIR PS3 roadmap

## Current state — v1.0.0 release validation

- Version source is `release.json`: app `1.0.0`, SFO `01.00`, title ID `AIRPLAY02`, XMB title `AIR PS3`, RAOP service name `AirPS3`.
- Gate 0 is user-accepted from the v0.2.8 dev-unit run: about 45 minutes, four XMB cycles, no reported PCM underflows, ALAC failures, RTP queue drops, concealed/skipped frames, or screen-presentation failures. The user accepted a brief ~45 ms service interval on XMB resume as a known hitch. This is summarized user evidence, not a new raw-log audit.
- The maintainer reports v1.0.0 is running beautifully on the development PS3; this is qualitative feedback, not a measured soak or a list of individually verified features. The exact-main build checkpoint and its package evidence are documented in the release report; the candidate must be rebuilt after final source changes and checked against the exact release tag.
- The release candidate is integrated into `main`. Draft staging and the manual publication workflow are the remaining release steps. Neither PS3 is changed as part of release preparation.

## Release path

1. Keep release identity centralized in `release.json`; app `1.0.0` maps to SFO `01.00`, next patch `1.0.1` to `01.01`.
2. Run the pinned Lima host/sanitizer, PPU, ELF, SFO, artwork, extraction, and debug-PKG checks from a clean committed source revision.
3. Produce `release-evidence.json` and `SHA256SUMS.txt` alongside the ignored local package; independently verify source SHA, version, package envelope, and payload records.
4. Resolve privacy and distribution gates. If any check is unresolved or fails, do not create a tag or GitHub Release.
5. Stage a draft only after all checks pass; publish separately through the manual GitHub workflow. The publish job alone has `contents: write`.
6. Keep hardware installation separate. This task does not install, restart, or alter either PS3.

## Priority order

1. Stage the verified app `1.0.0` / SFO `01.00` candidate as a draft and publish it through the manual workflow after its validation succeeds.
2. Verify the resulting GitHub Release assets and hashes against the local release evidence.
3. Retain the ~45 ms XMB-resume service interval as a known polish item. Gate 0 is user accepted; no fresh PS3 installation occurs during this work.
4. Start metadata, DualShock, and display-setting phases independently after the focused stable receiver release.

## Scope boundaries

- The product is an iPhone-sent AirPlay 1 / RAOP receiver; Spotify authentication/playback stays on the phone.
- A successful host test, package validation, or healthy log counter is not proof of visible display output, audible quality, or stable hardware behavior.
- Resident SPRX loading and in-game audio mixing are a separate feasibility effort. This roadmap does not promise universal game audio coexistence.
- Raw logs, local build outputs, device identifiers, network data, and local machine details stay out of public files. Use aggregate, provenance-labelled evidence only.
