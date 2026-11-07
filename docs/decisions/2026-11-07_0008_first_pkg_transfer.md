# Stage the first standalone receiver package

**Timestamp:** 2026-11-07 00:08 EST

## Scope of the pass

Verified PS3 FTP preconditions, transferred the validated standalone receiver package, checked the remote file size, downloaded it back, and recorded the first deployment checkpoint without installing or launching the app.

## Why this pass was needed

The first hardware validation requires testing the existing standalone receiver on the actual PS3. User authorization for the PKG transfer was provided after the console IP was supplied.

## Files changed

- `docs/TEST_MATRIX.md`
- `docs/ROADMAP.md`
- `docs/memory.md`
- `docs/scratchpad.md`
- `docs/README.md`
- `docs/reports/2026-11-07_first_hardware_test.md`
- `docs/decisions/2026-11-07_0008_first_pkg_transfer.md`
- `docs/decisions/README.md`

## Architecture / logic changes

No runtime code changed. The report separates local validation, FTP transfer, installation, launch, audible output, and AirPlay interoperability into distinct evidence states.

## Runtime behavior changes

The validated PKG now exists in the console's `/dev_hdd0/packages/` staging directory. The receiver is not installed or running.

## Validation performed

- Confirmed the webMAN FTP service responded.
- Read-only preflight found 9.2 GB free, an empty package staging directory, no existing `AIRPLAY02` title directory, and no existing receiver log.
- Uploaded only `ps3-airplay-receiver.pkg` (2,560,368 bytes).
- Confirmed remote listing and size, downloaded the package back, compared bytes, and verified SHA-256 `1b797033a25bbf95f7deb8ec9e95f5d12c66edcf2fd00b2f6ceb4a9e990704c2`.
- No installation, launch, reboot, or setting change was performed.

## Immediate results

- Package transfer: PASS.
- Installation and all application runtime/audio/AirPlay checks: NOT TESTED.
- No console instability was observed because the application has not been run.

## Known limitations / follow-ups

- The package contains no project icon; XMB icon appearance remains unverified.
- Remote FTP did not expose a hash command. Integrity was verified by download-back byte comparison and local SHA-256.
- User must install from the XMB Package Manager and report the TV-observed app, launch, exit, and tone behavior. Do not reboot without user approval; stop and report serious failures.

## Risks introduced

- The package occupies approximately 2.56 MB in `/dev_hdd0/packages/` until installed or deleted.
- No application execution risk has been exercised yet.

## Decision record

### Decision: Stage the PKG but stop before installation

- **Context:** The user authorized the package transfer, and the required read-only checks passed.
- **Options considered:** Transfer the package and install remotely; stage it and let the user use the existing XMB Package Manager; stop before writing any package file.
- **Why this option was chosen:** User instructions explicitly call for XMB installation guidance and require observing launch/audio behavior on the TV. The app has not yet established a tested recovery path for unattended installation or launch.
- **Tradeoffs accepted:** The user performs one manual installation step before hardware validation can continue.

## Next recommended step

Guide the user through XMB installation, then continue diagnostics only after they report what appeared and whether the app launched and produced audible sound.
