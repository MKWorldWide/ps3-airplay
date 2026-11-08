# Active work

- v1.0.0 / SFO 01.00 candidate built from `f3da352809f81daac5e3b61a35d7fc18362bf1cc` in the existing Lima VM; full host/sanitizer, PPU, ELF, package, extracted SFO, and asset checks passed.
- Local debug PKG: 3,906,640 bytes, SHA-256 `ef8f0afdacef560e9f5825fa9539247f3eb17e7791282aa6790f086fa8b1288e`, ignored under `work/builds/1.0.0/`.
- Gate remains BLOCKED for publication: source is on `lily/air-ps3-v028`, not current `origin/main`; integrate and rebuild from exact main tip, then perform final release asset privacy review.
- v1.0.0 was FTP-transferred and installed on the development PS3; exact installed SFO/EBOOT verified. It has not been launched. Primary PS3 untouched.
- Git global and repository identity now use the authenticated EsKaye no-reply email. Historical commits still resolve to the prior GitHub account; no public history rewrite performed.
- No tag/release created.

## Next

1. If hardware testing proceeds, launch v1.0.0 on the development unit and observe display, controls, and playback; installation alone is not runtime validation.
2. Integrate the reviewed source into `main`; rebuild from exact `origin/main` before staging a draft.
3. Keep publication blocked until the main-tip candidate and final privacy checks pass.
