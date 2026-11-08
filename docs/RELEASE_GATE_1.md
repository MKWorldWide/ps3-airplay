# AIR PS3 v1.0.0 Gate 1 status

**Release gate status: BLOCKED**

The user accepted Gate 0 for AIRPLAY02 v0.2.8 / SFO `00.48`: about 45 minutes on the development unit, four XMB open/close cycles, and no reported PCM underflows, ALAC failures, RTP queue drops, concealed/skipped frames, or screen-presentation failures. The user accepted a maximum audio-service interval near 45 ms on XMB resume as a brief hitch. This is user-supplied summarized evidence and has not been independently re-audited from the raw log in this pass.

| Gate | State | Evidence / remaining work |
|---|---|---|
| Gate 0 hardware acceptance | PASS, user accepted | v0.2.8 run and observations above. No fresh console operation is authorized or performed here. |
| Release identity | PASS | `release.json`: app `1.0.0`, SFO `01.00`, AIRPLAY02 / AIR PS3 / AirPS3. Next mapping is `1.0.1` / `01.01`. |
| Exact-source Lima build | PENDING | Must run after implementation is committed; record full source SHA, toolchain, ELF/PKG hashes and extracted metadata. |
| Package, SFO, assets | PENDING | Require the complete package extraction and content checks from the release gate. |
| Privacy review | PASS for this source/doc pass | Full staged diff and changed files reviewed; targeted local-path, private IPv4, MAC, email, and secret-assignment scans returned no matches. No secret-scanner binary was available, so none was installed. Raw logs, local build directories, and agent instructions are excluded. Exact built release assets still require a final review before staging. |
| License/distribution review | PASS for documented components | Project MIT, ALAC notice, OpenAirPlay key-data notice, Mbed TLS 3.6.7 license, and PS3DK/PSL1GHT/RSX license texts are included in the repository and release asset list. The user explicitly authorized the supplied artwork for public project/PKG use; no broader image license is asserted. |
| Release branch integration | PENDING | Current work is on the preserved feature branch; integrate the verified candidate into main and build from the exact current `origin/main` tip before publishing. |
| GitHub tag/release | NOT CREATED | Do not create a tag or release until all required gates pass. |
| PS3 installation | NOT PERFORMED | Neither PS3 is to be installed, restarted, or altered for this task. |

## Candidate build path

`release.json` is the version source. `scripts/release-lima-standalone-receiver.sh` produces the standalone debug PKG and, only after every build stage passes, writes local `release-evidence.json` and `SHA256SUMS.txt` under ignored build output. `scripts/verify-release-evidence.py` checks exact source/version identity, recorded validation stages, package size/hash, package type, content ID, and evidence records. The Lima build gate has already extracted and compared package contents before producing that evidence. `scripts/stage-github-release.sh` refuses to create a tag or draft while this document is not PASS, verifies the exact candidate evidence and hashes, and requires the source commit to be integrated into `origin/main`. The manual `.github/workflows/publish-release.yml` validates the draft asset set and exact source before publishing; only its `publish` job receives `contents: write`.

The package format is a PS3 type-1 debug PKG. The workflow is intentionally a publisher, not a PS3 compiler: the pinned PS3DK/PSL1GHT/Mbed TLS build runs in the existing Lima environment. GitHub-hosted Actions does not receive the local toolchain or a self-hosted runner. The locally generated candidate is accepted only when its exact source revision and evidence match.

## Remaining release blockers

1. Run the exact-source Lima host/sanitizer, PPU, ELF, SFO, artwork, extraction, and package-envelope gate from the clean current `main` tip.
2. Re-scan the exact generated candidate and release assets before staging; keep private logs, local paths, and machine identifiers out.
3. Keep the documented third-party notices, license texts, installation guidance, and release notes attached to the draft.
4. Integrate the selected candidate into `main`, rebuild from that exact tip, then stage a draft. Publish only through the deliberate workflow dispatch.

No tag or GitHub Release was created for this blocked candidate.
