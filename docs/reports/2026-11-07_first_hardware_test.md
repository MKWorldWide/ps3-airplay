# PS3 hardware test — audio works, display and AirPlay need diagnosis

**Date:** 2026-11-07 (America/New_York)

## User-reported outcome

- The earlier package first failed to start with `80029530`. A corrected NPDRM package was then used for a hardware run; the user reports the five-second diagnostic tone played. The exact installed package version was not separately confirmed.
- The screen remained black and the user could not return to XMB through the app. The console was shut down using a long power-button press.
- An iPhone listed `ps3-airplay-diagnostic`; selecting it reported that it could not connect. No RAOP session or music playback was confirmed.
- The current-run diagnostic log has not been retrieved. Preserve it for inspection if present at `/dev_hdd0/tmp/ps3-airplay.log` after the console next starts.

## Evidence summary

| Check | Status | Evidence |
|---|---|---|
| Lima PPU build | PASS | Screen-enabled receiver ELF built with the PS3DK/PSL1GHT toolchain |
| Screen-enabled package | PASS, host-side | Package listing and extraction include NPDRM EBOOT, SFO, XMB icon, and exact 1280x720 RGB background |
| Package on console | NOT STAGED | New screen-enabled package awaits approval before replacing the existing staged PKG |
| Audible diagnostic tone | PARTIAL PASS | User reports hearing the five-second tone on hardware |
| Display | FAIL | User reports a black screen while the app was active |
| XMB return | FAIL on previous build | User needed a long power-button shutdown; new START-button cleanup path is not yet built into a hardware-tested package |
| iPhone discovery | PARTIAL PASS | User saw `ps3-airplay-diagnostic` in the iPhone's receiver list |
| iPhone connection / RTSP | FAIL | iPhone reported it could not connect; no successful RAOP session confirmed |
| Current-run diagnostic log | NOT RETRIEVED | Console was powered off after the run; inspect the capped log before further protocol changes |
| AirPlay music playback | NOT TESTED | No successful connection or music playback reported |
| Resident PRX / game audio mix | NOT TESTED | No SPRX was installed or loaded; game coexistence remains a separate feasibility gate |

## Screen and exit follow-up

Source inspection showed that the receiver previously initialized audio and networking but did not initialize RSX video or poll controller input. The new local candidate follows PSL1GHT's framebuffer setup pattern, scales the public maintainer-provided background to the active display resolution, overlays `PRESS START TO EXIT`, and polls the controller START button to perform normal cleanup and return from the app. A PS-button XMB exit continues through the system utility callback.

This is a source-based explanation for the blank screen and missing app exit path, not proof of the exact console failure. The new candidate compiles and its package extracts correctly in Lima; RSX output, controller polling, and return to XMB require another hardware run. The app remains a foreground standalone receiver; no in-game overlay or universal game audio mixing is claimed.

## Package and privacy notes

- The first failed package and the corrected NPDRM-only package are documented in the preceding deployment record.
- The new candidate contains `USRDIR/background.rgb`, produced from `assets/diagnostic-background.jpg`; generated RGB data and the PKG remain in ignored build output.
- The public source JPEG was re-encoded without EXIF metadata. Its reuse terms are unspecified and documented separately from the MIT-licensed code.
- Candidate version `01.01`: 3,222,640 bytes; SHA-256 `7824a4b15e4757afad3213a3f55cd3bea816b33795403da12bfd7ede29f93db4`. Do not replace the existing staged PKG until the user approves this exact artifact.
- Do not publish private network addresses, unique console identifiers, credentials, or raw diagnostic logs.

## Safe next hardware check

1. After package replacement is explicitly approved and the user installs it from XMB, verify that the image and `PRESS START TO EXIT` prompt appear.
2. Press controller START once and confirm the app returns to XMB without a forced shutdown. The updated path has not yet been hardware-proven.
3. Relaunch only if the first check exits normally. Try the iPhone connection once, then return using START.
4. Retrieve `/dev_hdd0/tmp/ps3-airplay.log` over FTP and inspect/redact it locally. Use the RTSP and session events to locate the failed connection stage before adjusting service capabilities.
5. Keep boot plugin configuration unchanged. Do not repeat a long power-button shutdown unless the console becomes unresponsive and no safe exit is available.
