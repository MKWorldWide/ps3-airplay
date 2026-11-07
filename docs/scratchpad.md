# Active work

- **Task:** Diagnose why installed v0.1.2 fails at XMB launch before further console tests.
- **Source:** Remote `main` is still `a6b36cd`; local branch `docs/v012-preflight-review` contains only the later documentation commit `f7e1169` before this report.
- **Package:** v0.1.2 is 3,225,712 bytes, SHA-256 `923aebf6eca5693b21b494a9a4225c36629f13e8b316b2825d5deb3763226ebf`. Installed SFO reports AIRPLAY02 / 0.1.2; installed SFO and 445,232-byte EBOOT hashes match the extracted package files.
- **Hardware result:** User reports launch error `80028F21`. No current-build startup log was written; the retrieved 1,817-byte log is unchanged and belongs to the earlier unidentified run. Cause is unknown.
- **Hypotheses to check:** NPDRM SELF metadata, packaged startup/link assumptions, then a minimal reversible launch control. Avoid speculative CFW/settings changes and do not stage another build until the fault is isolated and the recovery route is clear.
- **Next:** Inspect SELF metadata and toolchain startup contract, then document a concrete cause before preparing any replacement package. No fresh display, audio, controller exit, or AirPlay result is available for v0.1.2.
