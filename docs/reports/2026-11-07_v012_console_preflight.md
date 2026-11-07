# v0.1.2 console preflight and prior-log review

**Timestamp:** 2026-11-07 03:06 EDT
**Source revision:** `a6b36cd43fe289b63549cbd06b888f984fe87936`

## Read-only preflight

- PS3 is reachable over the existing FTP connection and status page; status reports XMB and 9.2 GB free.
- Installed `/dev_hdd0/game/AIRPLAY02/PARAM.SFO` reports `APP_VER=01.01`, `TITLE_ID=AIRPLAY02`, and `TITLE=PS3 AirPlay`.
- Staged `/dev_hdd0/packages/ps3-airplay-receiver.pkg` is 3,222,640 bytes with SHA-256 `7824a4b15e4757afad3213a3f55cd3bea816b33795403da12bfd7ede29f93db4`. This matches the previously approved 01.01 package, not the v0.1.2 candidate.
- A byte-for-byte local copy of that staged package is preserved under ignored `work/hardware/2026-11-07-preflight/` for rollback. The 1,817-byte current diagnostic log is also preserved there. Neither file is tracked or staged.
- v0.1.2 remains local: 3,225,712 bytes, SHA-256 `923aebf6eca5693b21b494a9a4225c36629f13e8b316b2825d5deb3763226ebf`. No transfer, install, launch, settings change, or boot-plugin change occurred in this pass.

## Existing log evidence

The preserved log records `APP_START`, `SCREEN_READY`, `CONTROLLER_READY`, `MDNS_READY`, `AUDIO_READY`, diagnostic tone start and completion, repeated `PCM_UNDERFLOW`, and `APP_STOP`. It contains no RTSP event, no `USER_EXIT_START`, and no package-version marker. The log therefore supports that a diagnostic tone completed and that cleanup ran; it does not establish that the iPhone connected, that the run used v0.1.2, that START caused the exit, or that XMB returned normally. `SCREEN_READY` still does not prove visible HDMI output.

## Recovery and deployment boundary

The console is currently at XMB after a prior forced power-off/reboot, which demonstrates a recovery route back to the current state but does not make forced power-off a preferred shutdown. The app is installed under `AIRPLAY02`; the package inspection record contains a standalone EBOOT, SFO, icon, and background, with no boot plugin or firmware change. Recovery for a failed standalone launch is: use the controller exit if responsive; otherwise use the console power control only as an emergency, boot back to XMB, and reinstall the preserved 01.01 package if the AIRPLAY02 title must be restored. Do not load the separate SPRX or modify boot plugins.

## Next action

Request approval before replacing only the staged package with `ps3-airplay-receiver-0.1.2.pkg` (3,225,712 bytes, SHA-256 `923aebf6eca5693b21b494a9a4225c36629f13e8b316b2825d5deb3763226ebf`) at `/dev_hdd0/packages/ps3-airplay-receiver.pkg`. The local 01.01 backup is the staged-file rollback. After an approved transfer, compare the downloaded-back hash, then have the user install from XMB. The first test must be the display/tone/START-exit test; do not attempt AirPlay until the user confirms the screen and normal XMB return.

The display source has unbounded RSX label/flip waits. This remains a launch-hang risk; if it hangs, stop and assess a bounded wait correction before further runs. The current console being available does not make the test itself risk-free.
