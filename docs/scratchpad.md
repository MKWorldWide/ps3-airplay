# Current scratchpad

- Task: publish existing AIR PS3 v1.0.0; no next-version work and no PS3/RPCS3 operations.
- Release source/tag `e1fdb44b03e2beb048954aded0eda5ae11248215`; exact Lima build passed. PKG is 3,906,640 bytes, SHA-256 `b815536168c5c1994ff6a5bb79f4aa16ffed6720691239bd9066e0a9e2075c43`, app `1.0.0`, SFO `01.00`.
- All 10 release assets are in a draft, hash-checked locally. Draft tag is v1.0.0; its browser URL is still synthetic until publication. No release is published.
- First Actions run verified tag/source, then failed because tag-based `gh release download` could not resolve draft assets with the workflow token. Authenticated asset-ID download succeeded locally. Workflow is patched to fetch draft assets through the Releases API, check exact set/state/sizes, then run original hashes/evidence checks.
- Next: privacy-review and commit/push workflow/docs fix to main, dispatch workflow again with tag SHA and package hash, wait for validation/publish, verify release URL and exact asset hashes. Tag stays pinned to its verified release-source commit; the workflow-only follow-up commit does not change the package source.
- Issues #1/#2 closed on historical evidence; #3–#6 open. No console changed.
