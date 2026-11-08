# Current scratchpad

- Task: continue isolated resident AirPlay work on `experimental/resident-airplay`; do not touch `main`, public release assets, or physical PS3s.
- Completed: installed and ran the fake-SPRX `RPRX90001` RPCS3 harness. Three start/ready/stop/unload cycles passed; module list returned to six after each.
- Current limitations: RPCS3 rejects raw PRX (`0x80011148`) and cannot open the resident log; harness diagnostics provide lifecycle evidence only. No RAOP session, resident PCM output, or foreground coexistence test yet.
- Candidate output/hashes: ignored `build/rpcs3-candidate/`; current report: `docs/experimental/RESIDENT_OVERNIGHT_REPORT.md`.
- Next: probe PS3 `cellAudio` port startup and output in RPCS3, then decide whether a bounded resident playback queue is supportable. Hardware remains NOT READY.
