# AIRPLAY02 v0.2.2 controller fallback

**Status:** Installed and launched; user reports START and PS still do nothing. Manual reboot/launch comparison pending.

## Build

- Source commit: `6151aae43b7ed26dfa940f090afbdd5b13ba53ac`.
- App version `0.2.2`; AIRPLAY02 SFO APP_VER/VERSION `00.42`.
- PPU ELF: 2,754,776 bytes; SHA-256 `554c2580944bb0cd6d214f96b410c5d2c2395da609beeca9230455d2a3aa1bf6`.
- Finalized PKG: 3,232,112 bytes; SHA-256 `44886cd6ce7bbeff8089bd2d5d752cce3b76ded01f36b214cfdaff057f6748cd`.
- Extracted EBOOT SHA-256 `b250051d8784922f696540cab7a5b672d7e85d651e1e6aa2df6144fec7e88fbf`; SFO and package identity/asset/NPDRM validators passed.
- Existing Lima x86_64 guest: PS3DK v0.27.0 PPU compiler 16.1.0, GNU ld 2.42, PSL1GHT `dd4e8c9be744e3273d665c393fa6d97a87402e2e`.
- Full host/sanitizer suite, PPU objects, standalone PPU ELF, SFO, package extraction, asset, and finalized-envelope validation passed.

## Hardware deployment

- The prior app was exited via webMAN's XMB exit command. Its log recorded `SYSUTIL_EXIT_GAME`, screen/audio/receiver cleanup completion, and `APP_STOP 0`.
- Uploaded the new versioned package over FTP; downloaded-back bytes and SHA-256 matched. Installed via webMAN. Installed SFO matches exactly; installed EBOOT matches package SELF plus 32 zero bytes.
- Launched AIRPLAY02. Fresh log records `PKG_VERSION_0.2.2`, screen/controller setup, RTSP/RTP, mDNS and audio ready. Startup tone remains absent.
- The user reports START and PS still do nothing. Captured log: 2,340 bytes; SHA-256 `010188b420884a0233e7cc04abfc17d8f0dcbf2c88aeee78a2107f38c6229f8f`.
- Log records zero `PAD_REPORTS` during two heartbeat windows. At 01:13:14Z it records `SYSUTIL_MENU_OPEN`; the next heartbeat has two pad reports, but no `PAD_RAW_MASKS` transition, `PAD_BUTTONS` transition, `USER_EXIT_START`, or `SYSUTIL_MENU_CLOSE`. The callback indicates a PS-menu-open event reached the app, but does not prove the menu was visible or responsive.
- User plans to reboot and launch manually without webMAN to compare startup/control behavior. No further webMAN/FTP console command should be sent until the user reports the result.

## Interpretation and limits

The raw-mask fallback did not produce a button event in this test. Two positive pad reports are not evidence of digital-button data. Audio playback was not retested on v0.2.2; no audible result is claimed. The manual-launch comparison may distinguish launch-path effects from app input handling, but the current evidence does not establish either as the cause.
