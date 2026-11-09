# Current scratchpad

- Task: build and install a verbose resident test package on the development PS3, staying on `experimental/resident-airplay` and avoiding Cobra/autoload changes.
- Built fresh signed PRX and SPRX with the bounded diagnostic journal. Installed the first `RPRX90002` package; its console-side SPRX matches the signed build, but the user reported the prior launch error. The first SFO used `0.1.0`.
- Corrected both SFO version fields to `00.01`, rebuilt, and independently parsed the package SFO. Corrected PKG hash: `54d5d299c6831a18e7c0474aac3577a8a02de443a7a3b98e03f64d242b9010b9`. Artifacts remain ignored under `build/resident-hardware/`.
- Next: commit and push the SFO correction, upload/install the corrected package as an update to `RPRX90002`, launch it, then retrieve the harness log read-only and compare every cycle's final module count to baseline.
- No lifecycle, RAOP, audible output, or Cobra compatibility success is established until the corrected app's log and any user-observed launch state are checked.
