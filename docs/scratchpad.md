# Active work

- Current task: v0.2.4 PS-menu refresh and append-only per-launch logs.
- v0.2.3 START exited to XMB once by user report. Pressing PS later produced no visible menu; START no longer worked afterward. Private log SHA-256 `ae8689248b6e0bdac6d8a25aa1dd06e7a1f75acc1151470b730156726dcd280e`: DRAW_BEGIN/MENU_OPEN, 18 reports, then zero; no menu close.
- Hypothesis: static-screen optimization stopped RSX flips while VSH attempted its PS menu. v0.2.4 resumes paced flips only during MENU_OPEN. Hardware confirmation required.
- Logging change: append-only file; each launch starts with UTC-timestamped APP_START. Each session has a 16 KiB cap, emits LOG_SESSION_CAPPED, and stops logging until next launch. Earlier sessions remain intact; total file grows by at most 16 KiB per run.
- Candidate: app 0.2.4 / SFO 00.44. Run logger append/cap test and full Lima host/sanitizer + PPU/package build, then upload/install under standing authorization.
