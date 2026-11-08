# AIR PS3

![AIR PS3 artwork](assets/air-ps3/ICON0.PNG)

AIR PS3 is a standalone **AirPlay 1 audio receiver** for PlayStation 3 homebrew environments. Choose the PS3 from an iPhone's AirPlay output menu to send audio to the console. Spotify and other audio apps continue running on the iPhone; AIR PS3 is not Spotify Connect and does not run Spotify on the PS3.

**Current release: v1.0.0** · SFO `01.00` · Title ID `AIRPLAY02` · AirPlay name `AirPS3`

## Download

- [Download AIR PS3 v1.0.0](https://github.com/MKWorldWide/ps3-airplay/releases/latest)
- [Download the PKG directly](https://github.com/MKWorldWide/ps3-airplay/releases/download/v1.0.0/ps3-airplay-receiver-1.0.0.pkg)
- [Verify the release checksums](https://github.com/MKWorldWide/ps3-airplay/releases/download/v1.0.0/SHA256SUMS.txt)

The package is a **type-1 debug/homebrew PKG** for a compatible CFW homebrew environment. It was tested on the demonstration system running Evilnat 4.93 CFW; Cobra 8.5 was observed. Installation on retail firmware is not promised.

## Install and use

1. Download the PKG and `SHA256SUMS.txt` from the same release. Verify the checksum before installing. On macOS use `shasum -a 256 -c SHA256SUMS.txt`; on Linux use `sha256sum -c SHA256SUMS.txt`.
2. Install the PKG using the XMB Package Manager.
3. Launch **AIR PS3** from Game, then choose **AirPS3** from the iPhone's AirPlay output menu.
4. **Press START to exit AIR PS3 and return to the XMB.**

See the [installation, removal, rollback, and recovery guide](docs/INSTALLATION.md) for full steps.

## What is tested

The maintainer reports that v1.0.0 is running beautifully on the development PS3. No duration, log measurements, or specific playback/control checks were included with that report. Earlier v0.2.8 evidence records a user-accepted run of about 45 minutes with four XMB open/close cycles; it also records a brief service interval near 45 ms during resume. See the [test matrix](docs/TEST_MATRIX.md) for evidence boundaries and the [release notes](docs/RELEASE_NOTES-v1.0.0.md) for scope.

## Scope and limitations

- AIR PS3 is a foreground receiver. **Resident SPRX loading and in-game audio mixing are separate experiments**; universal game-audio mixing is not claimed.
- AirPlay video/mirroring, Spotify Connect, and retail-firmware installation are not included.
- The experimental resident module is not installed by this package and must not be treated as part of the standalone release.

## Contribute and test

**Community contributions and testing are welcome.** Useful contributions include protocol and host tests, build improvements, documentation, and carefully reported PS3/iPhone test results. Read [CONTRIBUTING.md](CONTRIBUTING.md) and the [documentation index](docs/README.md) first. Share only details intended for public review; do not post credentials, personal or machine paths, network/device identifiers, or raw diagnostic logs.

## License

Project code is licensed under [MIT](LICENSE), subject to the third-party terms in [THIRD_PARTY_NOTICES.md](THIRD_PARTY_NOTICES.md). Package assets and required license texts are provided with the release.

### ❤️ Support Development

AIR PS3 is free, community-developed homebrew software.

If you’re enjoying it and want to support future improvements, new features, and other MKWorldWide experiments, consider becoming a [GitHub Sponsor](https://github.com/sponsors/EsKaye).

Your support helps keep the experiments alive, the hardware running, and the ideas flowing.

**Sponsorship is completely optional. AIR PS3 remains free to download and use.**
