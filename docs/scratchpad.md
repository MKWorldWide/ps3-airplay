# Current scratchpad

- Task: isolate the resident SPRX loader rejection without changing the standalone receiver or Cobra configuration.
- The 00.04 hardware harness failed both diagnostic modules all three times with `0x80010007`; module count stayed at 11 and cleanup returned to baseline 9. Its 50-cycle loop and audio submissions never ran.
- Offline webMAN MOD and OpenPS3FTP v5.4 comparisons are logged in `docs/experimental/RESIDENT_SPRX_REFERENCE_RESEARCH_2026-11-09.md`. OpenPS3FTP is a public PS3DK-built VSH plugin whose outer flags word matches our production artifact, but its VSH imports/load context make it unsuitable for direct app-loader comparison. Key revision alone is not the fix.
- No resident module start, RAOP-to-PCM evidence, or audible output. Do not call the candidate hardware-ready. User-reported hard shutdown remains uncorrelated to the 00.04 log.
- Next useful experiment: build an app-context no-import reference using a documented independent PRX pipeline, then compare its module metadata and attempt controlled manual loading through the isolated harness. Keep physical changes manual and diagnostic; do not use Cobra autoload.
