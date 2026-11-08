# Current scratchpad

- Task: continue isolated resident AirPlay work on `experimental/resident-airplay`; do not touch `main`, public release assets, or physical PS3s.
- Completed: installed and ran the fake-SPRX `RPRX90001` RPCS3 harness. Three start/ready/stop/unload cycles passed; module list returned to six after each.
- Current limitations: RPCS3 rejects raw PRX (`0x80011148`) and cannot open the resident log; harness diagnostics provide lifecycle evidence only. The harness app's `cellAudio` path accepted a 5-second test tone, but resident PCM output, RAOP-in-RPCS3, and foreground coexistence are untested.
- Candidate output/hashes: ignored `build/rpcs3-candidate/`; current report: `docs/experimental/RESIDENT_OVERNIGHT_REPORT.md`.
- Next: wire a bounded PCM queue and playback worker into the resident using the tested `cellAudio` path, then validate generated PCM and teardown in RPCS3. Hardware remains NOT READY.
