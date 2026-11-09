# Current scratchpad

- Task: resolve the production signed resident SPRX loader rejection without changing the standalone receiver or Cobra configuration.
- Installed diagnostic title `RPRX90002` as SFO `00.02`. Extracted and installed SFO values were independently checked; harness launches and emits a fresh run ID.
- Fresh run still returns `0x80010007` (`ENOEXEC`) on its first `sys_prx_load_module` call. Installed signed SPRX matches the package artifact. Harness foreground audio cleanup returns zero. Local ignored package/log artifacts are under `build/resident-hardware/`.
- Evidence does not identify whether the root cause is SELF signing/key compatibility, PRX format/relocations, or target import resolution. Do not guess at binary/signing changes or substitute `.fake.sprx` for hardware.
- Next: inspect the PS3DK signing path and module loader requirements, then isolate the toolchain format using the smallest safe diagnostic PRX before another install. Keep the physical console at the diagnostic title; no Cobra/autoload changes.
