# Active work

- v1.0.0 / SFO 01.00 candidate built from former commit `f3da352809f81daac5e3b61a35d7fc18362bf1cc`; after the author rewrite, its identical source tree is `a1edbba176f31076eccd10d2c627916963cb5605`. Full host/sanitizer, PPU, ELF, package, extracted SFO, and asset checks passed; artifact evidence still references the old SHA.
- Local debug PKG: 3,906,640 bytes, SHA-256 `ef8f0afdacef560e9f5825fa9539247f3eb17e7791282aa6790f086fa8b1288e`, ignored under `work/builds/1.0.0/`.
- Gate remains BLOCKED for publication: source is on `lily/air-ps3-v028`, not current `origin/main`; integrate and rebuild from exact main tip, then perform final release asset privacy review.
- v1.0.0 was FTP-transferred and installed on the development PS3; exact installed SFO/EBOOT verified. It has not been launched. Primary PS3 untouched.
- Git global/repository identity and all reachable GitHub branch/tag history now resolve to EsKaye. A verified local pre-rewrite bundle is retained; external clones/caches may retain old hashes.
- No tag/release created.

## Next

1. If hardware testing proceeds, launch v1.0.0 on the development unit and observe display, controls, and playback; installation alone is not runtime validation.
2. Integrate the reviewed source into `main`; rebuild from the exact rewritten `origin/main` before staging a draft.
3. Keep publication blocked until the main-tip candidate and final privacy checks pass.
