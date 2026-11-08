# AIR PS3 v1.0.0 Gate 1 status

**Release gate status: PASS**

The user accepted Gate 0 for AIRPLAY02 v0.2.8 / SFO `00.48`: about 45 minutes on the development unit, four XMB open/close cycles, and no reported PCM underflows, ALAC failures, RTP queue drops, concealed/skipped frames, or screen-presentation failures. The user accepted a maximum audio-service interval near 45 ms on XMB resume as a brief hitch. This is user-supplied summarized evidence and has not been independently re-audited from the raw log in this pass.

| Gate | State | Evidence / remaining work |
|---|---|---|
| Gate 0 hardware acceptance | PASS, user accepted | v0.2.8 run and observations above. No fresh console operation is authorized or performed here. |
| Release identity | PASS | `release.json`: app `1.0.0`, SFO `01.00`, AIRPLAY02 / AIR PS3 / AirPS3. Next mapping is `1.0.1` / `01.01`. |
| Exact-source Lima build | PASS | The full pinned release script has passed for the release source. `release-evidence.json` is authoritative for the source SHA, toolchain, required host/sanitizer and PPU checks; the staging helper and workflow require that SHA to match the release commit. |
| Package, SFO, assets | PASS | AIRPLAY02 type-1 debug package identity is `1.0.0` / SFO `01.00`, title `AIR PS3`, service `AirPS3`. The release evidence and checksum asset are authoritative for the exact package, ELF, SELF, SFO, artwork, envelope, and extracted contents. A prior build checkpoint is documented [here](reports/air_ps3_v1_0_0_main_build_checkpoint.md). |
| Privacy review | PASS | Scanned all reachable branch/tag-history blobs for local paths, private IPv4, MAC addresses, emails, and credential patterns; no matches in those categories. One embedded AirPort Express RAOP compatibility key is intentionally present and attributed in the third-party notices. Staged docs and the exact candidate assets were manually reviewed; no secret scanner was installed because none was available. |
| License/distribution review | PASS for documented components | Project MIT, ALAC notice, OpenAirPlay key-data notice, Mbed TLS 3.6.7 license, and PS3DK/PSL1GHT/RSX license texts are included in the repository and release asset list. The user explicitly authorized the supplied artwork for public project/PKG use; no broader image license is asserted. |
| Release branch integration | PASS | Reviewed release work was fast-forwarded into `main`, and the merged feature branch was removed after its commits were confirmed reachable from `main`. No history rewrite or force-push was used in this integration. |
| GitHub tag/release | READY TO STAGE | No v1.0.0 tag or release exists yet. Stage the exact verified assets with `scripts/stage-github-release.sh`, then publish only through the validated manual workflow. |
| v1.0.0 development-unit installation | PASS | Uploaded by FTP, downloaded back byte-for-byte, installed through webMAN, and verified by exact installed PARAM.SFO/EBOOT hashes. No launch was performed during this install step; the maintainer's later qualitative run report is separate. The primary PS3 was not changed. |
| v1.0.0 maintainer hardware report | PASS, qualitative report | Maintainer says v1.0.0 is running beautifully on the development PS3. No duration, measurements, fresh log, or specific playback/control checks were supplied; no broader hardware claim is inferred. |

## Candidate build path

`release.json` is the version source. `scripts/release-lima-standalone-receiver.sh` produces the standalone debug PKG and, only after every build stage passes, writes local `release-evidence.json` and `SHA256SUMS.txt` under ignored build output. `scripts/verify-release-evidence.py` checks exact source/version identity, recorded validation stages, package size/hash, package type, content ID, and evidence records. The Lima build gate has already extracted and compared package contents before producing that evidence. `scripts/stage-github-release.sh` refuses to create a tag or draft while this document is not PASS, verifies the exact candidate evidence and hashes, and requires the source commit to be integrated into `origin/main`. The manual `.github/workflows/publish-release.yml` validates the draft asset set and exact source before publishing; only its `publish` job receives `contents: write`.

The package format is a PS3 type-1 debug PKG. The workflow is intentionally a publisher, not a PS3 compiler: the pinned PS3DK/PSL1GHT/Mbed TLS build runs in the existing Lima environment. GitHub-hosted Actions does not receive the local toolchain or a self-hosted runner. The locally generated candidate is accepted only when its exact source revision and evidence match.

## Publication sequence

1. Stage the exact-source candidate and reviewed documentation/notices as a draft.
2. Dispatch `.github/workflows/publish-release.yml` from `main` with tag `v1.0.0`, exact source revision, and package SHA-256.
3. Verify the workflow succeeded and the published release contains the exact expected assets.

This gate records prepublication readiness; a passing local gate is not itself a published GitHub Release.

Build evidence and host-only boundary are recorded in [the candidate build report](reports/air_ps3_v1_0_0_candidate_build.md); that build pass did not change either PS3. The later user-reported hardware observation is separately recorded [here](reports/air_ps3_v1_0_0_user_report.md).

The commit identity correction and rewritten public refs are documented in [the attribution migration report](reports/git_author_attribution_migration.md).
