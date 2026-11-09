# Current scratchpad

- Task: isolate and fix the resident SPRX loader rejection without changing the standalone receiver or Cobra configuration.
- `00.03` physical run: no-import minimal and SDK-macro reference probes each returned `0x80010007` before start. Count stayed at 11 after each failure; harness cleanup returned to baseline 9. One attempt per probe ran.
- Comparison with released webMAN MOD 1.47.50: shared auth/vendor/program type/SCE version, but reference SELF flags/key revision are `0x000A` versus PS3DK `make_sprx` `0x0001`; its ELF has two PT_LOAD segments versus our custom PRX's one RWX PT_LOAD. These differences are leads, not proven causes.
- Built `00.04` package with both diagnostic probes signed by scetool at revision `0x0A`; resident remains unchanged. Local package SHA-256 `af75bf0150629064143117d44f38c184f754704336c62e19c30f68e9e5770e2e`. Package not installed: development console was unreachable from the host.
- Harness now continues through all three configured failed-load attempts and records count after each. Next: install/run `00.04` when console connectivity is available; if probes load, check three full lifecycle cycles and baseline counts. Do not resume resident audio work until the resident itself starts.
