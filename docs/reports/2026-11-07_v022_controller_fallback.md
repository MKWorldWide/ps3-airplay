# AIRPLAY02 v0.2.2 controller fallback

**Status:** Installed and launched; manual reboot/direct XMB launch reproduced the same no-buttons report. Controller failure remains unresolved.

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
- Manual reboot/direct XMB launch comparison completed: user reports the same failure, with no buttons working. Captured private log at `work/hardware/2026-11-07-pre-v020-install/v022-live-log-after-manual-reboot.log`: 3,672 bytes; SHA-256 `d51d7fb882e0ddb5dc60cae08449bb1aab4292c59c700ecb0fde5a1b051c0c9f`. No remote console action is being taken; the user retains control.

## Interpretation and limits

The manual launch did not change the reported behavior, so webMAN launch is not the differentiator. The log begins at `APP_START` / v0.2.2, records a connected pad and one early input report, then `SYSUTIL_MENU_OPEN` at about five seconds. One subsequent heartbeat reports two pad samples; later heartbeat counts are zero. No raw digital transition, START event, `SYSUTIL_MENU_CLOSE`, or `APP_STOP` appears. The callback confirms event delivery only; it does not prove a visible or responsive system menu. Audio playback was not retested in this run.

## Next controlled observation

On a fresh launch, test START first, before PS, and report whether the app exits. If it remains open, press PS once and report separately whether the system menu visibly appears and responds. This isolates app button handling from the PS system-menu event ordering. Do not change source or produce another package until this distinguishes the input path.
