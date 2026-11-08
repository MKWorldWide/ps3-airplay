# Active work

- Prepare AIR PS3 v1.0.0 / SFO 01.00 candidate and a fail-closed GitHub release path.
- Gate 0 is user accepted for v0.2.8 (~45 minutes, four XMB cycles); accepted known hitch: ~45 ms service interval on XMB resume. Evidence is a user summary, not a fresh raw-log audit.
- `release.json` is version source; next mapping is app 1.0.1 / SFO 01.01. Existing branch/worktree remains unchanged.
- Full Lima build still needs the exact committed source archive; guest has >8 GiB free and pinned tools are present.
- Source/doc privacy and documented license notices pass review; the generated bundle still needs its final privacy check. Release staging remains blocked until the Lima package gate passes and exact source is integrated into origin/main. Do not create a tag or GitHub Release while blocked.
- No PS3 is to be installed, restarted, or changed.

## Next

1. Finish the concise sanitation review, commit, and push only the existing branch.
2. Transfer the committed source archive to the existing Lima VM; run the release gate and verify package/SFO/evidence.
3. Record results and keep the release blocked if any required gate remains unresolved.
