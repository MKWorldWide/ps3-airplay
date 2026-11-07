# Native PS3DK Launch Control

This is a small, separate XMB application used to check whether an app built through PS3DK's native runtime can launch, initialize video, accept START input, and return to XMB. It does not link the AirPlay receiver, audio, networking, or the resident SPRX.

It identifies itself as v0.1.0, opens `/dev_hdd0/tmp/ps3-airplay-native-launch.log`, initializes the controller before graphics, configures the console's current video mode, and cycles six full-screen colors. Press START to exit, or let the 20-second frame limit return to XMB. RSX label and flip polling have finite limits. A log event records initialization stages and exit; `DISPLAY_INIT_READY` means setup completed, not that pixels were visibly confirmed on HDMI.

The app uses a separate title ID (`AIRTEST01`), so its package will not replace AIRPLAY02. It does not change HDMI settings, firmware, LV2 state, or boot plugin configuration. No package should be transferred or installed without the exact artifact hash and a separate approval.

## Build

Build in the existing Lima guest using its PS3DK v0.27.0 source and staged PS3DEV toolchain:

```sh
cmake -S prototype/native-ps3dk-launch-control \
  -B build/native-ps3dk-launch-control \
  -DCMAKE_TOOLCHAIN_FILE="$PS3DK_SOURCE/cmake/ps3-ppu-toolchain.cmake"
cmake --build build/native-ps3dk-launch-control --parallel 4
```

The CMake post-build path emits the ELF/SELF forms and an NPDRM PKG. Inspect the PARAM.SFO title ID/version, content ID, embedded EBOOT, package listing, and SHA-256 before any hardware transfer.

## Hardware acceptance

For one approved launch attempt, record separately: XMB launch result; whether a stable color screen appears; whether START exits to XMB; whether the app log is freshly written and ends with `APP_STOP`; and whether the 20-second fallback exits cleanly. A failure here tests the native PS3DK launch/display path, not AirPlay audio or receiver compatibility.

The display setup follows the API sequence in PS3DK's `hello-ppu-cellgcm-clear` sample. That sample and its build/runtime details are documented at [FirebirdTA01/PS3DK](https://github.com/FirebirdTA01/PS3DK).
