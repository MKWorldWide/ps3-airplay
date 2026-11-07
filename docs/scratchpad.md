# Active work

- **Task:** Validate v0.1.2 and prepare the display/exit hardware check before investigating AirPlay audio.
- **Verified:** Current `main` matches `origin/main` at `a6b36cd`; candidate package is 3,225,712 bytes with SHA-256 `923aebf6eca5693b21b494a9a4225c36629f13e8b316b2825d5deb3763226ebf` and prior package extraction checks match APP_VER 0.1.2 / AIRPLAY02.
- **Blocked:** The console is unreachable by ping and FTP. Do not transfer until read-only disk, staged package, installed app, log, and recovery checks are completed and the exact artifact transfer is approved.
- **Risks:** Display code has unbounded RSX label/flip waits; START input initializes afterward. Fresh direct receiver loopback test stalls at timing-probe receive, while trace-assisted test passes. Neither issue is resolved.
- **Next:** Restore PS3 network reachability, then complete read-only preflight. Decide whether v0.1.2 is safe to test as-is or needs a bounded display-wait correction first.
