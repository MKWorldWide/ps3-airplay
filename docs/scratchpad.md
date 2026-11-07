# Active work

- Current source/package candidate: AIRPLAY02 v0.1.39 / SFO 00.39; production TCP 5000, RPCS3-only TCP 5001.
- Code: hold RTP sequence gaps for 80 ms; conceal up to 128 lost ALAC frames as PCM silence; skip larger sequence jumps to bound latency. Full Lima ASan/UBSan and PPU component suite passes.
- v0.1.38 emulator test: installed/launched, visible background, tone complete, encrypted 128-packet session decoded/submitted PCM, keyboard START exited cleanly. No packet loss, acoustic confirmation, or mDNS success; recovery not exercised in RPCS3.
- Physical baseline remains v0.1.35. User has PS3 off; no console operations.
- Next: finish privacy review and commit/push; sync exact commit into Lima; build v0.1.39 production + RPCS3 packages; install/test emulator package and validate bounded gap telemetry. Make no Spotify quality claim until hardware/listening tests.
