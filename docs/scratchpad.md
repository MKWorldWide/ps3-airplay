# Current scratchpad

- Task: build and install a verbose resident test package on the development PS3, staying on `experimental/resident-airplay` and avoiding Cobra/autoload changes.
- Built fresh signed PRX and SPRX with the bounded diagnostic journal. Installed the first `RPRX90002` package; its console-side SPRX matches the signed build, but the user reported the prior launch error. The first SFO used `0.1.0`.
- Corrected both SFO version fields to `00.01`, rebuilt, and independently parsed the package SFO. Corrected PKG hash: `54d5d299c6831a18e7c0474aac3577a8a02de443a7a3b98e03f64d242b9010b9`. Artifacts remain ignored under `build/resident-hardware/`.
- The corrected title is installed with SFO `00.01`; its verbose harness starts. `sys_prx_load_module` returns `0x80010007` (`ENOEXEC`) for the exact signed SPRX hash, before `module_start`. The harness records failure on cycle 1 and closes its foreground audio port cleanly.
- Branch commits through `0fd76a3` are pushed. The corrected package hash is `54d5d299c6831a18e7c0474aac3577a8a02de443a7a3b98e03f64d242b9010b9`; the console log is in ignored build output.
- Next: inspect PS3DK PRX signing/format requirements and import compatibility against this exact `ENOEXEC`; make a targeted fix only when the loader cause is evidenced. Keep the current title and package available. Do not swap in the RPCS3 fake or claim any resident lifecycle/audio result.
- Current result: SFO gate passes; resident loader gate fails. No RAOP, resident PCM, sound, 50-cycle cleanup, or Cobra behavior was exercised.
