# Improve RSX scanout diagnostics

**Timestamp:** 2026-11-07 00:49 EST

## Scope of the pass

Prime RSX scanout with the spare framebuffer before presenting the primary framebuffer, draw a bright blue `DISPLAY ACTIVE` header and color bars over the public automotive-light background, and record the selected framebuffer dimensions in `SCREEN_READY`. Bump the package version to 01.02 so the candidate is distinguishable from the package already staged on the console.

## Why this pass was needed

The user reported a black screen during a run that played the diagnostic tone and required a long power-button shutdown. A retrieved log contained `SCREEN_READY`, but that event does not prove the display received visible output. PSL1GHT issue [#144](https://github.com/ps3dev/PSL1GHT/issues/144) describes a similar black-screen and unavailable-XMB symptom on real hardware with Evilnat. This project already uses the `-O2` flag mentioned in that report, so the report is a compatibility clue rather than a confirmed cause.

## Files changed

- `docs/ROADMAP.md`
- `docs/TEST_MATRIX.md`
- `docs/decisions/README.md`
- `docs/decisions/2026-11-07_0040_stage_screen_package.md`
- `docs/decisions/2026-11-07_0042_inspect_startup_log.md`
- `docs/decisions/2026-11-07_0049_display_scanout_diagnostic.md`
- `docs/lessons-learned.md`
- `docs/memory.md`
- `docs/reports/2026-11-07_first_hardware_test.md`
- `docs/scratchpad.md`
- `prototype/README.md`
- `prototype/standalone-receiver/source/diagnostic_screen.c`
- `prototype/standalone-receiver/source/main.c`
- `scripts/package-lima-standalone-receiver.sh`

## Architecture / logic changes

The setup still reads and preserves the active video mode and uses the existing two-framebuffer allocation. It now submits and waits for the spare framebuffer first, then submits and waits for framebuffer zero, matching the upstream PSL1GHT `videoTest` startup order. Both buffers carry the supplied image plus a high-contrast banner and exit prompt. `SCREEN_READY` records width as its numeric value; a separate `SCREEN_HEIGHT` event records height. The logger's byte-count field remains zero.

## Runtime behavior changes

If RSX output is visible, a bright blue banner labeled `DISPLAY ACTIVE` and the START exit prompt should be obvious even though the supplied background is mostly black. Controller START and normal resource cleanup are unchanged. No new auto-exit behavior or boot plugin configuration was introduced.

## Validation performed

- Rebuilt the standalone PPU ELF in the existing x86_64 Lima VM using `scripts/build-lima-standalone-receiver.sh`; compile flags included `-O2 -Wall -Wextra -Werror`.
- Packaged version 01.02 using `scripts/package-lima-standalone-receiver.sh`; SFO inspection reported `AIRPLAY02` and `01.02`.
- Package listing and extraction checks passed for the NPDRM EBOOT, SFO, XMB icon, and 2,764,800-byte RGB background.
- ELF SHA-256: `48d0760bfa912889b88e83052b984e8983c9c2af5a6c9a9d7b6ad3d59a7fa2bc`.
- PKG SHA-256: `ddedd7d07fcac3554275b331b821741615019ea2d3a69f9fc9965cbdfbaec94a`; size 3,223,664 bytes.
- Copied the build artifacts back to ignored local `build/` output. No FTP, install, launch, or hardware test was performed.
- The full working-tree and staged diff was reviewed for privacy before commit; the user-provided image remains public by explicit request. `git diff --cached --check` is required at commit time.

## Immediate results

- Version 01.02 builds and its package structure validates in Lima.
- The new banner makes visible output much easier to distinguish from a no-signal or all-black result.
- This does not prove RSX output reaches the PS3 display or fix a hardware compatibility issue. The candidate remains local pending approval for transfer.

## Known limitations / follow-ups

- Install only after exact candidate transfer approval; report whether the banner is visible and whether START returns to XMB.
- If it remains black, capture the new `SCREEN_INIT_FAILED` or `SCREEN_READY` dimensions from a fresh capped log and isolate video output with the upstream sample or a minimal display-only app before changing protocol/audio code.
- Do not resume AirPlay connection attempts until display and clean exit behavior are confirmed.

## Risks introduced

- The new spare-buffer-first flip adds one synchronous frame transition during app startup. Hardware timing and video compatibility remain unverified.
- The `DISPLAY ACTIVE` label is shown only in the framebuffer; it is not a system-level confirmation that HDMI scanout is working.
- An RSX/firmware hang may still prevent controller or PS-button exit; keep the previously documented recovery route available.

## Decision record

### Decision: Prime scanout in the same order as PSL1GHT `videoTest`

- **Context:** Current code used the same video mode setup and `-O2` optimization as the sample, but submitted framebuffer zero as its first flip. The upstream sample first flips buffer one, waits, then proceeds to buffer zero.
- **Options considered:** Keep the current first flip; add the sample's spare-buffer-first prime; abandon RSX and remove the visible screen.
- **Why this option was chosen:** It is a small, reversible alignment with an upstream hardware sample and provides a concrete diagnostic path without changing the selected output mode.
- **Tradeoffs accepted:** It adds a frame of startup time and does not establish that the upstream sample works on this console.

### Decision: Make video output unmistakable during the next run

- **Context:** The user-provided background is mostly black, and a dark image could be confused with failed scanout.
- **Options considered:** Replace the image with a test pattern; keep the image and existing small prompt; retain the image and add a bright banner and color bars.
- **Why this option was chosen:** It preserves the requested image while making the foreground output easy to identify at a glance.
- **Tradeoffs accepted:** The overlay covers narrow strips at the top and bottom of the image.

### Decision: Version the new artifact as 01.02 and leave it local

- **Context:** Version 01.01 is already staged, and the newer package has a different payload and hash.
- **Options considered:** Reuse version 01.01; bump to 01.02 and transfer immediately; bump the version and wait for exact transfer authorization.
- **Why this option was chosen:** A distinct version avoids confusion between the staged and reviewed artifacts; no prior approval covered this new hash.
- **Tradeoffs accepted:** The hardware test waits for approval of the exact new package.

## Next recommended step

After the public commit, request approval to transfer the reviewed 3,223,664-byte 01.02 package with SHA-256 `ddedd7d07fcac3554275b331b821741615019ea2d3a69f9fc9965cbdfbaec94a`. Test only the visible banner and START-to-XMB exit; defer AirPlay until that check succeeds.
