# Replace the staged NPDRM package

**Timestamp:** 2026-11-07 00:17 EST

## Scope of the pass

Uploaded the corrected standalone receiver PKG over FTP after explicit approval to replace the existing staged file. Downloaded it back and verified its size and SHA-256. Updated the first hardware report with this replacement checkpoint.

## Why this pass was needed

The first installed package failed to start with `80029530`. The old package used a generic SELF wrapper rather than PSL1GHT's NPDRM signer with a matching content ID. The user approved replacing the staged PKG after seeing the new hash and existing-file conflict.

## Files changed

- `docs/TEST_MATRIX.md`
- `docs/ROADMAP.md`
- `docs/memory.md`
- `docs/scratchpad.md`
- `docs/reports/2026-11-07_first_hardware_test.md`
- `docs/decisions/2026-11-07_0017_replace_staged_pkg.md`
- `docs/decisions/README.md`

## Architecture / logic changes

No runtime code changed in this pass. The console staging path now contains the corrected NPDRM PKG, while the installed `AIRPLAY02` title remains the first failed-to-launch build until the user reinstalls or updates it from XMB.

## Runtime behavior changes

The PKG at `/dev_hdd0/packages/ps3-airplay-receiver.pkg` was replaced. Its NPDRM EBOOT was generated with `make_self_npdrm` using `UP0001-AIRPLAY02_00-0000000000000000`; the package also includes the PSL1GHT default XMB icon.

## Validation performed

- User approved replacement of the existing 2,560,368-byte PKG with the corrected 416,240-byte candidate.
- FTP listing confirmed the package path.
- FTP-reported remote size: 416,240 bytes.
- Downloaded the remote file back, compared it byte-for-byte against the local package, and verified SHA-256 `f1b17d07a05679030143e607c74b545592e92ad521135b860bc2aca677160cf2`.
- No installation, launch, reboot, or settings change was performed in this pass.

## Immediate results

- Corrected package staging: PASS.
- Corrected app installation/launch, audio, logging, and AirPlay behavior: NOT TESTED.
- Installed title still contains the original build that returned `80029530`.

## Known limitations / follow-ups

- User must update or reinstall the corrected package from XMB and report any prompt or failure.
- Retrieve the new diagnostic log after launch; no log has been created by the failed attempt.
- The corrected package has not been executed on the PS3, so the NPDRM diagnosis remains a strong hypothesis rather than a confirmed fix.

## Risks introduced

- The previous staged PKG was overwritten with the approved replacement.
- The installed old title remains until XMB update/reinstall and may still fail to launch.

## Decision record

### Decision: Replace the staged package and leave the installed title for user-controlled update

- **Context:** The user approved replacing the existing staged PKG after reviewing the new digest and the overwrite conflict.
- **Options considered:** Overwrite the staged package and install remotely; overwrite only the staged package and let the user update from XMB; keep the old package.
- **Why this option was chosen:** It preserves the requested XMB interaction and lets the user observe installation and launch on the TV, while the package file can be verified independently over FTP.
- **Tradeoffs accepted:** A manual XMB update/reinstall is still needed before the corrected executable is tested.

## Next recommended step

Guide the user through the XMB update/reinstall, then collect the visible launch result and retrieve the capped startup log over FTP.
