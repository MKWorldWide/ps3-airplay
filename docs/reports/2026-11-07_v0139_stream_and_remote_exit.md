# v0.1.39 active stream and remote exit follow-up

Date: 2026-11-07

## Summary

While the user reported AirPlay working but controller/PS input unresponsive, webMAN MOD was used to send `/xmb.ps3$exit`. It returned HTTP 200. The receiver log then shows the PS3 exit-game event and complete application cleanup. A repeated FTP read returned an identical log; read-only webMAN status showed XMB with no AIRPLAY02 title active. The console was not rebooted or power-cycled.

## Stream evidence

The capped log retained a live segment at 44.1 kHz, stereo, encrypted RAOP recording state. It shows two RTP receive batches of 512 packets each (721,913 payload bytes combined), with receiver queue-depth snapshots of 97 and 93 in a 256-frame queue. The two batch checkpoints reported 42 and 57 missing sequence numbers. Separate five-second stats checkpoints reported 65 and 47 recovered packets against 23 and 30 resend requests, 10 and 6 concealed missing frames, zero large-gap skips, and zero resend rejection categories. Different sampling boundaries prevent reconciling exact losses against exact recovery counts.

RTP decrypt success and ALAC decode checkpoints appear; ALAC failure totals were zero in both retained stats snapshots. The ALAC checkpoint is 352 samples / 1,408 PCM bytes. PCM ring occupancy was 12,426 then 12,512 of 16,384 input frames, roughly 76% or 282-284 ms at 44.1 kHz. Queued/read frame deltas were 220,704/220,617 then 220,704/220,618; GameOS block submissions were 240,128 output frames per interval. No PCM_UNDERFLOW event appears in the retained active segment. These prove software pipeline progress, not the exact HDMI waveform or perceived fidelity.

The log contains one invalid 32-byte RTP timing response; that is not an arrival-jitter estimate. The log had wrapped at 16,370 previous bytes, so earlier stream behavior and any earlier underflows are unavailable in this retained copy.

## Control and lifecycle evidence

PAD_HEALTH repeatedly reports result 0 and mask 1. In source, this means pad polling succeeded and controller port 0 was present; it does not show button states. No `USER_EXIT_START` event appears. `APP_HEARTBEAT` records session state 3 during streaming and later 0 (idle); its menu-open field remains 1 throughout the retained idle heartbeats, but this internal flag alone does not prove the XMB overlay was actually visible or usable.

At 23:54:03Z the log records `SYSUTIL_EVENT value=257`, which PSL1GHT defines as `SYSUTIL_EXIT_GAME`, followed by a second unclassified event value 17, `CLEANUP_BEGIN`, `SCREEN_CLOSE_DONE`, `AUDIO_CLOSE_DONE`, `RECEIVER_CLOSE_DONE`, and `APP_STOP`. This is an orderly application exit. Subsequent log retrieval was unchanged and webMAN status showed XMB.

## Acceptance and follow-up

- Remote XMB exit and cleanup: PASS.
- Encrypted RTP through decrypt, ALAC and PCM submission: PASS to software output submission.
- RTP delivery: measurable gaps remain; retransmission recovery is active; 16 concealed frames were counted in two stats snapshots.
- Controller button receipt: UNKNOWN; current diagnostics omit button states.
- Direct arrival jitter, HDMI loopback quality, visible display, acoustic quality, and Spotify-specific output: NOT MEASURED in this test.

Next, add privacy-safe button transitions and actual monotonic arrival-jitter/PCM occupancy-extrema metrics. Then run one controlled playback and exit test and correlate those measurements with a user recording.
