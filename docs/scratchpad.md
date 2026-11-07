# Active work

- **Task:** Validate v0.1.2 and prepare the display/exit hardware check before investigating AirPlay audio.
- **Verified:** Current `main` matches `origin/main` at `a6b36cd`; candidate package is 3,225,712 bytes with SHA-256 `923aebf6eca5693b21b494a9a4225c36629f13e8b316b2825d5deb3763226ebf` and prior package extraction checks match APP_VER 0.1.2 / AIRPLAY02.
- **Preflight:** Console is reachable and currently reports XMB with 9.2 GB free. Installed AIRPLAY02 is 01.01; staged PKG is 3,222,640 bytes, SHA-256 `7824a4b15e4757afad3213a3f55cd3bea816b33795403da12bfd7ede29f93db4`. Exact staged backup and current 1,817-byte log are preserved under ignored `work/hardware/2026-11-07-preflight/`.
- **Log:** Tone completed and `APP_STOP` recorded, but there is no `USER_EXIT_START`, RTSP event, or build marker. Treat package identity and clean exit as unknown.
- **Next:** Obtain exact approval to replace the staged file with v0.1.2 only. After transfer and downloaded-back hash verification, user installs from XMB; test visible output and START exit before attempting iPhone connection.
- **Risks:** Display code has unbounded RSX label/flip waits; START input initializes afterward. Fresh direct receiver loopback test stalls at timing-probe receive, while trace-assisted test passes. Neither issue is resolved.
