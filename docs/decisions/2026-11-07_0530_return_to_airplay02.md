# Return to AIRPLAY02 and bounded network startup

**Timestamp:** 2026-11-07 05:30 EDT

## Scope of the pass

Park the separate silent AIRTEST01 diagnostic and resume receiver development. Restore the previously used route-based local IPv4 lookup as the first option, add a bounded NetCtl wait/fallback, and tie the startup version event to the package version. Build and inspect AIRPLAY02 v0.1.4 locally. Update project records to distinguish the user's earlier tone/start observation from the later launch failures.

## Why this pass was needed

The maintainer reports that an earlier AIRPLAY02 run started and played the five-second tone. AIRTEST01 v0.1.0 was a silent PS3DK launch-control app; it returned the same user-reported XMB error and did not advance receiver behavior. Later AIRPLAY02 code also required NetCtl `IPObtained` immediately, unlike the earlier route lookup.

## Files changed

- `README.md`
- `docs/README.md`
- `docs/ROADMAP.md`
- `docs/TEST_MATRIX.md`
- `docs/RISKS.md`
- `docs/memory.md`
- `docs/lessons-learned.md`
- `docs/scratchpad.md`
- `docs/decisions/2026-11-07_0530_return_to_airplay02.md`
- `docs/reports/2026-11-07_return_to_receiver.md`
- `prototype/standalone-receiver/Makefile`
- `prototype/standalone-receiver/source/main.c`
- `scripts/build-lima-legacy-runtime.sh`
- `scripts/build-lima-standalone-receiver.sh`
- `scripts/package-lima-standalone-receiver.sh`

## Architecture / logic changes

`resolve_local_ipv4()` now tries the earlier UDP route-based local-address discovery first. If that fails, it polls NetCtl at 100 ms intervals for up to five seconds. If NetCtl does not return a usable address, it makes one final route-based attempt and then exits with a bounded failure. Only the path/state is logged; the address is not. The logger and package builder now receive the same `APP_VERSION` input.

The changes preserve the existing RAOP receiver, audio, screen, and diagnostic tone implementation. They do not explain the XMB launch rejection, prove pixels reach HDMI, or establish successful AirPlay.

## Runtime behavior changes

When the app reaches `main()`, the startup log will identify v0.1.4. A delayed NetCtl IP state no longer immediately blocks discovery if the route-based local address is already available; otherwise, startup waits no longer than five seconds before a fallback/failure decision. No PS3 files or boot settings were changed in this pass.

## Validation performed

- `scripts/build-lima-crypto.sh` on the existing Lima host: crypto, session, SDP, and mDNS checks passed. The script then stalled in the direct receiver loopback test at the timing-probe stage; it was interrupted. The full suite is **not** green.
- Targeted host tests: RTSP listener/log passed; ALAC reference decode passed (353 samples); PCM ring/stereo conversion/resampling passed; five-second 440 Hz diagnostic tone passed.
- `scripts/build-lima-legacy-runtime.sh` using PSL1GHT commit `6e565a70e927f55813babe86dc6f64149535f908` and its matching GCC 7.2/Mbed TLS build: full standalone receiver linked. ELF is big-endian ELF64 PowerPC64.
- `scripts/package-lima-standalone-receiver.sh`: PKG creation, extraction, content ID, SFO, icon, EBOOT, and background asset checks passed. SFO reports `APP_VER=0.1.4`, `TITLE_ID=AIRPLAY02`; package content ID matches. The embedded background is 2,764,800 bytes.
- Candidate package: 3,183,584 bytes, SHA-256 `0c1e3c74eea71f7b5829201673f4dd5211bcbf704ab00cb55e1b41cb87020ee4`.
- ELF SHA-256 `6e2352c5f7e3cb65f1cf9615606d790d60a02a3cc6bd36bb4e09639a8fea65fc`; packaged EBOOT SHA-256 `639115c46db122e28d88db27ece84e1a85cc5ac2f6943b55a2afd99315d6ccff`.
- PS3 developer error reference lookup: no entry for `80028F21`; its cause remains unknown.
- No console transfer, installation, launch, or settings change occurred in this pass.

## Immediate results

- A new AIRPLAY02 candidate builds on the earlier runtime path and the NetCtl startup gate is recoverable and bounded.
- The direct uninstrumented receiver loopback hang recurred; other targeted host checks passed.
- The user-confirmed earlier startup/tone observation remains valuable but is not attributed to an exact build. Versions 0.1.2, 0.1.3, and the separate AIRTEST01 v0.1.0 each later produced the user's `80028F21` launch report, with no fresh AIRPLAY log for the AIRPLAY02 failures.

## Known limitations / follow-ups

- v0.1.4 is only locally built; it is not installed or hardware-tested.
- The launch rejection occurs before a new diagnostic log and is not explained by this change.
- Visible output, START exit, AirPlay connection, RTP, decryption, ALAC stream decode, audible music, and long-run behavior are not validated.
- The direct receiver timing-probe test needs a deadline/race diagnosis.

## Risks introduced

- The five-second NetCtl wait can add up to five seconds to startup when route lookup fails. Its PS3 timebase/poll behavior must be verified on hardware.
- The route-based UDP lookup preserves earlier behavior but can select an interface based on the active route; LAN hardware testing must validate the advertised service address.
- This package still has the same broad XMB launch-compatibility uncertainty as previous candidates. Do not assume v0.1.4 solves `80028F21`.

## Decision record

### Decision: resume AIRPLAY02 and park AIRTEST01

- **Context:** The user reports prior receiver startup and audible tone; AIRTEST01 is silent and has a separate title ID.
- **Options considered:** Continue with the isolated launch-control app; abandon the receiver; return to the receiver path while keeping the diagnostic app untouched.
- **Chosen:** Return to AIRPLAY02 and park AIRTEST01.
- **Tradeoffs:** We lose a potentially useful comparison experiment for now, but preserve focus on the user's requested audible receiver and do not modify/remove the installed diagnostic title.

### Decision: retain route-first discovery with bounded NetCtl fallback

- **Context:** Immediate NetCtl gating was stricter than the earlier route-based logic and could fail before service advertisement.
- **Options considered:** Require immediate NetCtl success; use route lookup only; try route lookup first, then bounded NetCtl, then one route retry.
- **Chosen:** Route-first with a five-second NetCtl wait and final route retry.
- **Tradeoffs:** This preserves prior behavior and avoids an unbounded wait, but the selected address and readiness path still require real LAN testing.

### Decision: create a local v0.1.4 candidate, withhold console deployment

- **Context:** New code changes need a distinguishable artifact, but earlier launch reports remain unexplained.
- **Options considered:** Reuse a prior package; install the candidate immediately; validate the candidate locally first and keep the console untouched until review.
- **Chosen:** Local build/package validation only in this pass.
- **Tradeoffs:** Hardware diagnosis is deferred until the exact new package and safe exit test are selected.

## Next recommended step

Bound the existing RSX waits and move controller readiness ahead of potentially blocking graphics work. Compare v0.1.4's SELF metadata/build flags with the preserved earlier AIRPLAY02 candidate that accompanied the tone report. Once this safety gate is met and the user resumes the paused goal, use the authorized reversible hardware path to test one exact receiver build for XMB launch, visible output, tone, and START-to-XMB. Retrieve a fresh log before attempting an iPhone connection.
