# Install and use AIR PS3

**AIR PS3 v1.0.0** is an AirPlay 1 audio receiver for a PlayStation 3 running compatible custom firmware. The release was tested on **Evilnat 4.93 CFW**, with **Cobra 8.5** observed. It's a foreground app, not a background music plugin.

## Download and check the package

1. Open the [latest AIR PS3 release](https://github.com/MKWorldWide/ps3-airplay/releases/latest).
2. Download `ps3-airplay-receiver-1.0.0.pkg` and `SHA256SUMS.txt` into the same folder.
3. Verify the package before installing.

**macOS**
```sh
grep '  ps3-airplay-receiver-1.0.0.pkg$' SHA256SUMS.txt | shasum -a 256 -c -
```

**Linux**
```sh
grep '  ps3-airplay-receiver-1.0.0.pkg$' SHA256SUMS.txt | sha256sum -c -
```

The command should print `ps3-airplay-receiver-1.0.0.pkg: OK`. The full checksum file also includes `release-evidence.json`; download that third file if you want to validate the entire manifest with a plain `sha256sum -c SHA256SUMS.txt`. The [release page](https://github.com/MKWorldWide/ps3-airplay/releases/tag/v1.0.0) includes the package, build evidence, and license notices.

The package is a type-1 debug/homebrew PKG, intended for compatible **CFW** installations; retail firmware is not supported. No system firmware updates or boot plugins are required by AIR PS3 itself.

## Install

1. Copy the verified PKG to a USB device or your usual package staging location.
2. On your compatible CFW console, open **XMB → Package Manager** and install it.
3. From the XMB **Game** category, launch **AIR PS3**.
4. On the iPhone, open your AirPlay audio output list and select **AirPS3**. The iPhone and PS3 should be on a network where they can discover each other.
5. Play audio from the phone. Spotify or another audio app continues to run **on the iPhone**; AIR PS3 receives its AirPlay 1 audio stream.

**To exit AIR PS3, press START on your PS3 controller.** Wait until you are back at the XMB before relaunching.

## What to expect

- AIR PS3 uses the approved black-and-silver wave artwork with the AirPlay symbol.
- The PS button opens the system menu; a brief hitch on returning to the app has been observed in earlier hardware testing.
- This release does **not** mirror video, run Spotify Connect, mix with game audio, or load a resident background-audio SPRX.
- Compatibility beyond the tested Evilnat 4.93 CFW environment is not guaranteed.

See [testing and known boundaries](TEST_MATRIX.md) for more technical details.

## Remove, roll back, or recover

To remove the app, delete **AIR PS3 (AIRPLAY02)** from the XMB using the standard game/app removal controls. Keep any known-good earlier package and its hash if you want to roll back; you may need to remove the current installation before installing an older build.

If the app is responsive, **press START to exit**. If controller input stops working but the console is still reachable, an existing, authorized webMAN exit method may work. If the whole console is unresponsive, use its normal power-button recovery procedure rather than modifying boot plugins.

The local diagnostic log is stored at `/dev_hdd0/tmp/ps3-airplay.log`. It can contain device/runtime details. Save a private copy for troubleshooting and redact it before sharing excerpts publicly.

## Building from source

For a Linux x86_64 build using PS3DK v0.27.0, PSL1GHT, Mbed TLS and the checked release tag, follow [Build from Source](BUILD_FROM_SOURCE.md). Build/package validation does not guarantee compatibility with a physical PS3.
