# PS3 AirPlay

An open source AirPlay 1 (RAOP) receiver project for PlayStation 3 homebrew, developed by **MKWorldWide**. The goal is to let an iPhone send audio to a PS3, starting with a standalone receiver and exploring resident plugin support separately.

**Community contributions and testing are welcome.** See [Contributing](CONTRIBUTING.md) to help with code, documentation, or carefully reported hardware tests.

## Project status

This is experimental software. The maintainer reports an earlier AIRPLAY02 launch and five-second diagnostic tone, but the exact package identity is unknown. Later PS3 installs of AIRPLAY02 v0.1.2 and v0.1.3 returned XMB error `80028F21`. RPCS3 rejected local v0.1.4–v0.1.6 candidates before app startup. The v01.01 reference and a fresh PS3DK-linked candidate share the expected CellOS ELF layout, while v0.1.6's legacy-linker ELF does not; whether this explains the RPCS3 error remains unproven. The corrected pipeline is documented in [Build Pipeline](docs/BUILD_PIPELINE.md); v0.1.7 has not yet passed its RPCS3 launch gate. Visible output, clean exit, iPhone connection, and AirPlay music remain unverified. See the [roadmap](docs/ROADMAP.md) and [versioning policy](docs/VERSIONING.md).

The resident PRX currently receives and decodes supported audio packets for diagnostics; it does not output PCM. Game audio coexistence is a separate feasibility question. No universal in-game mixing capability is claimed.

**Demonstration target:** PS3 with Evilnat 4.93 CFW; Cobra 8.5 was observed in the system status. The console model was not recorded. The reported tone and discovery came from an earlier package of unconfirmed version; they do not validate versions 0.1.2–0.1.4.

## What is in the repository

- Bounded RAOP discovery, RTSP session, RTP transport, crypto, ALAC decoding, PCM buffering, and diagnostic logging components.
- A standalone receiver prototype and an experimental resident PRX candidate.
- A maintainer-provided diagnostic background image in `assets/`, used by the standalone screen experiment.
- Host tests and build scripts, plus a test matrix that distinguishes host evidence from hardware validation.

## Documentation

- [Documentation index](docs/README.md)
- [Architecture and project boundaries](docs/ARCHITECTURE.md)
- [Development setup](docs/DEV_SETUP.md)
- [Standalone build and package pipeline](docs/BUILD_PIPELINE.md)
- [Roadmap](docs/ROADMAP.md)
- [Test matrix](docs/TEST_MATRIX.md)
- [Risks and safety gates](docs/RISKS.md)
- [Third-party notices](THIRD_PARTY_NOTICES.md)
- [Contributing](CONTRIBUTING.md)

## Getting involved

Issues and pull requests are welcome. Useful contributions include reproducible host tests, protocol interoperability research, build improvements, documentation, and results from controlled PS3/iPhone testing. Please share only information you are comfortable making public. Remove console/account identifiers, network addresses, credentials, and unredacted diagnostic logs from reports.

Start with an issue for substantial changes so the scope and test evidence are clear. Do not test experimental packages on a device you do not own or have permission to use. See [the contributor guide](CONTRIBUTING.md) for the reporting checklist.

## License

Project code is released under the [MIT License](LICENSE), subject to the third-party terms and notices in [THIRD_PARTY_NOTICES.md](THIRD_PARTY_NOTICES.md).

## References

- [azarucha/sys-airplay](https://github.com/azarucha/sys-airplay) — AirPlay 1 protocol reference for Nintendo Switch; its platform code is not a direct PS3 port.
- [ps3dev/PSL1GHT](https://github.com/ps3dev/PSL1GHT) and [ps3dev/ps3toolchain](https://github.com/ps3dev/ps3toolchain) — PS3 homebrew development resources.
