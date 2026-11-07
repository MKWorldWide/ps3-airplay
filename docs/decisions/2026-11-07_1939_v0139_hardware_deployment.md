# v0.1.39 physical deployment and diagnostic decision log

## Timestamp

2026-11-07 19:39 EDT

## Scope of the pass

Deployed the already validated production AIRPLAY02 v0.1.39 package to the PS3, verified installed identity, launched it, collected a fresh diagnostic log, and performed bounded same-LAN DNS-SD and RTSP OPTIONS probes. Updated current-state memory, lessons, scratchpad, roadmap, risk, test matrix, and hardware report.

## Why this pass was needed

The hardware had returned online and the approved next deployment target was v0.1.39. Fresh hardware evidence was needed before further protocol changes.

## Files changed

- `docs/ROADMAP.md`
- `docs/TEST_MATRIX.md`
- `docs/RISKS.md`
- `docs/memory.md`
- `docs/lessons-learned.md`
- `docs/scratchpad.md`
- `docs/reports/2026-11-07_v0139_ps3_launch.md`
- `docs/decisions/2026-11-07_1939_v0139_hardware_deployment.md`

## Architecture / logic changes

No receiver source code changed. The current status now separates package/install/startup verification from visible output, acoustic output, RTSP completion, streamed audio, and clean-exit evidence.

## Runtime behavior changes

AIRPLAY02 v0.1.39 was installed and launched. It remains running for the user to observe. No boot plugin, firmware, resident module, or global console setting changed.

## Validation performed

- Preflight read staged/installed identity, prior log, console reachability and free space over FTP/webMAN.
- Uploaded production PKG to a new staged name and downloaded it back; size and SHA-256 matched exactly.
- Installed with webMAN; FTP verified SFO 00.39; webMAN confirmed AIRPLAY02 running after launch.
- Retrieved a fresh capped log and issued bounded same-LAN DNS-SD/RTSP OPTIONS probes.
- Privacy-reviewed documentation changes before commit; raw logs remain local and ignored.

## Immediate results

Startup reached screen/controller, network, RTSP/RTP listeners, mDNS, audio, and tone-complete checkpoints. DNS-SD browse found RAOP. RTSP probe connected but got EOF; target log reported status 200 after send success.

## Known limitations / follow-ups

Visible display, audible tone, clean exit, repeated launch, complete iPhone RTSP negotiation, RTP, decrypt, ALAC, PCM, Spotify, and sustained playback are not confirmed. Resolve the RTSP response discrepancy and collect user observations before claiming those gates.

## Risks introduced

The foreground app remains open until the user exits it; user should report any hang or display issue. The diagnostic log's server-side send status may be mistaken for client receipt if read without the probe result.

## Decision record

### Decision 1: Leave the tested app running

- **Context:** Deployment and startup checkpoints succeeded; the user may now inspect the display and tone.
- **Options considered:** Stop immediately or keep the app available for human observation.
- **Why chosen:** User requested installation and launch, and no hang was observed through remote status/logs.
- **Tradeoffs accepted:** User observation is still required; current foreground session remains active.

### Decision 2: Treat RTSP as unresolved

- **Context:** Client EOF conflicts with server status-200-after-send log.
- **Options considered:** Call OPTIONS successful from server log, or preserve both boundaries as partial evidence.
- **Why chosen:** Server send completion does not prove peer received/parses bytes.
- **Tradeoffs accepted:** Further protocol diagnosis is required before claiming a session.

## Next recommended step

Record user-confirmed visible display, audible tone, and START/PS exit; then diagnose the OPTIONS EOF and perform one controlled iPhone connection test.
