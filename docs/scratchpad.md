# Active work

- Current task: fix/diagnose inconsistent controller input in v0.2.1.
- v0.2.0 (PKG SHA-256 `924a01ef0ef812e1badb8e94011930f5187c4dba91b253e578e16ab72de4f29b`) was installed/launched and a user-streamed song reached RTSP RECORD, RTP decrypt, ALAC decode, and PCM submission. The user reports START and PS buttons did not work. Saved logs show pad poll success/pad 0 present but no button data.
- webMAN normal exit recorded `SYSUTIL_EXIT_GAME`, full cleanup, and `APP_STOP`; status confirmed XMB. v0.2.0 and the prior v0.1.39 rollback package remain staged. Raw hardware logs are private and ignored under `work/hardware/2026-11-07-pre-v020-install/`.
- v0.2.1 / SFO 00.41 is installed. User reports START exited once, then after relaunch START and PS did nothing. Latest log shows a system-menu-open callback but no close, button transitions, or START exit. Treat reported callback as OS event delivery; visible PS menu is unconfirmed.
- Display visibility and the user's subjective audio quality are not confirmed. The capped log wrapped; current counters are interval evidence, not complete-session totals.
- Package: 3,231,920 bytes; SHA-256 `f45b2935213dc1cfa62cfc70f958fc7a240925979fbe7aa779822a1a4a8a2c50`. Source commit `27da6c5cef4b41b58b672dc7e275b16667790b6c`; full Lima host/sanitizer suite and package validators passed.
- Latest hardware log is private/ignored at `work/hardware/2026-11-07-pre-v020-install/v021-live-log-latest.log` (10,658 bytes; SHA-256 `8fd3a3e922808cca9b5182149f0a1aed6c236770381dea2c7a2a59a64602e8e7`). It shows an RTSP session, ALAC decode, PCM queueing/submission, and teardown; this does not prove audible output.
- Next: make v0.2.2 / SFO 00.42 capture raw digital button masks and report counts, and detect START by both the PSL1GHT bitfield and documented raw mask. Preserve safe XMB callback handling; rebuild/install only after checks.
