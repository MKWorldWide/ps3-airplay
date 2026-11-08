# AIR PS3 roadmap

## Current state — v1.0.0 release candidate

- Version source is `release.json`: app `1.0.0`, SFO `01.00`, title ID `AIRPLAY02`, XMB title `AIR PS3`, RAOP service name `AirPS3`.
- Gate 0 is user-accepted from the v0.2.8 dev-unit run: about 45 minutes, four XMB cycles, no reported PCM underflows, ALAC failures, RTP queue drops, concealed/skipped frames, or screen-presentation failures. The user accepted a brief ~45 ms service interval on XMB resume as a known hitch. This is summarized user evidence, not a new raw-log audit.
- The v1.0.0 candidate must be rebuilt from one exact source revision with the full Lima gate. A release tag/package is not published yet. Current source is on the existing feature branch and should be integrated to the intended release branch before publication.
- Public release distribution remains gated on review of third-party notices, artwork permission/provenance, exact debug-PKG contents, automated candidate verification, and release-branch integration. Neither PS3 is changed as part of release preparation.

## Release path

1. Keep release identity centralized in `release.json`; app `1.0.0` maps to SFO `01.00`, next patch `1.0.1` to `01.01`.
2. Run the pinned Lima host/sanitizer, PPU, ELF, SFO, artwork, extraction, and debug-PKG checks from a clean committed source revision.
3. Produce `release-evidence.json` and `SHA256SUMS.txt` alongside the ignored local package; independently verify source SHA, version, package envelope, and payload records.
4. Resolve privacy and distribution gates. If any check is unresolved or fails, do not create a tag or GitHub Release.
5. Stage a draft only after all checks pass; publish separately through the manual GitHub workflow. The publish job alone has `contents: write`.
6. Keep hardware installation separate. This task does not install, restart, or alter either PS3.

## Priority order

1. Complete the exact-source Lima build for app `1.0.0` / SFO `01.00` and record package, ELF, and extracted-payload evidence.
2. Finish the privacy, license/notice, package-format, and release-branch reviews. Keep the tag and GitHub Release absent while any gate is open.
3. Stage a reviewable draft only after every Gate 1 check passes; publish through the manual workflow.
4. Retain the ~45 ms XMB-resume service interval as a known polish item. Gate 0 is user accepted; no fresh PS3 installation occurs during this work.
5. Start metadata, DualShock, and display-setting phases independently after the focused stable receiver release.

## Scope boundaries

- The product is an iPhone-sent AirPlay 1 / RAOP receiver; Spotify authentication/playback stays on the phone.
- A successful host test, package validation, or healthy log counter is not proof of visible display output, audible quality, or stable hardware behavior.
- Resident SPRX loading and in-game audio mixing are a separate feasibility effort. This roadmap does not promise universal game audio coexistence.
- Raw logs, local build outputs, device identifiers, network data, and local machine details stay out of public files. Use aggregate, provenance-labelled evidence only.
