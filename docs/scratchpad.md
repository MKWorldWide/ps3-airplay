# Current scratchpad

- Task: build and install a verbose resident test package on the development PS3, staying on `experimental/resident-airplay` and avoiding Cobra/autoload changes.
- Built fresh signed PRX and SPRX with the bounded diagnostic journal. Built a unique `RPRX90002` PKG with the signed SPRX embedded byte-for-byte; `prx-gen check`, package listing, and extraction/hash checks passed. Artifacts stay ignored under `build/resident-hardware/`.
- Lima host crypto, RAOP, RTSP, encrypted RTP, ALAC, PCM, resampling, and sanitizer checks passed. The final test harness compiles for 50 lifecycle cycles. The PS3 install/run is pending.
- Next: run the targeted host format/diagnostics checks, review and push the branch diff with docs, upload the new PKG under a unique console filename, install through webMAN, launch only the new diagnostic title, then retrieve its bounded harness log read-only and compare final module count against baseline.
- Do not call installation or lifecycle successful until console log evidence is retrieved; no network stream, audible playback, or Cobra compatibility is established by package build alone.
