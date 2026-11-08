# Resident PRX lifecycle guard and RPCS3 harness

## Timestamp in EDT

2026-11-08 06:09 EDT

## Scope of the pass

Added configurable resident log and port settings, bounded worker-completion handling during module stop, and a dedicated RPCS3 package/harness for repeated PRX lifecycle cycles. Fixed the target's missing math link and ensured the PRX overlay is included in the generated package.

## Why this pass was needed

The resident build initially failed at link time on `powf`. The existing prototype used a blocking join without first observing worker cleanup completion. No dedicated emulator title exercised load/start/stop/unload. The host's TCP 5000 service also conflicts with the receiver's default test port.

## Files changed

- `experimental/resident/CMakeLists.txt`
- `experimental/resident/module.c`
- `experimental/resident/harness/CMakeLists.txt`
- `experimental/resident/harness/sfo.xml`
- `experimental/resident/harness/source/main.c`
- `scripts/build-lima-resident.sh`
- `scripts/build-lima-resident-rpcs3-harness.sh`
- `docs/memory.md`
- `docs/lessons-learned.md`
- `docs/scratchpad.md`
- `docs/decisions/2026-11-08_0609_resident_lifecycle_harness.md`

## Architecture / logic changes

- Resident log path, RTSP port, and RTP base port can be set at build time; existing defaults are preserved. The harness build uses a dedicated log and ports 5001/6100.
- The worker signals completion only after receiver and discovery cleanup. `module_stop` waits at most five seconds for that signal, then joins the completed joinable thread. A timeout or join error reports `SYS_PRX_STOP_FAILED` and does not close resources still potentially used by the worker.
- A separate homebrew package uses diagnostic Title ID `RPRX90001`, loads `/dev_hdd0/game/RPRX90001/USRDIR/ps3-airplay-receiver.prx`, and executes three cycles. It checks receiver-ready, worker-stopped, and module-stop log events and does not unload after a failed stop.
- CMake creates the package overlay directory at configure time because the SDK helper decides whether to copy it during configuration.

## Runtime behavior changes

Module stop now waits for positive worker cleanup evidence for up to five seconds before joining. A receiver that cannot bind because of configured port use logs receiver-open failure and can still be stopped/reaped. The diagnostic harness itself emits cycle markers to a separate app log. No user-facing product behavior or standalone receiver code changed; no resident PCM playback was added.

## Validation performed

- Full `scripts/build-lima-resident-rpcs3-harness.sh` on Lima using PS3DK 0.27.0 and Mbed TLS 3.6.7.
- Host checks passed: crypto/base64/AES/RSA, encrypted RTSP negotiation/decrypt, session and SDP, mDNS, persistent RTSP/RTP and slow-reader disconnect, encrypted RTP-to-ALAC-to-PCM, ALAC reference/robustness, PCM ring/resampling quality, gain/mute/ramp, and diagnostic tone.
- One earlier full run stopped at `raop_receiver_test`'s 500 ms audio-observation assertion. The isolated test rerun and the subsequent full suite passed; cause remains unproven.
- PPU PRX link and `prx-gen check` passed: one load segment, 6094 relocations, one management export, four import libraries.
- Harness SELF and PKG built. Package listing confirms `USRDIR/ps3-airplay-receiver.prx` is present; package identity is `RPRX90001`, and local emulator title search found no existing directory for this ID.
- RPCS3 install/run was not performed because computer-use reported the Mac locked. No RPCS3 setting was changed and no physical console was contacted.

## Immediate results

- The resident PPU link succeeds with `-lm`.
- The host suite's synthetic encrypted receive/decode path produces test PCM that matches fixtures; this is host evidence only.
- The diagnostic PRX still discards PCM and does not submit audio to PS3 output.
- RPCS3 load/start/stop/unload is prepared but unverified pending access to the emulator UI.

## Known limitations / follow-ups

- Must test actual RPCS3 package install and three lifecycle cycles; current emulator UI is inaccessible while the Mac is locked.
- Need test worker startup failure, stop timeout, and repeated lifecycle paths under emulator where possible.
- No PS3 Cobra/Sprx compatibility, actual console audio output, or in-game coexistence claim is supported by this pass.
- Host RTP timeout was intermittent once; keep observation visible and investigate only if it recurs.

## Risks introduced

- Configured ports can still conflict with other emulator services; defaults remain unchanged for ordinary builds.
- The stop timeout uses the PS3 system clock and a bounded poll count; the PS3DK/runtime semantics still need emulator/hardware verification.
- Harness lifecycle API behavior may differ from Cobra or physical hardware and must not be treated as evidence of hardware compatibility.

## Decision record (embedded)

### Decision: fail closed on worker cleanup timeout

- **Context:** Joining a live worker risks hanging; forcibly closing sockets/logging resources could race with the worker.
- **Options considered:** Keep unbounded join; detach and abandon worker resources; wait boundedly for a cleanup signal and refuse stop on timeout.
- **Why chosen:** Only the bounded wait preserves resources when thread liveness is uncertain while allowing clean termination when observed.
- **Tradeoffs accepted:** A failed stop leaves the PRX resident and requires process-level recovery; it avoids pretending cleanup succeeded.

### Decision: configure separate test ports and logs

- **Context:** Host port 5000 is used by another macOS service, and diagnostic logs must not overwrite release diagnostics.
- **Options considered:** Stop/change the host service; use receiver ephemeral ports; compile the harness with distinct fixed ports and dedicated logging.
- **Why chosen:** Keeps host configuration untouched and makes emulator evidence repeatable.
- **Tradeoffs accepted:** Port selection remains environment-specific and is not proof that networking works under RPCS3.

### Decision: package the module into a separate diagnostic title

- **Context:** Repeated PRX APIs require a controlled app process and a unique title ID.
- **Options considered:** Install into an existing AIR PS3 title; invoke a raw ELF; use a dedicated PKG and title ID.
- **Why chosen:** An isolated package prevents interference with AIRPLAY02 and makes the PRX path deterministic.
- **Tradeoffs accepted:** Installation and runtime still depend on RPCS3's behavior, which is currently untested.

## Next recommended step

If local RPCS3 becomes accessible, install the package under `RPRX90001`, capture the exact load/start/stop/unload return values and app/module logs for all three cycles, and preserve the emulator's prior settings. Then finish the overnight report and hardware-readiness assessment.

Follow-up within this pass: start failure and a nonresident start now each attempt an explicit unload and record that result. The updated harness rebuilt successfully; package listing still contains the module payload. Final PKG SHA-256 after this harness update: `9e33f1efb81e9e89e6141cff875221b11c70169dddaaffe120d3e2ae3dc506d1` (the PKG container may vary between tool invocations; recompute after any rebuild).
