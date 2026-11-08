# v0.1.39 live stream and remote exit diagnostic record

## Timestamp

2026-11-07 18:58 EST

## Scope of the pass

Sent the webMAN XMB-exit command to the active receiver, retrieved and analyzed the fresh capped log, confirmed the log remained unchanged on a second read, and checked read-only webMAN status. Recorded hardware findings and diagnostic gaps in project documentation.

## Why this pass was needed

The user reported that AirPlay still worked while controller and PS button input appeared unresponsive. A controlled remote exit was needed before inspecting evidence and deciding whether the app was frozen.

## Files changed

- `docs/ROADMAP.md`
- `docs/TEST_MATRIX.md`
- `docs/RISKS.md`
- `docs/memory.md`
- `docs/lessons-learned.md`
- `docs/scratchpad.md`
- `docs/reports/2026-11-07_v0139_stream_and_remote_exit.md`
- `docs/decisions/2026-11-07_1858_v0139_stream_exit_diagnostics.md`

## Architecture / logic changes

No source changed. The documented test state now separates controller presence from button receipt, packet loss recovery from jitter measurement, PCM submission from physical HDMI output, and remote app cleanup from a manually observed controller exit.

## Runtime behavior changes

webMAN remote exit asked the running app to quit. The app closed screen, audio, receiver sockets and reported APP_STOP. XMB was confirmed by webMAN status. No restart or boot configuration change occurred.

## Validation performed

- webMAN `/xmb.ps3$exit`: HTTP 200.
- FTP log read: 7,280 bytes; repeated read had identical SHA-256 and byte comparison.
- Read-only webMAN status: HTTP 200, XMB shown, AIRPLAY02 absent.
- Inspected source meaning of session state, pad-health fields, counter deltas and cleanup callbacks.

## Immediate results

The app was not fully stalled during the retained interval: 10-second heartbeats and successful pad-query status continued after AirPlay teardown until the webMAN exit. Stream state had reached recording and produced encrypted RTP, decrypted ALAC and PCM output submissions. Significant RTP sequence gaps and silence concealment remain plausible artifact contributors.

## Known limitations / follow-ups

Log wrap removed older history. It records no actual controller buttons or arrival jitter. No HDMI loopback or fresh user acoustic assessment was obtained. The OPTIONS EOF issue from the separate earlier probe remains unresolved.

## Risks introduced

No new code risks. Interpretation risk remains if recovery counters from different intervals are combined or if `PAD_HEALTH` is treated as proof a button press was received.

## Decision record

### Decision 1: Exit through webMAN, not power-cycle

- **Context:** User requested XMB return while input appeared unresponsive.
- **Options considered:** Console power cycle or webMAN game-exit command.
- **Why chosen:** The documented command requests the app exit through PS3 system utilities without restarting the console.
- **Tradeoffs accepted:** If the app were frozen below sysutil, command might fail; in this run it completed cleanup.

### Decision 2: Attribute crackle only to observed risk factors

- **Context:** RTP gaps and concealment coexist with ALAC/PCM output.
- **Options considered:** Declare network jitter as the cause, or retain it as a leading candidate pending arrival timing/loopback measurement.
- **Why chosen:** Current data do not record arrival jitter or exact user-audible artifacts.
- **Tradeoffs accepted:** Additional instrumented run is needed before tuning buffer policy.

## Next recommended step

Add bounded, privacy-safe controller button-transition, RTP inter-arrival jitter, and PCM occupancy/starvation telemetry before the next hardware playback test.
