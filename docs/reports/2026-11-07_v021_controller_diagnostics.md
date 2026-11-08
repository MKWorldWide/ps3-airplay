# AIRPLAY02 v0.2.1 controller diagnostics

**Status:** Installed and launched; controller result is inconsistent and requires a targeted follow-up.

## Build

- Source commit: `27da6c5cef4b41b58b672dc7e275b16667790b6c`.
- App version: `0.2.1`; SFO APP_VER: `00.41`; title ID: `AIRPLAY02`.
- Finalized PKG: 3,231,920 bytes; SHA-256 `f45b2935213dc1cfa62cfc70f958fc7a240925979fbe7aa779822a1a4a8a2c50`.
- ELF: CellOS PPC64, five PT_LOAD segments; SHA-256 `e875966c332610bb61e56f297b36eaaa69f74c190147c920d8c52a45ebfdc6c3`.
- Toolchain: existing Lima x86_64 guest, PS3DK v0.27.0 PPU compiler 16.1.0, GNU ld 2.42, PSL1GHT `dd4e8c9be744e3273d665c393fa6d97a87402e2e`.
- Full Lima host/sanitizer suite, PPU component compile, standalone ELF validation, and finalized package/SFO/content/extraction/EBOOT/assets/NPDRM checks passed.
- Artifact preserved locally under ignored `work/builds/0.2.1/`.

## Hardware deployment

- Before installation, webMAN showed XMB and approximately 10.4 GB free. v0.2.0 and v0.1.39 package rollback files were staged.
- Uploaded v0.2.1 to a new package path by FTP, then verified the remote 3,231,920 bytes and SHA-256 against the local artifact. Installed via the keep-package webMAN command.
- Installed SFO reports `00.41`. Installed EBOOT is 451,440 bytes; the first 451,392 bytes match the extracted package SELF exactly, followed by 48 zero bytes.
- Launched AIRPLAY02 through webMAN. Current status reports AIRPLAY02 / SFO 00.41 active.
- User reports START exited to XMB once. After reopening the app, they report neither START nor PS worked.
- Latest private log snapshot: 10,658 bytes; SHA-256 `8fd3a3e922808cca9b5182149f0a1aed6c236770381dea2c7a2a59a64602e8e7`. It records one `APP_START`, `PAD_CONNECTED`, initial `PAD_BUTTONS value=0`, and `SYSUTIL_MENU_OPEN` at 00:58:05Z. Subsequent 10-second heartbeats show pad API success and port mask 1; there are no later `PAD_BUTTONS`, `USER_EXIT_START`, `SYSUTIL_MENU_CLOSE`, or `APP_STOP` events.
- The same snapshot shows RTSP OPTIONS/ANNOUNCE/SETUP/RECORD, ALAC initialization and frames, PCM queued/consumed/submitted, RTP reception/decryption and resend recovery, followed by RTSP TEARDOWN. This proves data reached decode/output submission, not audible quality.
- The sysutil event confirms a PS menu-open event reached the callback, but not that the menu became visible or responsive. The app remained alive after the AirPlay client disconnected.
- Raw hardware logs stay under ignored local `work/hardware/2026-11-07-pre-v020-install/`; no unredacted log is committed.

## Limits

This build adds diagnostics, not a proven controller fix. Display visibility is unconfirmed. The app log does not establish visible or responsive XMB; use the verified webMAN XMB-exit path only if recovery is needed.

## Controller diagnosis

The poller skips processing when `ioPadGetData` returns a zero-length unchanged report. Its periodic `PAD_HEALTH` reports only `ioPadGetInfo` success and port presence. v0.2.2 adds raw digital masks, positive report counts, and a raw Start-mask fallback alongside the existing PSL1GHT field. Do not describe v0.2.1 as having fixed controls; v0.2.2 still requires hardware verification.
