# Current scratchpad

- Task: finish public v1.0.0 release validation and publish; do not begin later feature work or touch either PS3.
- `main` is current; reviewed release work was fast-forwarded from the feature branch. The exact-main checkpoint build at `38f13ba2b3255352a9957160c3d075e5426cd069` passed; because this documentation pass changes the source SHA, repeat the full Lima build from the final committed main revision.
- Historical hardware: user says v1.0.0 is “running beautifully”; this is qualitative only. No duration/log/individual checks inferred. Issue #1/#2 closed on historical launch/tone/idle exit; #3–#6 remain open.
- Whole-ref scan found no local paths, private IPs, MAC addresses, emails, or credential patterns. One intentional, attributed public RAOP compatibility key is in source. Final candidate asset review and GitHub surface checks remain before staging.
- Next: commit/push these reviewed docs, verify EsKaye authorship, remove the now-merged release branch reference as requested, rerun the pinned Lima release gate at the new main SHA, rescan exact assets, stage the draft, publish through the manual workflow, and verify URLs/hashes.
