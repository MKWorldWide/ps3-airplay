# AIRPLAY02 v0.2.3 normalized START fallback

**Status:** Built and installed. User reports START exited to XMB once; PS-menu behavior failed on a subsequent v0.2.3 launch.

## Context

On v0.2.2 the user pressed PS first; a menu-open callback was logged, but the user confirms the PS system menu never appeared visibly. Source inspection found that START was normalized from both `data.BTN_START` and the raw `PAD_CTRL_START` mask, but the exit action checked only `data.BTN_START`.

## Change

Source revision `5a04d93459611df730cc0b0f2d6f2afe84903b86` changes the START exit action to check the normalized mask. Release label is v0.2.3 and SFO APP_VER/VERSION are 00.43. This completes the START fallback path; it does not alter PS-menu behavior.

## Build and validation

- Full Lima host/ASan/UBSan suite and PPU component checks: PASS.
- PPU compiler: PS3DK v0.27.0 toolchain, GCC 16.1.0. Linker: GNU ld 2.42. PSL1GHT revision `dd4e8c9be744e3273d665c393fa6d97a87402e2e`.
- ELF: CellOS PPC64, 2,754,776 bytes, SHA-256 `59f42ec05dc233c7450abfd2303f4fb929caf835dbc0d582e8164448b53b20fd`.
- PKG: type-1 debug, AIRPLAY02, five items, 3,232,112 bytes, SHA-256 `756ac28147e98a37222279c607208639f157ddd4395bd2f4dfad8816ff4581b4`.
- Package extraction, SFO, title/content identity, assets, EBOOT, and package-envelope checks: PASS.

## Hardware deployment

- FTP stage path used a new v0.2.3 filename. Download-back bytes and SHA-256 matched the local artifact.
- webMAN install returned HTTP 200. Installed SFO validates as AIRPLAY02 / 00.43.
- Installed EBOOT is 451,632 bytes and matches the package SELF byte-for-byte.
- A subsequent webMAN launch command returned HTTP 200, but the console remained at XMB and the log contained no new `APP_START`; launch is NOT VERIFIED. The package remains staged and installed.

## Next step

Follow-up: user manually launched v0.2.3 and reports START exited once. On a later run, PS produced no visible menu and START no longer worked. The fresh 3,673-byte log SHA-256 `ae8689248b6e0bdac6d8a25aa1dd06e7a1f75acc1151470b730156726dcd280e` records DRAW_BEGIN, MENU_OPEN, 18 pad reports, then zero; no raw transition or menu-close. Candidate v0.2.4 continues paced flips only while the system menu is open. A separate v0.2.4 change makes logs append-only across launches, retaining each UTC APP_START timestamp and limiting each session to 16 KiB. Both changes require build and hardware validation.
