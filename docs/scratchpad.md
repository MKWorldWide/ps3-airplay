# Active work

- Current task: preserve v0.2.4 as the safe functional baseline; v0.2.5 is built and validated, with console install deferred until the active app is safely at XMB.
- v0.2.3 START exited to XMB once by user report. Pressing PS later produced no visible menu; START no longer worked afterward. Private log SHA-256 `ae8689248b6e0bdac6d8a25aa1dd06e7a1f75acc1151470b730156726dcd280e`: DRAW_BEGIN/MENU_OPEN, 18 reports, then zero; no menu close.
- Hypothesis: static-screen optimization stopped RSX flips while VSH attempted its PS menu. v0.2.4 resumes paced flips only during MENU_OPEN. Hardware confirmation required.
- v0.2.4 logging is append-only with UTC APP_START, but the second observed run hit the 16 KiB cap 37 seconds after startup; no more log events can be recorded in that session.
- Installed AIRPLAY02 0.2.4 / SFO 00.44 from source `f870d823f15398691a6a38e6393f8d1376161bee`; package SHA-256 `a27d5c7b5d2d84f12fbb82ee4e164b9b50ccc825b5104b9b749f053d72a42bf7`. FTP download-back and installed SFO/EBOOT comparisons passed.
- User confirms PS menu opens, resume leaves a dim screen and frozen loading indicator, START exits cleanly, and AirPlay music streams with short audio stutters. Passive log snapshots show no growth after the new 16 KiB session cap.
- Requested 12-minute v0.2.4 observation completed without interruption. FTP snapshots stayed byte-identical after the 16 KiB cap; playback duration after the cap is not confirmed by logs.
- v0.2.5: source `9d9f60657f117daedc47c6b1bc3285a5982e5ae3`; 5 MiB per-launch cap, append history, UTC APP_START, full-cap regression, and fail-fast release wrapper. Full Lima gate passed. Package SHA-256 `61e8310a6d4330bad1e4d64213365f9ba27de81a6fa14851a315f5b338894440`; it is local only and not installed. Resume dimming and audio stutter remain unresolved.
