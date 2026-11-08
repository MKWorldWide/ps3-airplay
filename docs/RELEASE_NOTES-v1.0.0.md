# AIR PS3 v1.0.0

**AirPlay audio for your PlayStation 3.**

AIR PS3 is a standalone AirPlay 1 receiver: select **AirPS3** on your iPhone and play music through your PS3. Your music app stays on the phone. Press **START** on the controller to exit to the XMB.

This is our first public release. It's been tested on a PlayStation 3 running **Evilnat 4.93 CFW**, with Cobra 8.5 observed. The package is intended for compatible homebrew/CFW setups, not retail firmware.

## In this release

- AirPlay 1 discovery, RAOP connection handling, and encrypted ALAC audio playback.
- Bounded buffering, recovery from missing RTP packets, and asynchronous diagnostics.
- AIR PS3 XMB title and artwork; the receiver appears as **AirPS3**.
- A foreground app with START-button exit.
- A type-1 debug/homebrew PKG, independently checked against its build evidence.

**Download:** [AIR PS3 v1.0.0 on GitHub](https://github.com/MKWorldWide/ps3-airplay/releases/tag/v1.0.0) · [Installation guide](INSTALLATION.md)

## Known limitations

Returning from the PS-button menu may introduce a short audio-service hitch; previous development testing measured approximately 45 ms at its worst. This release does not support AirPlay video, Spotify Connect, background game-audio mixing, or resident plugin playback.

The maintainer reports that v1.0.0 is working well on the development console. Detailed previous hardware observations and their limits are retained in the [test matrix](TEST_MATRIX.md); a subjective report is not a substitute for broad compatibility testing.

## For builders

The published package is app **1.0.0**, SFO **01.00**, Title ID **AIRPLAY02**. Its source, SHA-256 checksum, package inspection results, and third-party notices are available in the release.

Want to verify or modify the source? Follow [Build from Source](BUILD_FROM_SOURCE.md) for the Linux x86_64 / Lima + PS3DK v0.27.0 / PSL1GHT toolchain, tests, and PKG checks.

Thanks for giving this old console another trick.
