# Restore foreground responsiveness and target PCM byte order

## Timestamp in EDT
2026-11-07 1423 source-local America/New_York

## Scope of the pass
ALAC byte order, regular presentation, control polling order, bounded graphics waits and diagnostic rate limiting. Version 0.1.32 / SFO 00.32.

## Why this pass was needed
Crackling and unavailable START/PS reported on hardware. Live log kept updating but idle noise erased session records.

## Files changed
src/codec/alac.c; prototype/standalone-receiver/source/main.c; prototype/standalone-receiver/source/diagnostic_screen.c; prototype/standalone-receiver/source/diagnostic_screen.h; prototype/standalone-receiver/Makefile; scripts/build-lima-standalone-receiver.sh; scripts/package-lima-standalone-receiver.sh; docs/memory.md; docs/scratchpad.md; docs/lessons-learned.md; docs/BUILD_PIPELINE.md; this log.

## Architecture / logic changes
Compiler byte-order selection replaces constant zero. Foreground loop polls controls and sysutil first, presents without waits and decodes at most four packets per iteration. A finite backend-label wait replaces unbounded graphics finish at close. Failed drain retains allocations until process exit. Existing capped log gets less idle noise and more state/input diagnostics.

## Runtime behavior changes
Correct PCM byte order on PS3. Regular display flips for VSH overlay composition; result is unverified on real hardware. No blocking display wait in the service loop. START skips further stream work once detected.

## Validation performed
Live HTTP log retrieval; normal webMAN quit produced complete cleanup and APP_STOP 0. Target build caught signed-clock versus unsigned-deadline comparisons; explicit casts fix the warning under -Werror. Host sanitizer fixture and big-endian PPU fixture comparison PASS. Full-app paced RPCS3 test records 256 decrypts and 90,368 PCM frames queued/consumed; heartbeat/pad API active and no presentation failure. Package and installed payload checks PASS. Privacy review before commit.

## Immediate results
Target byte-order correction verified against golden PCM; candidate installed with exact payload match. Earlier app recovered through normal quit without reboot. Physical overlay/audio outcome awaits maintainer.

## Known limitations / follow-ups
System-menu behavior and audible music require physical observation. Prior session logs were overwritten, preventing packet/decode attribution for the crackling.

## Risks introduced
Nonblocking display cadence changes RSX command timing. On RSX timeout allocations persist until process teardown; bounded to existing screen resources. No firmware or global video-setting changes.

## Decision record (embedded)
- Decision: portable compile-time byte-order detection. Context: hard-coded little-endian host. Options: PS3-only swap or actual endian selection. Chosen: endian selection. Tradeoff: unknown compiler byte orders fail compilation.
- Decision: nonblocking regular flips. Context: only startup flips, unavailable in-game overlay. Options: display rewrite or retain static buffers and present. Chosen: present existing buffers. Tradeoff: compositor hypothesis needs hardware confirmation.
- Decision: keep single service loop with input first. Context: log proves loop alive. Options: introduce threads or bound/reorder work. Chosen: bound/reorder. Tradeoff: remaining OS calls are still synchronous.
- Decision: normal remote quit once. Context: live log preserved and HTTP reachable. Options: forced reboot or documented quit. Chosen: normal quit. Tradeoff: only log completion is verified; visible XMB return awaits observation.

## Next recommended step
Maintainer tests installed v0.1.32 PS/held-PS menu and START before short streaming; collect logs without relaunch.

Artifact evidence: docs/reports/2026-11-07_v0132_controls_and_pcm.md; docs/VERSIONING.md; docs/TEST_MATRIX.md; docs/ROADMAP.md; docs/RISKS.md. Raw logs stay local.
