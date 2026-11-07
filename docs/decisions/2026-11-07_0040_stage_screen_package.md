# Stage the screen-enabled PS3 package

**Timestamp:** 2026-11-07 00:40 EST

## Scope of the pass

After the user approved the exact candidate, replaced the staged receiver PKG over FTP with version 01.01. Preserved the previously staged package locally, then downloaded the new remote file and verified exact size, SHA-256, and byte-for-byte equality. Updated deployment records and the next hardware check.

## Why this pass was needed

The previous hardware run played the five-second tone but left the screen black and required a forced power-button shutdown. The new screen-enabled package was built and validated in Lima, then presented with its exact size and digest for approval before overwriting the staged file.

## Files changed

- `docs/TEST_MATRIX.md`
- `docs/ROADMAP.md`
- `docs/memory.md`
- `docs/scratchpad.md`
- `docs/reports/2026-11-07_first_hardware_test.md`
- `docs/decisions/README.md`
- `docs/decisions/2026-11-07_0040_stage_screen_package.md`
- `prototype/README.md`

## Architecture / logic changes

No runtime code changed in this deployment pass. `/dev_hdd0/packages/ps3-airplay-receiver.pkg` now contains the approved version 01.01 package. This package includes the RSX diagnostic background and controller START exit path. Staging a PKG does not install or execute it; the installed title version is not confirmed.

## Runtime behavior changes

The staged file changed from the previous 416,240-byte NPDRM-only package to the 3,222,640-byte version 01.01 screen-enabled package. No app installation, launch, reboot, configuration edit, SPRX transfer, or boot plugin change occurred.

## Validation performed

- FTP connected and the package directory was readable.
- Existing target size before replacement: 416,240 bytes. Downloaded and preserved the previous staged file locally; SHA-256 was `f1b17d07a05679030143e607c74b545592e92ad521135b860bc2aca677160cf2`.
- User approved replacing the staged file with the exact version 01.01 artifact: 3,222,640 bytes; SHA-256 `7824a4b15e4757afad3213a3f55cd3bea816b33795403da12bfd7ede29f93db4`.
- FTP-reported new size: 3,222,640 bytes.
- Downloaded the staged file back and confirmed byte-for-byte equality and the approved SHA-256.
- No unit tests or console runtime tests were performed in this pass.

## Immediate results

- Approved package staging: PASS.
- Version 01.01 install, RSX display, controller START exit, and AirPlay connection: NOT TESTED.
- Existing staged package was preserved locally before it was replaced.

## Known limitations / follow-ups

- The user must install version 01.01 from XMB before its screen and exit path can be tested.
- The current-run capped log from the previous test has not been retrieved.
- iPhone discovery succeeded per user report, but RAOP connection and playback remain unverified.

## Risks introduced

- The staged package on console storage was overwritten, as approved. A local copy of the prior staged package was preserved.
- The new package may still fail during RSX setup or the return-to-XMB path; neither has been confirmed on hardware.

## Decision record

### Decision: Replace only the staged package, not the installed title

- **Context:** The user approved a specified package digest and explicitly limited the requested action to staged-file replacement. The console was FTP-reachable at deployment time.
- **Options considered:** Upload the approved package to the existing path; leave the earlier staged package; install or launch the application as part of the transfer.
- **Why this option was chosen:** It carries out the exact approval while keeping installation and first launch under the user's XMB control.
- **Tradeoffs accepted:** A separate XMB install and hardware validation are still required.

## Next recommended step

Install version 01.01 from XMB. Confirm that the image appears, press controller START once, and report whether the app returns to XMB. If it exits cleanly, retry the iPhone connection once and retrieve the capped log for RAOP diagnosis.
