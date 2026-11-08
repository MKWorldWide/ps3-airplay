# AIR PS3 documentation

Welcome! Start with what you want to do; the detailed development history is preserved separately.

## Install and listen

- **[Download the latest release](https://github.com/MKWorldWide/ps3-airplay/releases/latest)** — the PKG and SHA-256 checksums.
- **[Installation and recovery](INSTALLATION.md)** — install, connect from an iPhone, **START to exit**, verify files, uninstall or roll back.
- **[v1.0.0 release notes](RELEASE_NOTES-v1.0.0.md)** — what ships, what doesn't, and what was tested on **Evilnat 4.93 CFW**.

## Build, verify, and contribute

- **[Build from Source](BUILD_FROM_SOURCE.md)** — Linux x86_64 / Lima, pinned SDKs, build commands and independent verification.
- [Development setup](DEV_SETUP.md) — SDK layout and experimental development paths.
- [Detailed build pipeline](BUILD_PIPELINE.md) — exact release gate and historical PS3 toolchain findings.
- [Contributing](../CONTRIBUTING.md) — help improve the receiver or report tests.
- [Test matrix](TEST_MATRIX.md) — measured behavior and hardware evidence boundaries.
- [Architecture](ARCHITECTURE.md) — protocol, codec, PCM, and PS3 components.

## Follow the project

- [Roadmap](ROADMAP.md) — metadata, album artwork, controller controls, display settings, and future development.
- [Versioning](VERSIONING.md) — why 1.0.0 maps to SFO 01.00.
- [Risks and safety](RISKS.md) — compatibility limits and testing cautions.
- [Public release process](PUBLIC_RELEASE_PLAN.md) and [release gate](RELEASE_GATE_1.md) — how a validated PKG reaches GitHub.

## Engineering archive

Want to trace an old bug, review a failed build, or inspect the reasoning behind a fix?

- **[Hardware and technical reports](reports/README.md)** — selected release reports, then the full historical report collection.
- **[Decision records](decisions/README.md)** — project decisions and dated engineering history.
- [Research notes](research/README.md) — protocol and SDK reference work.

These archives include earlier experiments, unsuccessful candidates, and historical measurements. **They are not the current installation or build instructions.** The public v1.0.0 release and the linked guides above are the starting points.
