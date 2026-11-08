# Install AIRPLAY02 v0.2.6 on the PS3

**Timestamp (EDT):** 2026-11-07 23:26

## Scope of the pass

Uploaded the validated v0.2.6 PKG, verified the remote bytes, installed through webMAN, and checked installed metadata.

## Why this pass was needed

The release gate for v0.2.6 passed; physical resume and audio checks require installing this exact build.

## Files changed

- `docs/memory.md`
- `docs/scratchpad.md`
- `docs/decisions/2026-11-07_2326_v026_hardware_install.md`

## Architecture / logic changes

No source or architecture changes. Installed the existing AIRPLAY02 title update.

## Runtime behavior changes

The console now has AIRPLAY02 SFO 00.46 installed. The app remains at XMB; this pass did not launch it.

## Validation performed

- Confirmed webMAN and FTP reachable.
- Retrieved the previous log, then exited the running v0.2.5 app through webMAN. Its log records `SYSUTIL_EXIT_GAME`, full screen/audio/receiver cleanup, and `APP_STOP`.
- Uploaded `ps3-airplay-receiver-0.2.6.pkg`; FTP download-back matched the local file byte-for-byte: 3,233,072 bytes, SHA-256 `472915766a1b950f44558259a3583b5931c0bb3f8d3a8b0e5f1a044715927ca6`.
- webMAN installation returned HTTP 200. FTP-read installed PARAM.SFO identifies AIRPLAY02 / 00.46. Installed EBOOT size is 452,592 bytes, equal to the validated package's extracted EBOOT size.

## Immediate results

Package transfer and installation verified. No v0.2.6 startup, visible output, XMB resume, controller, or audio behavior is claimed.

## Known limitations / follow-ups

The bounded post-menu redraw is validated with host presenter mocks only. Test physical PS-menu visibility and resume, then START exit and short AirPlay playback.

## Risks introduced

The installed AIRPLAY02 title has been replaced by v0.2.6. Previous validated packages and the local artifact remain available for rollback. No firmware, boot configuration, resident module, or unrelated title was modified.

## Decision record

- **Decision:** Install v0.2.6 after byte-verifying its FTP copy.
- **Context:** User directly requested installation; the exact package had passed the Lima release gate.
- **Options considered:** Install immediately after the exact transfer check, or defer for another approval.
- **Why this option was chosen:** The standing authorization covers automatic PS3 package installation, and this is the requested next step.
- **Tradeoffs accepted:** The AIRPLAY02 title is updated before the user observes its runtime behavior; rollback artifacts are retained.

## Next recommended step

Launch v0.2.6 once from XMB and record display/resume, START exit, and short-stream results separately.
