# Current scratchpad

- Task: isolate the resident SPRX loader rejection without changing the standalone receiver or Cobra configuration.
- The 00.04 hardware harness failed both diagnostic modules all three times with `0x80010007`; module count stayed at 11 and cleanup returned to baseline 9. Its 50-cycle loop and audio submissions never ran.
- Offline webMAN MOD, OpenPS3FTP v5.4, and the pinned PS3DK app-context sample pair are logged in `docs/experimental/RESIDENT_SPRX_REFERENCE_RESEARCH_2026-11-09.md`. The tested EBOOT and production SPRX match at outer flags `0x00010001`; this does not validate full signing metadata. PS3DK v0.27.0 notes its `make_sprx` output was untested on hardware.
- No resident module start, RAOP-to-PCM evidence, or audible output. Do not call the candidate hardware-ready. User-reported hard shutdown remains uncorrelated to the 00.04 log.
- Next useful experiment: build the exact PS3DK v0.27.0 `hello-sprx-import`/`hello-sprx-export` pair with matching real-signed containers and EBOOT, then test load/start/handoff/stop/unload in the isolated manual harness. Do not enable Cobra autoload.
