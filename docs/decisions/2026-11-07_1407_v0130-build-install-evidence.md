# Validate and install corrected-label v0.1.30

## Timestamp in EDT
2026-11-07 1407 source-local America/New_York

## Scope of the pass
Build exact committed source, validate and transfer the corrected-label receiver, verify installed payload, and record results.

## Why this pass was needed
Requested release is 0.1.30 with SFO 00.30; the first recovered launch used historical label 0.30.

## Files changed
docs/memory.md; docs/scratchpad.md; docs/reports/2026-11-07_v030_hardware_launch_and_options_stall.md; this decision log.

## Architecture / logic changes
No further code changes; artifact source remains bfca67f.

## Runtime behavior changes
Only the release marker differs from historical 0.30. Existing OPTIONS stall remains unresolved.

## Validation performed
Canonical Lima build and packaging scripts; extracted SFO semantic validation; content/asset/ELF/package checks; FTP upload/download-byte comparison; retrieved installed SFO/EBOOT comparison. All PASS.

## Immediate results
Package uploaded and installed; exact payload confirmed. Human launch result remains the preceding historical 0.30 observation.

## Known limitations / follow-ups
No real streamed music; last hardware log stops within OPTIONS. Need measured challenge/RNG phase tracing in 0.1.31 before further extended tests.

## Risks introduced
Replaced installed receiver under standing automatic-install authorization. Original historical package and previous versioned packages remain available for rollback. No system configuration changes.

## Decision record (embedded)
- Decision: preserve both historical and corrected-label artifacts. Context: separate source/version identity. Options: overwrite historical file or new filename. Chosen: new filename and verified bytes. Tradeoff: extra package storage.
- Decision: verify installed payload after remote install request. Context: HTTP 200 alone is not completion. Options: trust response or compare installed files. Chosen: byte comparisons. Tradeoff: small additional read-only transfer.

## Next recommended step
Investigate the OPTIONS challenge/RNG boundary and keep controller responsiveness as a gate before streamed-audio testing.
