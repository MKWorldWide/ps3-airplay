# Current scratchpad

- Task: continue resident AirPlay work on `experimental/resident-airplay`; no stable-release or physical-console changes.
- Latest RPCS3 result: rebuilt `RPRX90001` completed three audio-enabled load/start/ready/stop/join/unload cycles. Module count returned to six each time; harness ended `HARNESS_PASS value=3`.
- Likely fix: at least 72,008 bytes of audio-worker local buffers exceeded the 65,536-byte stack. Buffers now live in single-worker-owned static storage. This is the leading explanation for the earlier crash, not a proven exact faulting instruction.
- Verified audio boundary: RPCS3 accepted seeded resident PCM and foreground tone submissions on separate ports. No acoustic output, real-game mixing, or resident RAOP packet-to-output run is verified. Network startup still reports `EADDRNOTAVAIL`.
- Next: add or run an isolated synthetic RAOP-in-resident path with usable networking, measure non-module resources across repeat cycles, and gather independent output evidence. Hardware readiness remains NOT READY.
