# PS3 hardware test — audio works, display and AirPlay need diagnosis

**Date:** 2026-11-07 (America/New_York)

## User-reported outcome

- The earlier package first failed to start with `80029530`. A corrected NPDRM package was then used for a hardware run; the user reports the five-second diagnostic tone played. The exact installed package version was not separately confirmed.
- The screen remained black and the user could not return to XMB through the app. The console was shut down using a long power-button press.
- An iPhone listed `ps3-airplay-diagnostic`; selecting it reported that it could not connect. No RAOP session or music playback was confirmed.
- A 350-byte diagnostic log was retrieved over FTP. It records `APP_START`, `SCREEN_READY`, `CONTROLLER_READY`, `MDNS_READY`, `AUDIO_READY`, and `AUDIO_TEST_TONE_START`. Its package version is unconfirmed; there is no tone-complete, RTSP, or stop event in this snapshot.

## Evidence summary

| Check | Status | Evidence |
|---|---|---|
| Lima PPU build | PASS | Screen-enabled receiver ELF built with the PS3DK/PSL1GHT toolchain |
| Screen-enabled package | PASS, host-side | Package listing and extraction include NPDRM EBOOT, SFO, XMB icon, and exact 1280x720 RGB background |
| Screen-enabled package staging | PASS | User approved version 01.01; 3,222,640-byte package downloaded back byte-for-byte with matching SHA-256 |
| Audible diagnostic tone | PARTIAL PASS | User reports hearing the five-second tone on hardware |
| Display | FAIL | User reports a black screen while the app was active |
| XMB return | FAIL on previous build | User needed a long power-button shutdown; new START-button cleanup path is not yet built into a hardware-tested package |
| iPhone discovery | PARTIAL PASS | User saw `ps3-airplay-diagnostic` in the iPhone's receiver list |
| iPhone connection / RTSP | FAIL | iPhone reported it could not connect; no successful RAOP session confirmed |
| Diagnostic log | PARTIAL | 350-byte snapshot has screen/controller/mDNS/audio startup events; no RTSP event or tone-complete event was present |
| AirPlay music playback | NOT TESTED | No successful connection or music playback reported |
| Resident PRX / game audio mix | NOT TESTED | No SPRX was installed or loaded; game coexistence remains a separate feasibility gate |

## Screen and exit follow-up

Source inspection showed that the receiver previously initialized audio and networking but did not initialize RSX video or poll controller input. The screen-enabled candidate follows PSL1GHT's framebuffer setup pattern, scales the public maintainer-provided background to the active display resolution, overlays `PRESS START TO EXIT`, and polls controller START for normal cleanup. The retrieved log's `SCREEN_READY` event did not prove visible pixels. PSL1GHT issue #144 describes a similar black-screen/XMB problem with graphics samples on Evilnat; the project already uses the `-O2` setting noted in that report, so it does not identify this project's root cause.

The current local revision primes the spare framebuffer before the primary buffer, matching the upstream `videoTest` startup sequence, and adds a bright `DISPLAY ACTIVE` banner with color bars. `SCREEN_READY` records framebuffer width and a separate `SCREEN_HEIGHT` event records framebuffer height without overloading the logger's byte-count field. Version 01.02 built in Lima with `-O2 -Wall -Wextra -Werror`; its extracted package contains the expected SFO, NPDRM EBOOT, icon, and exactly 2,764,800 bytes of background data. Candidate PKG SHA-256: `ddedd7d07fcac3554275b331b821741615019ea2d3a69f9fc9965cbdfbaec94a` (3,223,664 bytes). This is a diagnostic improvement, not a verified hardware fix; the candidate has not been transferred or run.

The retrieved log's `SCREEN_READY` event means the app completed RSX setup and its first flip, but the exact package version used for that log is not confirmed and the user had reported black video. The event alone does not prove visible pixels reached the display. FTP became unreachable on a subsequent check, so the log may be incomplete. Controller exit and visible rendering remain unconfirmed. The app remains a foreground standalone receiver; no in-game overlay or universal game audio mixing is claimed.

## Package and privacy notes

- The first failed package and the corrected NPDRM-only package are documented in the preceding deployment record.
- The new candidate contains `USRDIR/background.rgb`, produced from `assets/diagnostic-background.jpg`; generated RGB data and the PKG remain in ignored build output.
- The public source JPEG was re-encoded without EXIF metadata. Its reuse terms are unspecified and documented separately from the MIT-licensed code.
- Version `01.01` is now staged at `/dev_hdd0/packages/ps3-airplay-receiver.pkg`: 3,222,640 bytes; SHA-256 `7824a4b15e4757afad3213a3f55cd3bea816b33795403da12bfd7ede29f93db4`. The previous 416,240-byte staged package (`f1b17d07a05679030143e607c74b545592e92ad521135b860bc2aca677160cf2`) was preserved locally before replacement.
- Staging does not install or launch the title. The current installed `AIRPLAY02` version has not been confirmed; version 01.01 must be installed from XMB before testing the display and exit behavior.
- Do not publish private network addresses, unique console identifiers, credentials, or raw diagnostic logs.

## Safe next hardware check

1. Build and inspect the current local revision, then obtain authorization for the exact package before any transfer. After installation from XMB, verify that the bright `DISPLAY ACTIVE` banner, background, and `PRESS START TO EXIT` prompt appear.
2. Press controller START once and confirm the app returns to XMB without a forced shutdown. The updated path has not yet been hardware-proven.
3. Relaunch only if the first check exits normally. Try the iPhone connection once, then return using START.
4. Retrieve `/dev_hdd0/tmp/ps3-airplay.log` over FTP after a confirmed test run and inspect/redact it locally. Use the RTSP and session events to locate the failed connection stage before adjusting service capabilities.
5. Keep boot plugin configuration unchanged. Do not repeat a long power-button shutdown unless the console becomes unresponsive and no safe exit is available.
