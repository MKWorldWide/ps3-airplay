# Build and validate the v0.1.14 candidate

**Timestamp:** 2026-11-07 07:57 EDT

## Scope of the pass

Pulled pushed source 6d9f28d0f079c445b96203695ea1912e8deed887 into the existing Lima build workspace, rebuilt the AIRPLAY02 standalone ELF, generated v0.1.14, validated the finalized PKG, and copied the package into the ignored local build area with a matching hash.

After preserving the v0.1.14 artifact, advanced the Makefile and build/package script defaults to the next unused version, v0.1.15.

## Why this pass was needed

The source now requests SO_REUSEPORT best-effort. The PS3DK header defines this option, and RPCS3 source maps it to the native host option on non-Windows builds. A package from the exact committed source is required to validate that behavior in RPCS3 and, separately, on the PS3.

## Files changed

- docs/BUILD_PIPELINE.md
- docs/TEST_MATRIX.md
- prototype/standalone-receiver/Makefile
- scripts/build-lima-standalone-receiver.sh
- scripts/package-lima-standalone-receiver.sh
- docs/lessons-learned.md
- docs/memory.md
- docs/scratchpad.md
- docs/reports/2026-11-07_v0114_reuseport_build.md
- docs/decisions/2026-11-07_0757_v014_package_validation.md

## Architecture / logic changes

None in this pass. The earlier v0.1.14 source change requests SO_REUSEPORT before binding UDP 5353, without changing mDNS protocol or port behavior.

## Runtime behavior changes

No package has been installed or run in this pass. No console files, system settings, or boot plugins were changed.

## Validation performed

- Full Lima crypto/host test script ran through crypto, session, SDP, and mDNS tests but stalled in the RTSP/RTP loopback test; a 90-second bound terminated the run.
- The RTSP/RTP loopback passed in a bounded strace run with LeakSanitizer disabled. The RTSP listener/log, ALAC, PCM ring/resampling, and diagnostic tone tests passed individually. The timing-sensitive behavior remains unexplained.
- Native host warnings-as-errors compile and mDNS serializer test passed.
- Confirmed PS3DK exposes SO_REUSEPORT; PPU build compiled the receiver.
- ELF validation passed for the 2,758,392-byte CellOS PPC64 executable.
- Package validation passed for the finalized NPDRM envelope, SFO identity/version, content ID, relative entries, and extracted payload equality.
- Package: 3,225,008 bytes, SHA-256 5994cf4828976245f530b1c90d180fd4f0e80abddcfd0d09d3780c351ad006c1.
- RPCS3 interactive testing could not be completed in this pass. The PS3 did not answer the read-only reachability check; no console write was attempted.

## Immediate results

The v0.1.14 package is reproducibly tied to pushed source and passes structural/ABI validation. Host socket evidence and upstream RPCS3 source support the SO_REUSEPORT path, but neither app-level DNS-SD advertisement nor a native PS3 bind has been observed.

## Known limitations / follow-ups

The RPCS3 GUI launch test and native PS3 test remain pending. The full host script's RTSP/RTP loopback is timing-sensitive; traced success does not resolve the untraced stall.

## Risks introduced

Port sharing may result in duplicate multicast replies. The socket option is best-effort and does not alter the system mDNS responder. Do not call this a discovery fix until the app's advertisement and iPhone selection succeed.

## Decision record

### Decision

Preserve v0.1.14 as the validated candidate and wait for RPCS3 UI access and PS3 network reachability before claiming runtime behavior.

### Context

The build and package validate, but the interactive emulator was unavailable and the console endpoint did not respond. The host socket probe is not equivalent to application execution.

### Options considered

- Attempt to automate RPCS3 despite unavailable UI.
- Change more protocol code based only on host socket results.
- Preserve the artifact, record the exact test boundary, and resume with real runtime logs.

### Why this option was chosen

The staged package is testable, while additional protocol changes without seeing the app's actual bind/announce outcome would be speculative.

### Tradeoffs accepted

The next implementation decision is deferred until RPCS3 or native PS3 logs show whether SO_REUSEPORT reached the host socket and allowed mDNS setup.

## Next recommended step

Run the exact v0.1.14 package in RPCS3 and inspect the fresh log for MDNS_READY versus MDNS_FAILED. Then run it on the PS3 when reachable, confirm visible display/START exit, and attempt one iPhone discovery selection.
