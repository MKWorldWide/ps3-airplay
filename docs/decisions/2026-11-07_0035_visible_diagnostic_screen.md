# Add a visible diagnostic screen and clean app exit

**Timestamp:** 2026-11-07 00:35 EST

## Scope of the pass

Added the maintainer-approved background artwork as a public asset, initialized a PS3 RSX framebuffer for the standalone receiver, drew an on-screen controller exit prompt, added controller START polling and ordered cleanup, added screen initialization result codes to the capped log, and packaged the artwork as a 1280x720 RGB runtime asset. Bumped the XMB package version to 01.01 so it is presented as an update candidate.

## Why this pass was needed

The user reported that the receiver played its five-second diagnostic tone but left the screen black, did not return to XMB, and had to be shut down with a long power-button press. The iPhone displayed `ps3-airplay-diagnostic` but reported that it could not connect. The current-run log is not yet available.

## Files changed

- `README.md`
- `assets/README.md`
- `assets/diagnostic-background.jpg`
- `docs/ARCHITECTURE.md`
- `docs/ROADMAP.md`
- `docs/TEST_MATRIX.md`
- `docs/lessons-learned.md`
- `docs/memory.md`
- `docs/reports/2026-11-07_first_hardware_test.md`
- `docs/scratchpad.md`
- `docs/decisions/README.md`
- `docs/decisions/2026-11-07_0035_visible_diagnostic_screen.md`
- `prototype/README.md`
- `prototype/standalone-receiver/Makefile`
- `prototype/standalone-receiver/README.md`
- `prototype/standalone-receiver/source/main.c`
- `prototype/standalone-receiver/source/diagnostic_screen.c`
- `prototype/standalone-receiver/source/diagnostic_screen.h`
- `scripts/package-lima-standalone-receiver.sh`
- `scripts/prepare-diagnostic-background.py`

## Architecture / logic changes

The standalone app now loads a fixed-size RGB image from its installed USRDIR, initializes the active video mode with PSL1GHT RSX APIs, creates two display buffers, scales the image to the display size, and draws `PRESS START TO EXIT` and `PS3 AIRPLAY DIAGNOSTIC`. It polls connected pads for START and then follows the existing cleanup path. The PS-button system utility callback remains in place.

Screen setup failures are logged with bounded numeric stage codes: argument, memory, image, RSX init, video state/configuration, display-buffer allocation/mapping/registration, or flip. The service remains standalone; no resident SPRX or game-audio mixing work was introduced.

The package script embeds `USRDIR/background.rgb`, checks its exact size and extracted bytes, and emits SFO version 01.01. The JPEG is included publicly because the maintainer explicitly requested that it remain public. It was re-encoded without EXIF metadata; reuse terms remain unspecified and are separated from the MIT code license.

## Runtime behavior changes

If screen initialization succeeds, the foreground app should show the submitted automotive-light image with a visible START exit prompt while receiver/audio processing continues. Pressing controller START requests ordered app shutdown and return to XMB. If image/video initialization fails, the app continues its audio/network path and records the failing stage; it does not claim a visible fallback. This avoids turning an unrelated RSX issue into a guaranteed forced exit, but means black output is still possible until hardware-tested.

## Validation performed

- `git diff --check` passed against the final source and documentation diff.
- `python3 scripts/prepare-diagnostic-background.py assets/diagnostic-background.jpg build/diagnostic-background.rgb` produced exactly 2,764,800 RGB bytes. The JPEG has no EXIF tags after re-encoding.
- `scripts/build-lima-standalone-receiver.sh` completed in the existing x86_64 Lima VM with warnings treated as errors. Final ELF SHA-256: `85b984ecf81e53f758df3cf422b78bc06206b6c18e1706a3510b5224ab2067fb`.
- `scripts/package-lima-standalone-receiver.sh` passed package listing and extraction checks. SFO reports `AIRPLAY02`, version `01.01`; extracted package includes `ICON0.PNG`, NPDRM EBOOT, and exactly 2,764,800 bytes of `USRDIR/background.rgb`.
- Candidate PKG SHA-256: `7824a4b15e4757afad3213a3f55cd3bea816b33795403da12bfd7ede29f93db4`; size: 3,222,640 bytes.
- No unit or hardware tests were run. The package has not been transferred or installed on the console.

## Immediate results

- Screen-enabled PPU ELF build: PASS in Lima.
- NPDRM package and embedded-background inspection: PASS in Lima.
- Public artwork metadata review: PASS; no EXIF tags.
- Console display, START-to-XMB, startup stability, and iPhone connection: NOT TESTED on this candidate.
- Existing staged package remains unchanged pending approval for this exact new PKG.

## Known limitations / follow-ups

- RSX video initialization and START-driven return still need PS3 hardware confirmation.
- The preceding run's log must be retrieved before diagnosing the failed iPhone connection.
- An iPhone listing only proves discovery; RAOP RTSP, RTP, ALAC, and normal music playback remain unverified.
- If RSX initialization hangs inside the SDK calls, the in-app START path cannot run; preserve the recovery path and do not add boot-time loading.

## Risks introduced

- RSX initialization changes video state to the currently reported output mode and allocates 32 MiB of shared command/I/O memory plus two video framebuffers. The current output mode is reused, but hardware behavior has not been validated.
- A failed video flip or SDK call may leave the screen blank; this candidate adds diagnostic codes but cannot guarantee recovery from a hardware-level hang.
- Public artwork reuse beyond this repository is not licensed by the project's MIT code license.

## Decision record

### Decision: Keep the user-provided image public and package a converted RGB runtime copy

- **Context:** The maintainer explicitly requested that the attached image remain public. The console needs a small predictable pixel buffer without adding a PS3 JPEG decoder dependency.
- **Options considered:** Keep the image private; embed the JPEG and add runtime decode; publish the source JPEG and convert it to raw RGB during packaging.
- **Why this option was chosen:** It satisfies the public-image instruction and keeps runtime loading bounded and independent of decoder modules. EXIF was removed before publication.
- **Tradeoffs accepted:** Pillow is needed to prepare the package asset; the raw RGB copy is larger than the JPEG and stays in ignored build output. Separate image reuse terms are unspecified.

### Decision: Use controller START for an explicit orderly XMB exit

- **Context:** The previous build provided no visible exit hint and did not poll controller input.
- **Options considered:** Rely only on the system PS-button menu; keep the current forced-recovery behavior; add a visible START action handled by the standalone app.
- **Why this option was chosen:** It gives the user a direct, visible shutdown action in the same foreground app and runs existing resource cleanup before returning from `main`.
- **Tradeoffs accepted:** Controller polling and process return need hardware confirmation; if the system or RSX is unresponsive, a controller action cannot recover it.

## Next recommended step

Review this exact 3,222,640-byte PKG and its SHA-256, then approve or decline replacing the staged package. After the user installs version 01.01 from XMB, verify image visibility and START exit before attempting another iPhone connection; retrieve and inspect the capped log afterward.
