# AIR PS3 roadmap

## Current state — v1.0.0 released

- Version source is `release.json`: app `1.0.0`, SFO `01.00`, title ID `AIRPLAY02`, XMB title `AIR PS3`, RAOP service name `AirPS3`.
- Gate 0 is user-accepted from the v0.2.8 dev-unit run: about 45 minutes, four XMB cycles, no reported PCM underflows, ALAC failures, RTP queue drops, concealed/skipped frames, or screen-presentation failures. The user accepted a brief ~45 ms service interval on XMB resume as a known hitch. This is summarized user evidence, not a new raw-log audit.
- The maintainer reports v1.0.0 is running beautifully on the development PS3; this is qualitative feedback, not a measured soak or a list of individually verified features. The exact release source and published artifact are documented in `RELEASE_GATE_1.md` and the publication report.
- v1.0.0 is publicly released at [GitHub Releases](https://github.com/MKWorldWide/ps3-airplay/releases/tag/v1.0.0). Its release tag points to the exact Lima-validated source. The publication workflow passed all artifact checks. No PS3 was changed during publication.

## Release path

1. Keep release identity centralized in `release.json`; app `1.0.0` maps to SFO `01.00`, next patch `1.0.1` to `01.01`.
2. Run the pinned Lima host/sanitizer, PPU, ELF, SFO, artwork, extraction, and debug-PKG checks from a clean committed source revision.
3. Produce `release-evidence.json` and `SHA256SUMS.txt` alongside the ignored local package; independently verify source SHA, version, package envelope, and payload records.
4. Resolve privacy and distribution gates. If any check is unresolved or fails, do not create a tag or GitHub Release.
5. Completed for v1.0.0: the draft was validated and published through the manual GitHub workflow. Both validation and publish jobs have `contents: write`; validation uses the token for GET-only asset inspection, then the separate publish job changes release visibility.
6. Keep hardware installation separate. This task does not install, restart, or alter either PS3.

## Priority order

1. Completed: publish the verified app `1.0.0` / SFO `01.00` candidate after the manual workflow passed.
2. Completed: verify the public release assets and PKG checksum against the release evidence.
3. Retain the ~45 ms XMB-resume service interval as a known polish item. Gate 0 is user accepted; no fresh PS3 operation occurred during this release work.
4. Keep metadata, DualShock, and display-setting phases outside this release task; they belong to future roadmap work.

## Scope boundaries

- The product is an iPhone-sent AirPlay 1 / RAOP receiver; Spotify authentication/playback stays on the phone.
- A successful host test, package validation, or healthy log counter is not proof of visible display output, audible quality, or stable hardware behavior.
- Resident SPRX loading and in-game audio mixing are a separate feasibility effort. This roadmap does not promise universal game audio coexistence.
- Raw logs, local build outputs, device identifiers, network data, and local machine details stay out of public files. Use aggregate, provenance-labelled evidence only.
