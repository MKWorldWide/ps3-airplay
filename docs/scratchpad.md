# Current scratchpad

- Task: isolate the resident SPRX loader rejection without changing the standalone receiver or Cobra configuration.
- The 00.04 hardware harness failed both diagnostic modules all three times with `0x80010007`; module count stayed at 11 and cleanup returned to baseline 9. Its 50-cycle loop and audio submissions never ran.
- Offline webMAN MOD 1.47.50 source/package comparison is logged in `docs/experimental/RESIDENT_SPRX_REFERENCE_RESEARCH_2026-11-09.md`. Reference SELF key revision is `0x0A`; the 00.04 probes at `0x0A` still failed. Key revision alone is not the fix.
- No resident module start, RAOP-to-PCM evidence, or audible output. Do not call the candidate hardware-ready. User-reported hard shutdown remains uncorrelated to the 00.04 log.
- Next useful experiment: create a tiny no-import probe with the reference Sony CELL SDK PRX-link and scetool pipeline, then compare/load it through the isolated harness. Check toolchain availability first; keep all PS3 changes manual and diagnostic.
