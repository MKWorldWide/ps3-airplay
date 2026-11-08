# Current scratchpad

- Task: qualify the experimental resident audio path on `experimental/resident-airplay` only.
- Verified: host lifecycle mocks and bounded PCM SPSC/concurrency tests pass. PPU build/structure check passes. Latest PKG hash is `d803bfd2daf94e6dbf7a61434330077f5e5b55418831d389a12ffbf0fb888a3c`; its embedded fake SPRX matches the just-built output. `audio_thread_started` now uses atomic publication/read semantics.
- Blocker: audio cycles 1–2 stopped/unloaded; cycle 3 crashed in `ps3-airplay-audio` at a null target. The reported relocated module offset is not confidently mapped to a named function. Cycle-3 unload/module baseline is unknown.
- Do not: claim decoded AirPlay playback, audible output, in-game mixing, or hardware readiness. Do not touch physical PS3s, main, or the published release.
- Blocker: latest instrumented PKG is built but not installed; RPCS3 reports the Mac locked. Earlier two-port run passed cycles 1–2 and crashed cycle 3; exact fault address is not symbolized. Atomic readiness fix has not been run in RPCS3.
- Next: once RPCS3 UI is accessible, install the instrumented PKG, capture trace markers from a rerun, and require three audio-enabled cycles with module count six each; then test synthetic RAOP decode-to-output.
