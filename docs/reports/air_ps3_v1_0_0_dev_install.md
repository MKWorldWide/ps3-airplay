# AIR PS3 v1.0.0 development-unit installation

**Date:** 2026-11-08
**Source revision:** `f3da352809f81daac5e3b61a35d7fc18362bf1cc`
**Package:** AIRPLAY02 app `1.0.0`, SFO `01.00`, debug/type 1

## Transfer and installation

- Sent the webMAN exit-to-XMB command before deployment.
- Uploaded `ps3-airplay-receiver-1.0.0.pkg` to the development unit over FTP.
- Downloaded the staged PKG back and confirmed byte-for-byte equality with the local artifact.
- Installed the package through webMAN.
- Retrieved installed `PARAM.SFO` and `USRDIR/EBOOT.BIN` over FTP.
- Installed SFO passed the validator: `APP_VER=01.00`, `VERSION=01.00`, `TITLE_ID=AIRPLAY02`, `TITLE=AIR PS3`.
- Installed SFO SHA-256 `26de2a6b23df8e33d974351f10e474416b00e3ea171d03fdb4cce9101d341b16` matches the extracted candidate SFO.
- Installed EBOOT is 494,320 bytes with SHA-256 `ac07abd3ddc99f1b962402c3bf15b7a9725f232c44415a16554a8d3250e00759`, matching the candidate payload exactly.

## Result boundary

Installation identity is verified. The app was not launched in this pass, so no v1.0.0 runtime, display, controller, AirPlay, or audio behavior is claimed. The primary PS3 was not changed. Private network addresses and raw device logs are intentionally omitted.

## Next step

Run one controlled development-unit launch and record visible display, START exit, PS-menu behavior, AirPlay connection, and audible playback separately.
