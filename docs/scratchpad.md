# Current scratchpad

- Task: qualify resident audio on `experimental/resident-airplay`; stable release and physical consoles remain out of scope.
- Verified: full Lima crypto/protocol/build gate passes against hash-matched sources, including encrypted RAOP through SPSC and 44.1-to-48 kHz rendering (256 non-silent frames, no underflow). Prior host lifecycle mocks, SPSC concurrency, PPU PRX structure, and package identity checks pass.
- Candidate: latest ignored RPCS3 package SHA-256 `d803bfd2daf94e6dbf7a61434330077f5e5b55418831d389a12ffbf0fb888a3c`; embedded fake SPRX matched the built output. `audio_thread_started` uses atomic publication/read semantics.
- Blocker: previous audio-enabled RPCS3 run passed cycles 1–2 and crashed in the resident audio thread on cycle 3; exact call site and post-cycle module count are unknown. Latest instrumented package is not yet run because RPCS3 reports the Mac locked.
- Next: after unlock, run latest RPRX90001 package, collect trace markers, and complete three audio-enabled stop/unload cycles with module count six each; then test synthetic RAOP through resident output. Hardware readiness remains NOT READY.
