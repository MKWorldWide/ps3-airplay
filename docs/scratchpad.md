# Current scratchpad

- Task: continue resident AirPlay work on `experimental/resident-airplay`; no stable-release or physical-console changes.
- Latest verified RPCS3 result remains the pre-expansion package: three audio-enabled cycles passed and module count returned to six. It did not prove captured sound or resident RAOP receive.
- Implemented but not rebuilt: harness source now defaults to 50 cycles across no-seed, immediate stop, event-confirmed drain-then-stop, and longer two-port modes. Resident seed is 440 Hz, foreground tone 660 Hz. Package source uses a new diagnostic flag to suppress the seed on selected cycles.
- Added SELF container type validation and four Python unit tests. All four tests pass. The validator distinguishes `0x00010001` production SPRX format from `0x80000001` emulator fake format; neither format proves CFW/Cobra acceptance.
- Build blocker: this environment lacks the PS3DK/Mbed TLS toolchain and prior Lima build endpoints. Current expanded package, 50-cycle RPCS3 stress, resident RAOP session, and audio capture remain untested.
- Next: restore the PS3DK build environment; build and inspect fresh signed/fake containers; run the 50-cycle RPCS3 harness; then solve network reachability and add RAOP/output evidence. Hardware readiness remains NOT READY; no physical console has been accessed.
