# Active work

- v1.0.0 / SFO 01.00 candidate built from former commit `f3da352809f81daac5e3b61a35d7fc18362bf1cc`; after the author rewrite, its identical source tree is `a1edbba176f31076eccd10d2c627916963cb5605`. Full host/sanitizer, PPU, ELF, package, extracted SFO, and asset checks passed; artifact evidence still references the old SHA.
- Local debug PKG: 3,906,640 bytes, SHA-256 `ef8f0afdacef560e9f5825fa9539247f3eb17e7791282aa6790f086fa8b1288e`, ignored under `work/builds/1.0.0/`.
- Current task: polish public-facing v1.0.0 release/docs, integrate existing work into `main`, rebuild from exact main tip, validate release assets, and publish if every gate passes. Do not begin next-version development.
- The development unit has v1.0.0 installed; the maintainer now reports it is “running beautifully.” No duration, log, or specific playback/control checks were supplied. Do not alter either PS3 during this task.
- Git global/repository identity and all reachable GitHub branch/tag history now resolve to EsKaye. A verified local pre-rewrite bundle is retained; external clones/caches may retain old hashes.
- No tag/release has been created yet.

## Next

1. Complete public-facing docs and issue-status reconciliation with evidence boundaries.
2. Fast-forward `main` from the reviewed feature branch, then run the exact-main Lima build and full release checks.
3. Stage and publish v1.0.0 only if the release workflow and exact asset checks pass.
