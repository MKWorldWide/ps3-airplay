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

## Next action

Check the version shown in the app/log or package metadata for the copy that was just opened. Compare against v0.1.14 and current screen source. The current RPCS3 test requires an unlocked interactive desktop; do not use this report as a substitute for that test.
