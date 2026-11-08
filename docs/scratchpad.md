# Current scratchpad

- Task: publish existing AIR PS3 v1.0.0 only; no next-version work and no PS3/RPCS3 operations.
- `main` is current at `b33241e`; EsKaye author verified. The merged `lily/air-ps3-v028` branch was removed locally and remotely; historical fidelity and forensic branches remain.
- Full pinned-Lima build from `b33241e` passed. Exact PKG: 3,906,640 bytes, SHA-256 `74a2376c159d12809b5fbba7a0f7ef868d1877eafcfcb8b07b97730484290d21`, SFO `01.00`. Do not use this artifact after the next docs commit; rebuild from new HEAD.
- Public refs scan: 116 commits resolve as EsKaye (115) and MKWorldWide (1 initial org commit); no local paths, private IPv4, MACs or emails. One intentional third-party RAOP compatibility key is present and attributed. The package email-pattern matches are encrypted payload noise; no hits in extracted EBOOT/text assets.
- Issues #1/#2 closed with historical evidence boundaries; #3–#6 open. No PRs, releases, workflow runs, wiki, discussions, LFS or submodules were found. Public art use was explicitly authorized.
- The draft helper rejected the previous gate header. The required exact `**Release gate status: PASS**` marker is restored. Next: commit/push this correction, rebuild from final main, rerun asset scan, stage draft, dispatch and verify workflow/release.
