# Recovered hardware launch and incomplete AirPlay session


## First recovered hardware launch: historical build label 0.30

The maintainer confirms app startup, five-second tone, and START return to XMB while idle after the SFO correction. START during the apparent AirPlay playback attempt became unresponsive; PS button did not show XMB. A three-second power hold shut down with three beeps. No sustained or audible AirPlay audio was heard.

Recovered original log: 2,774 bytes, SHA-256 3e4f2c18af96834e36f3ef1e7fab8cff408badac9408bad610aece03ef6ae866. It identifies PKG_VERSION_0.30, SCREEN_READY 1920/SCREEN_HEIGHT 1080, mDNS and all receiver/audio listener readiness, and tone completion. TCP accepted and several OPTIONS responses returned 200. Final event is another RTSP_OPTIONS (159 bytes), with no status afterward. No ANNOUNCE, SETUP, RECORD, RTP, ALAC, USER_EXIT_START, or APP_STOP appears. The raw log stays local and private.

The iPhone's selected AirPlay route and Spotify playing indicator did not establish a completed RAOP session. The earliest measured failing boundary is the last OPTIONS handler. Apple-Challenge/RSA work is a hypothesis because challenge signing occurs synchronously before response construction; this log does not contain request headers or identify the exact blocked operation. Do not diagnose decoder/output or cleanup failure from this run alone.

Current requested release label is 0.1.30, filename/app logging 0.1.30, SFO APP_VER and VERSION 00.30. Historical 0.30 package remains a distinct preserved artifact. This pass changes the release label only; protocol and runtime logic match the launchable build. Investigate the OPTIONS freeze in the next version before further extended streaming tests.

Historical tested package source: c431ad71dabce678e03f7bd6663fb2b492ccd1bf. Historical package 0.30: 3,231,984 bytes; SHA-256 a4a8a9dabd6a8df94e95abf04f96aec330fcdc935837584b3b6bc952a239ac97. Installed SFO and extracted EBOOT matched that package before the maintainer's launch test. User observations and log stages are distinct evidence. Corrected-label 0.1.30 build results will be recorded after packaging.

## Corrected-label artifact and console installation

- Source: bfca67f63654e55ceeac811e213ccc1be09cdff2, committed archive with exact revision stamp.
- Release: 0.1.30; PS3 SFO APP_VER/VERSION: 00.30; AIRPLAY02; debug package type 0x00000001; production RTSP 5000.
- ELF: 2,780,208 bytes; SHA-256 a692ae4ccf3d9f33031673c9c4c31fdec38affab4ed887fcd2fd6c4f7780acdf.
- PKG: ps3-airplay-receiver-0.1.30.pkg, 3,231,984 bytes; SHA-256 9b2040289d54b71a50c00af8abc2171ada56031071ee1d1464e4ab3620385acf.
- Clean PPU build, CellOS ELF layout validation, package extraction/entry paths, SFO syntax/identity, content ID and embedded assets PASS.
- FTP uploaded as a new versioned package and downloaded back byte-for-byte. Remote installation invoked under standing authorization; retrieved installed EBOOT and SFO match the extracted package exactly. Original 0.30 and other rollback packages remain staged.
- No post-install launch of corrected-label 0.1.30 was performed by the agent. Earlier user startup/tone/idle-exit results refer specifically to the historical 0.30 artifact. Receiver behavior is unchanged, including the unresolved OPTIONS stall.
- Privacy review: full staged diffs and changed/new files inspected, whitespace and targeted identifier/credential scans clear; no installed secret scanner. Raw logs, artifacts and deferred RNG patch remain ignored and local.
