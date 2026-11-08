# AIR PS3 installation and recovery

AIR PS3 is distributed as a type-1 debug PKG for compatible PS3 homebrew/CFW environments. Retail firmware compatibility is not promised. The receiver is a foreground application; it does not install a resident SPRX, edit boot-plugin configuration, patch LV2, or mix with game audio.

## Install

1. Download [AIR PS3 v1.0.0](https://github.com/MKWorldWide/ps3-airplay/releases/latest) and its `SHA256SUMS.txt` from the same release. Verify the package hash before copying it to a USB device or the console's package staging directory. Use `shasum -a 256 -c SHA256SUMS.txt` on macOS or `sha256sum -c SHA256SUMS.txt` on Linux.
2. On a compatible CFW console, open the XMB Package Manager and install the PKG using the normal package-install path.
3. Launch **AIR PS3** from Game. On the iPhone, open the AirPlay output menu and choose **AirPS3**. Audio apps such as Spotify continue to run on the phone; the PS3 is an AirPlay receiver, not a Spotify Connect client.
4. **Press START to exit AIR PS3 and return to the XMB.** Wait for the XMB before relaunching. The source handles START as an application exit; prior hardware evidence records START returning to XMB while idle.

Earlier v0.2.8 testing recorded a maximum audio-service interval near 45 ms during XMB resume. It remains a known measured hitch; the measurement alone does not establish its audible or visual effect. The maintainer's v1.0.0 hardware report is qualitative and did not include a duration or specific control checks.

Installation and launch depend on CFW configuration and are not supported on retail firmware. Do not modify boot plugins or system video settings to use the standalone receiver.

## Remove or roll back

Use XMB Package Manager to delete **AIR PS3 / AIRPLAY02**. To roll back, install an earlier known-good package for the same title ID if you have one; XMB may require removing the current version first. Keep the package hash and version with any rollback copy; never assume a similarly named file has the same contents.

The receiver's diagnostic log is stored at `/dev_hdd0/tmp/ps3-airplay.log`. It may contain useful device-runtime details; do not publish a raw log. Save a private copy before removing it if investigating a failure, then share only a reviewed, redacted summary.

## Recovery

If the application is responsive, **press START to return to the XMB**. If controller input is unavailable but the console remains reachable, use the established webMAN exit path. Avoid repeated launch attempts after a hang. If the console becomes fully unresponsive, use the console's normal power-button recovery procedure; do not edit boot-plugin settings as a recovery experiment.

This guide describes the standalone package only. The experimental resident module is separate and is not part of the release.
