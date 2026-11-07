# Display cleanup follow-up

**Date:** 2026-11-07 08:02 EDT
**Source revision:** `3c2ec88` (before this documentation update)
**Artifact/platform:** Not identified by the user
**Test ID:** DISPLAY-FOLLOWUP-01

## User observation

The user reports that the application opens and the background image is visible. They still see blue borders and the `DISPLAY ACTIVE` label, and requested their removal. This is positive launch and visible-image evidence, but the package version and whether the observation came from RPCS3 or the physical PS3 were not specified.

## Source check

The current `main` source already removes the blue diagnostic bands and `DISPLAY ACTIVE` label in commit `46fca40`; the screen keeps the background and a white START-to-exit prompt. The v0.1.14 candidate was built after this cleanup. A mismatch between the observed copy and current source is plausible, but not yet verified. Do not alter the screen code or claim the current artifact regressed until the exact installed package is identified.

## Result matrix

| Check | Result |
|---|---|
| App opens | USER-REPORTED PASS; package/platform unknown |
| Background visible | USER-REPORTED PASS; package/platform unknown |
| Blue borders absent | USER-REPORTED FAIL in observed copy |
| `DISPLAY ACTIVE` absent | USER-REPORTED FAIL in observed copy |
| START exits cleanly | NOT REPORTED |
| AirPlay discovery / RTSP / RTP / audible stream | NOT REPORTED |

This observation does not establish current-source behavior, physical PS3 display reliability, controller exit, or AirPlay operation.

## Local runtime snapshot checked afterward

The local RPCS3 installation has AIRPLAY02 version 0.1.12; the v0.1.14 candidate is not installed there. Its available app log records RTSP port 5001 and RTP ports 6000–6002 ready, mDNS bind failure at stage 4 with numeric error 112, diagnostic tone completion, repeated PCM underflows with no RTP receive event, and APP_STOP. A bounded local RAOP browse found no service instance matching the expected project label. The previously used physical-console FTP and webMAN endpoints did not respond. These observations cannot be attributed to the user's visible-app report.

RPCS3 v0.1.14 remains the next runtime test. Its PKG was already built and hash-verified; do not rebuild until the exact candidate is installed and tested or a code defect is identified.

## Next action

Check the version shown in the app/log or package metadata for the copy that was just opened. Compare against v0.1.14 and current screen source. RPCS3 runtime validation requires its interactive package/install flow; do not use this report as a substitute for that test.

## Follow-up at 2026-11-07 10:53 EDT

The source lineage was rechecked after the report above. Commits `46fca40` (removes blue bands and `DISPLAY ACTIVE`) and `4a06c2a` (removes the remaining START prompt) are ancestors of the v0.1.20 build source `8a4e641`. Its existing finalized PKG is 3,230,384 bytes with SHA-256 `bfaa3f960de39daf95c2ab405bd361a031ed92bd35c76a99e667d4ffe8d19e87`. The screen renderer copies only the supplied background image.

The user again reports that the app opens and the image is visible, while reporting blue framing and `DISPLAY ACTIVE`. This observed copy still cannot be matched to a package or platform. A fresh Computer Use state check after the user said the Mac was being unlocked continued to report the desktop locked. The last accessible RPCS3 snapshot and log still identify v0.1.15; the v0.1.20 candidate has not been installed or run through that UI. Keep this mismatch unresolved; do not claim that the observed copy has the cleaned screen or make another identical package.

The next useful evidence is a fresh launch with an accessible package version/log and a view of the actual app screen. If the running copy is v0.1.20 or newer and still displays the overlays, inspect the packaged EBOOT/assets and runtime path before changing the renderer.
