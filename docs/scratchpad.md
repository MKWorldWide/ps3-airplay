# Current scratchpad

- Task: complete publication of the existing AIR PS3 v1.0.0 release; do not start next-version features or touch a PS3/RPCS3.
- Release remains draft. Tag `v1.0.0` is pinned to source `e1fdb44b03e2beb048954aded0eda5ae11248215`. PKG: 3,906,640 bytes; SHA-256 `b815536168c5c1994ff6a5bb79f4aa16ffed6720691239bd9066e0a9e2075c43`; app `1.0.0`, SFO `01.00`.
- Evidence correction: Actions runs 42468111811 and 42468417377 failed at `gh release view` with `release not found`. Run 42468710312 removed that CLI step, then the Releases API returned no matching draft with validation permission `contents: read`. It did not reach asset listing/download.
- GitHub docs state draft-release lists are visible only to actors with push access. Current change grants `contents: write` to validation for GET-only draft/asset inspection; publish remains a separate mutation job. Verify this by rerunning before claiming root cause fixed.
- Next: parse YAML, inspect staged diff, run privacy gate, commit/push as EsKaye, dispatch again against immutable tag/source/hash, and inspect exact stage. Publish only after all artifact checks pass; then verify public URL, asset list and PKG hash.
- No console was changed. Keep draft unpublished if runner access or validation fails.
