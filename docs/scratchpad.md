# Active work

- Current task: hardware-test installed v0.2.4 PS-menu refresh and append-only per-launch logs.
- v0.2.3 START exited to XMB once by user report. Pressing PS later produced no visible menu; START no longer worked afterward. Private log SHA-256 `ae8689248b6e0bdac6d8a25aa1dd06e7a1f75acc1151470b730156726dcd280e`: DRAW_BEGIN/MENU_OPEN, 18 reports, then zero; no menu close.
- Hypothesis: static-screen optimization stopped RSX flips while VSH attempted its PS menu. v0.2.4 resumes paced flips only during MENU_OPEN. Hardware confirmation required.
- Logging change: append-only file; each launch starts with UTC-timestamped APP_START. Each session has a 16 KiB cap, emits LOG_SESSION_CAPPED, and stops logging until next launch. Earlier sessions remain intact; total file grows by at most 16 KiB per run.
- Installed AIRPLAY02 0.2.4 / SFO 00.44 from source `f870d823f15398691a6a38e6393f8d1376161bee`; package SHA-256 `a27d5c7b5d2d84f12fbb82ee4e164b9b50ccc825b5104b9b749f053d72a42bf7`. FTP download-back and installed SFO/EBOOT comparisons passed.
- Previous app closed cleanly to XMB with screen/audio/receiver cleanup and APP_STOP. New version has not yet been launched.
- Next: user launches v0.2.4 once, checks whether PS menu appears and can return; then checks START. Retrieve the appended log and verify new APP_START timestamp plus preservation of previous session bytes.
