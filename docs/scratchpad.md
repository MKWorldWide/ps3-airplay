# Current scratchpad

- Task: complete publication of the existing AIR PS3 v1.0.0 release; do not start next-version features or touch a PS3/RPCS3.
- Release remains draft. Tag `v1.0.0` is pinned to source `e1fdb44b03e2beb048954aded0eda5ae11248215`. PKG: 3,906,640 bytes; SHA-256 `b815536168c5c1994ff6a5bb79f4aa16ffed6720691239bd9066e0a9e2075c43`; app `1.0.0`, SFO `01.00`.
- Evidence correction: Actions runs 42468111811 and 42468417377 both failed at `gh release view TAG --json isDraft` with `release not found`. Neither reached `gh release download` nor API asset retrieval. Do not infer why the Actions token cannot resolve the draft.
- Current workflow removes that CLI check and uses Releases API lookup plus asset IDs, exact inventory/state/size validation, existing checksum/evidence verification, and only then publish. Local API success does not prove Actions-token access.
- Next: validate YAML and diff, run privacy gate, commit/push as EsKaye, dispatch against the immutable tag/source/hash, inspect exact failing stage. Publish only after all validation steps pass; then verify public URL, asset list and PKG hash.
- No console was changed. Keep draft unpublished if runner access or validation fails.
