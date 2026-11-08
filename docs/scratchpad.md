# Current scratchpad

- Task: qualify the experimental resident audio path on `experimental/resident-airplay` only.
- Verified: host lifecycle mocks and bounded PCM SPSC/concurrency tests pass. RPCS3 accepted fake SPRX; resident and harness submitted PCM through two ports.
- Blocker: audio cycles 1–2 stopped/unloaded; cycle 3 crashed in `ps3-airplay-audio` at a null target. The reported relocated module offset is not confidently mapped to a named function. Cycle-3 unload/module baseline is unknown.
- Do not: claim decoded AirPlay playback, audible output, in-game mixing, or hardware readiness. Do not touch physical PS3s, main, or the published release.
- Next: isolate the audio-thread exit fault; rerun three audio-enabled cycles and require baseline module count after each; then test synthetic RAOP decode-to-output.
