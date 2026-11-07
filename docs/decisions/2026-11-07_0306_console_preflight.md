# Read-only PS3 preflight and existing-log review

**Timestamp:** 2026-11-07 03:06 EDT

## Scope of the pass

Completed read-only console checks after the user confirmed the PS3 was on at the same address. Verified free space, installed AIRPLAY02 SFO version, staged package identity, and current log. Preserved the staged package and log locally under ignored work material. Updated status and recovery documentation.

## Why this pass was needed

The earlier preflight was blocked while the console was offline. An exact package transfer decision requires knowing what is currently staged and installed, whether there is space, whether the diagnostic log exists, and what rollback path is available.

## Files changed

- `docs/TEST_MATRIX.md`
- `docs/ROADMAP.md`
- `docs/memory.md`
- `docs/lessons-learned.md`
- `docs/scratchpad.md`
- `docs/reports/2026-11-07_v012_console_preflight.md`
- `docs/decisions/2026-11-07_0306_console_preflight.md`

## Architecture / logic changes

No runtime source or package changed. Documentation now distinguishes the current 01.01 installation and staged package from the untransferred v0.1.2 candidate, and narrows the interpretation of `APP_STOP` to cleanup evidence.

## Runtime behavior changes

None. All console operations were read-only: FTP directory/file reads and an HTTP status-page read. The attempted FTP `SITE @info` command was rejected by the server with code 500 and did not proceed.

## Validation performed

- FTP reachability and package directory listing.
- HTTP status-page read: XMB, 9.2 GB free.
- Parsed installed AIRPLAY02 SFO: APP_VER 01.01 and expected title ID.
- Re-downloaded staged PKG and verified size 3,222,640 bytes and SHA-256 `7824a4b15e4757afad3213a3f55cd3bea816b33795403da12bfd7ede29f93db4`.
- Retrieved the current 1,817-byte log and preserved the unmodified original locally; reviewed only timestamp/event/counter fields. No RTSP event, `USER_EXIT_START`, or version marker is present.
- Confirmed v0.1.2 local artifact remains 3,225,712 bytes with SHA-256 `923aebf6eca5693b21b494a9a4225c36629f13e8b316b2825d5deb3763226ebf`.
- No console writes, package transfer/install, SPRX operation, firmware change, or boot-plugin change.
- Reviewed public diff for private host details, local addresses, credentials, and agent instructions; none were added. Local package/log backups remain ignored and untracked.

## Immediate results

The console is available and has ample space for the candidate. It currently has version 01.01 installed and staged. The current log shows tone completion and cleanup but cannot prove which package generated it, normal START exit, visible display, or AirPlay connection. The v0.1.2 candidate is still local.

## Known limitations / follow-ups

- The FTP status page does not establish exact package identity for the prior log.
- Clean START exit and visible display remain unverified.
- v0.1.2 requires explicit approval before replacing the staged 01.01 package.
- A forced power-off is an emergency recovery path, not evidence of clean app behavior.
- RSX waits remain unbounded; receiver loopback timing remains unstable outside tracing.

## Risks introduced

No runtime or console risk introduced. The log and previous package were downloaded to ignored private working material and will not be committed.

## Decision record (embedded)

### Keep console data read-only and stage backup first

- **Decision:** Preserve the exact current package and diagnostic log locally; do not transfer v0.1.2 in this pass.
- **Context:** The console was online and readable, but the next package would replace an existing staged file and the display launch has a known forced-shutdown history.
- **Options considered:** Replace the staged package immediately; download and verify the current file first, then request exact approval; modify the console app or plugins to establish recovery.
- **Why chosen:** The local verified backup creates a staged-file rollback, while no user approval for the v0.1.2 hash has been given. No firmware or boot behavior changes are needed.
- **Tradeoffs accepted:** Hardware validation pauses at a concrete approval gate.

### Do not equate APP_STOP with normal XMB exit

- **Decision:** Treat the current log as cleanup evidence only.
- **Context:** It contains APP_STOP but lacks USER_EXIT_START and any build-version marker.
- **Options considered:** Infer a clean exit; infer the exact package from installed SFO; preserve identity and exit path as unknown.
- **Why chosen:** Logs can survive across installs and the event does not encode which path triggered cleanup.
- **Tradeoffs accepted:** The prior screen/exit run remains unresolved until a version-identified run has visible output and user-confirmed XMB return.

## Next recommended step

Obtain the user's explicit approval to replace only the staged package with the exact v0.1.2 artifact described in the report. Then FTP it, verify downloaded-back SHA-256, and hand off XMB installation and the display/START-exit test before attempting iPhone connection.
