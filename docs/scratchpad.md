# Active work

- Current candidate: AIRPLAY02 v0.1.37 / SFO 00.37, commit `43eb638`. Production PKG is locally validated; emulator-only package uses port 5001.
- RPCS3: package install/launch and background rendering pass. Synthetic encrypted session negotiated, decrypted RTP, decoded ALAC; existing 128-packet fixture passed. Sustained/audio-quality and resume behavior remain unproven.
- Physical baseline: v0.1.35 log showed 195 RTP gaps, zero retained recoveries, and 86,106 active PCM underflow frames.
- Blocker: direct FTP and webMAN could not reach the PS3 at preflight; no console files were changed.
- Next: restore reachability; verify app state/version; use direct FTP to stage only the production PKG, then webMAN/XMB install and repeat short audio/display/exit tests.
