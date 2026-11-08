# AIR PS3 installation and recovery

AIR PS3 is distributed as a type-1 debug PKG for compatible PS3 homebrew/CFW environments. Retail firmware compatibility is not promised. The receiver is a foreground application; it does not install a resident SPRX, edit boot-plugin configuration, patch LV2, or mix with game audio.

## Install

1. Download the PKG and `SHA256SUMS.txt` from the same GitHub Release. Verify the package hash before copying it to a USB device or the console's package staging directory.
2. On a compatible CFW console, open the XMB Package Manager and install the PKG using the normal package-install path.
3. Launch **AIR PS3** from Game. Choose the receiver in the sender's AirPlay output menu.
4. To exit normally, press controller **START** and wait for XMB. The accepted v0.2.8 run had a maximum audio-service interval near 45 ms during XMB resume; the metric is retained as a known hitch and does not establish its audible or visual effect.

Installation and launch depend on CFW configuration and are not supported on retail firmware. Do not modify boot plugins or system video settings to use the standalone receiver.

## Remove or roll back

Use XMB Package Manager to delete **AIR PS3 / AIRPLAY02**. To roll back, install the known-good earlier package for the same title ID after removing the current version if the package manager requests it. Keep the package hash and version with any rollback copy; never assume a similarly named file has the same contents.

The receiver's diagnostic log is stored at `/dev_hdd0/tmp/ps3-airplay.log`. It may contain useful device-runtime details; do not publish a raw log. Save a private copy before removing it if investigating a failure, then share only a reviewed, redacted summary.

## Recovery

If the application is responsive, use START to return to XMB. If controller input is unavailable but the console remains reachable, use the established webMAN exit path. Avoid repeated launch attempts after a hang. If the console becomes fully unresponsive, use the console's normal power-button recovery procedure and report whether it beeps or requires a forced shutdown; do not edit boot-plugin settings as a recovery experiment.

This guide describes the standalone package only. The experimental resident module is separate and is not part of the release.
