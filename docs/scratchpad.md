# Active work

- Current source/package candidate: AIRPLAY02 v0.1.39 / SFO 00.39; production TCP 5000, RPCS3-only TCP 5001.
- Code: hold RTP sequence gaps for 80 ms; conceal up to 128 lost ALAC frames as PCM silence; skip larger sequence jumps to bound latency. Full Lima ASan/UBSan and PPU component suite passes.
- v0.1.38 emulator test: installed/launched, visible background, tone complete, encrypted 128-packet session decoded/submitted PCM, keyboard START exited cleanly. No packet loss, acoustic confirmation, or mDNS success; recovery not exercised in RPCS3.
- Physical baseline remains v0.1.35. User has PS3 off; no console operations.
- Pushed source: commit `1a5520d6e37239070012cb0f8ed3f7953074bbc0`. Both production port-5000 and RPCS3 port-5001 v0.1.39 / SFO 00.39 finalized packages validate; hashes are in docs/memory.md. Full Lima ASan/UBSan and PPU tests pass on that exact commit.
- RPCS3 currently last has v0.1.38 runtime evidence. Its latest log proves encrypted test fixture decode/PCM submit and clean emulator exit; no packet loss was injected and mDNS failed. v0.1.39 installation is pending: CUA timed out opening installer and reading AX tree. Do not switch to manual emulator HDD file replacement.
- Physical PS3 is off and untouched. Next: restore RPCS3 control, install/test v0.1.39 with baseline and loss/reorder sender fixtures; then wait for console availability before any hardware work. No claim of perfect or acoustically verified playback.
