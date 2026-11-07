# Inspect the receiver startup log

**Timestamp:** 2026-11-07 00:42 EST

## Scope of the pass

Retrieved the capped PS3 receiver log over FTP after staging version 01.01. Parsed only its timestamp, event, value, and byte-count fields; kept the raw log in ignored local build output. Recorded the partial evidence and the uncertain relationship between the log and the staged package.

## Why this pass was needed

The user reported that the app played its five-second tone, displayed black, and did not return to XMB. The iPhone listed the receiver but could not connect. The staged 01.01 package was replaced with explicit approval, and the prior run log could help distinguish display initialization, receiver startup, and connection failures.

## Files changed

- `docs/reports/2026-11-07_first_hardware_test.md`
- `docs/TEST_MATRIX.md`
- `docs/memory.md`
- `docs/lessons-learned.md`
- `docs/scratchpad.md`
- `docs/decisions/README.md`
- `docs/decisions/2026-11-07_0042_inspect_startup_log.md`

## Architecture / logic changes

No runtime code changed. Documentation now treats `SCREEN_READY` as RSX setup and first-flip evidence only, not proof that visible pixels appeared. It records the reported image failure separately from the log's stage result.

## Runtime behavior changes

None. FTP was reachable for package replacement and one log retrieval, then timed out on a subsequent read-only check. No install, launch command, reboot, or configuration change was performed by this pass.

## Validation performed

- Downloaded `/dev_hdd0/tmp/ps3-airplay.log` over FTP; remote and downloaded size were both 350 bytes, below the 16 KiB cap.
- The parsed events were `APP_START`, `SCREEN_READY`, `CONTROLLER_READY`, `MDNS_READY`, `AUDIO_READY`, and `AUDIO_TEST_TONE_START`.
- No `AUDIO_TEST_TONE_COMPLETE`, `USER_EXIT_START`, `APP_STOP`, or RTSP/session event was present in this snapshot.
- FTP timed out on the next check. No raw log was staged or committed.
- No unit, build, or hardware test was performed in this documentation pass.

## Immediate results

- The receiver reached its screen, controller, mDNS, and audio startup events according to the log.
- The log does not prove visible image output, a complete five-second tone, a clean exit, or an iPhone connection.
- Exact package identity for the log is unknown; a fresh, controlled run is needed to tie it to version 01.01.

## Known limitations / follow-ups

- Confirm whether the user installed and launched the staged 01.01 package.
- After a stable FTP connection, retrieve a fresh log from a controlled run and correlate it with the user's visible screen result.
- Diagnose RAOP only after an RTSP/session event or a reproducible connection attempt appears in the log.

## Risks introduced

- None. The raw log remains outside Git; published records contain only bounded event names and numeric values.

## Decision record

### Decision: Keep display and connection outcomes unresolved

- **Context:** The log reports `SCREEN_READY` and `MDNS_READY`, but the user reports black video and a failed iPhone connection. FTP then timed out, and the exact package version is unknown.
- **Options considered:** Mark screen and discovery successful from log events; assume the log proves a failed framebuffer; preserve the mismatch and collect a version-identified run.
- **Why this option was chosen:** The event schema records initialization stages, not photons or successful AirPlay handshakes. Preserving both observations avoids overstating what the file proves.
- **Tradeoffs accepted:** Hardware diagnosis remains open until the user confirms the installed version and visible result and a fresh log can be retrieved.

## Next recommended step

Ask whether version 01.01 was installed and launched and whether the image appeared. Once FTP is reachable, collect a fresh capped log after one controlled screen/exit check, then retry the iPhone connection only if the app exits cleanly.
