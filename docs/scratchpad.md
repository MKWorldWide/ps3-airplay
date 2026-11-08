# Active work

- Current task: diagnose the v0.2.0 controller failure with a v0.2.1 / SFO 00.41 candidate.
- v0.2.0 (PKG SHA-256 `924a01ef0ef812e1badb8e94011930f5187c4dba91b253e578e16ab72de4f29b`) was installed/launched and a user-streamed song reached RTSP RECORD, RTP decrypt, ALAC decode, and PCM submission. The user reports START and PS buttons did not work. Saved logs show pad poll success/pad 0 present but no button data.
- webMAN normal exit recorded `SYSUTIL_EXIT_GAME`, full cleanup, and `APP_STOP`; status confirmed XMB. v0.2.0 and the prior v0.1.39 rollback package remain staged. Raw hardware logs are private and ignored under `work/hardware/2026-11-07-pre-v020-install/`.
- v0.2.1 source now logs rate-limited pad connect/disconnect, button-mask transitions, pad API errors, and named PS-menu callbacks. Full Lima sanitizer/host suite, PPU component compile, and standalone PPU app compile/link passed. Package build is next.
- Display visibility and the user's subjective audio quality are not confirmed. The capped log wrapped; current counters are interval evidence, not complete-session totals.
- Next: commit/push the source and evidence after privacy review, then build finalized v0.2.1 / SFO 00.41 from that exact commit, install/launch using the already-authorized FTP/webMAN path, and collect button transitions during a short test.
