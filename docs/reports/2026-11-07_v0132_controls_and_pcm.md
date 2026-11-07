# v0.1.32 controls and target PCM validation

## v0.1.32 results and installed artifact

- Source: e34cbce8f2d89af6c3caa827817b8d5d3d532ca5. Production RTSP 5000; emulator smoke uses the same source with RTSP 5001.
- ALAC golden-fixture check passes on the host with ASan/UBSan and on the actual big-endian PPU ELF executed in RPCS3: 353 samples, output bytes equal the FFmpeg PCM fixture. The PPU test only changes fixture file paths for the emulator environment.
- Fresh full-app RPCS3 test: challenge response 55 ms; paced sender completes encrypted session, sends 256 packets spaced 8 ms apart after the diagnostic tone, and tears down successfully. Target log records prebuffer 128, decrypted packets 1/128/256, ALAC_FRAME 353 samples, PCM_QUEUED_FRAMES=90368 and PCM_CONSUMED_FRAMES=90368. Frames were submitted to the audio port; no human-audible music claim.
- Heartbeat and PAD_HEALTH remain active (successful pad API and connected-port mask 1); no SCREEN_PRESENT_FAILED event during the run. RPCS3 cannot establish physical PS-button overlay behavior. It still logs its known mDNS interface failure. Emulator stopped by harness SIGINT; no new controller-exit/cleanup timeout result is claimed.
- Production ELF: 2,780,528 bytes; SHA-256 45fe13932e255df611a3746acde682f815c013dcb10e6599cc0fc82fc52313eb.
- Package: ps3-airplay-receiver-0.1.32.pkg, 3,234,544 bytes; SHA-256 fca5547fda865f22faff7099834d0c3a8e77e6658f955b5c3a4a941cfd7ac7b3. SFO APP_VER/VERSION 00.32, AIRPLAY02, type-1 debug. ELF, extracted assets/content identity, SFO and package validators pass.
- Uploaded as a new versioned file and downloaded back byte-for-byte. Remote install requested under standing authorization; installed SFO and EBOOT match the validated package exactly. Old physical app was recovered first by normal webMAN quit; saved old live/cleanup logs and rollback packages remain local.
- Physical PS-menu/START behavior and audible streaming remain pending maintainer observation. Test PS/held-PS and idle START first, then a short sender stream and disconnect/START. Capture logs before relaunch.
- Reviewed staged diffs, changed/new files and ignored candidates; whitespace and targeted identifier/credential scans clear. No secret scanner installed; raw logs and artifacts remain ignored and private.
