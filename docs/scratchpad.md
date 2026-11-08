# Active work

- v1.0.0 / SFO 01.00 candidate built from `f3da352809f81daac5e3b61a35d7fc18362bf1cc` in the existing Lima VM; full host/sanitizer, PPU, ELF, package, extracted SFO, and asset checks passed.
- Local debug PKG: 3,906,640 bytes, SHA-256 `ef8f0afdacef560e9f5825fa9539247f3eb17e7791282aa6790f086fa8b1288e`, ignored under `work/builds/1.0.0/`.
- Gate remains BLOCKED for publication: source is on `lily/air-ps3-v028`, not current `origin/main`; integrate and rebuild from exact main tip, then perform final release asset privacy review.
- No tag/release created; neither PS3 was touched.

## Next

1. Review and commit this build report/status update, then push only the existing feature branch.
2. Obtain integration into `main`; rebuild from exact `origin/main` before staging a draft.
3. Keep publication blocked until the main-tip candidate and final privacy checks pass.
