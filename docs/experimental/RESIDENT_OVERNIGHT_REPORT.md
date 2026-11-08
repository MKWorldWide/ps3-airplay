# AIR PS3 Resident Development Report

**Updated:** 2026-11-08 17:23 EST
**Branch:** `experimental/resident-airplay`
**Source baseline:** `53e24116e8d9a26db80deeafd568be72bf8cf6d6`
**Scope:** RPCS3 lifecycle and diagnostic harness; no physical PS3 was contacted.

## Executive summary

The dedicated `RPRX90001` harness now completes three consecutive RPCS3
module load/start/stop/unload cycles. Every cycle reached receiver readiness,
stopped its worker, unloaded with result zero, and returned the loaded-module
count to baseline six. The harness uses PS3DK's fake-signed SPRX container:
RPCS3 rejected the raw PRX with `0x80011148` (`CELL_PRX_ERROR_UNSUPPORTED_PRX_TYPE`).

RPCS3 cannot open the resident's file-log path. A small diagnostics structure
passed by the test harness permits lifecycle-only testing in that environment;
normal resident startup still fails when its required log cannot be opened.
The module did not receive a synthetic AirPlay stream and does not output PCM.
An app-level RPCS3 `cellAudio` probe now initializes an audio port, submits a
five-second tone, and cleans the port up successfully; this does not yet test
audio calls from the resident PRX or prove audible output.

**Hardware readiness: NOT READY.** The repeated lifecycle is now demonstrated
in RPCS3, but audio output, real-network receive, physical PRX loading, memory
use under stress, and game coexistence are untested. Do not install this
candidate on a physical console based on emulator lifecycle alone.

## Test results

| Area | Status | Observed result |
|---|---|---|
| Host protocol and crypto | PASS | Lima host suite passed RAOP crypto/session/discovery, persistent RTSP/RTP, encrypted receive/decode, ALAC bounds, PCM ring/resampling/gain, and sanitizer smoke tests. |
| PPU PRX build and structure | PASS | PS3DK 0.27.0 emitted the PRX and both SPRX containers. `prx-gen check`: one load segment, 6,101 relocations, one management export, four import libraries. |
| Package payload identity | PASS | `RPRX90001` package extracted and its embedded `.fake.sprx` matched the current PRX build output byte for byte. |
| Raw PRX load in RPCS3 | FAIL | `sys_prx_load_module` returned `0x80011148`, `CELL_PRX_ERROR_UNSUPPORTED_PRX_TYPE`; the loader reported “Failed to decrypt file.” |
| Fake-SPRX module start | PASS | The `.fake.sprx` loaded, returned resident status, reached startup stage 7, created the worker, and published receiver-ready. |
| Three RPCS3 lifecycle cycles | PASS | All three load/start/ready/stop/worker-exit/unload cycles completed. Stop and unload returned zero each time. |
| Module-list baseline | PASS | RPCS3 reported six modules before the run and after each unload; no module entry accumulated. This does not measure all sockets or memory allocations. |
| Resident file logger in RPCS3 | BLOCKED | Each run published `MODULE_LOG_UNAVAILABLE`; lifecycle diagnostics still worked through the harness-provided structure. The underlying filesystem/logger incompatibility remains unresolved. |
| RPCS3 cellAudio port probe | PASS (app-level API) | `cellAudioInit`, port open/start, 940 stereo blocks at 48 kHz, stop/close, and `cellAudioQuit` all returned zero. The 5-second 440 Hz tone was submitted through RPCS3's audio path; acoustic audibility was not independently recorded, and the PRX did not submit this tone. |
| Synthetic RAOP session in RPCS3 | UNTESTED | The harness does not yet drive a synthetic sender through the resident's RTSP/RTP path. |
| Valid PCM from resident PRX | UNTESTED | Host decoder tests produce valid PCM, but the RPCS3 lifecycle run did not send audio packets. |
| Resident audio output | UNTESTED | The PRX currently clears decoded PCM and has no cellAudio submission path. |
| Foreground/resident audio coexistence | UNTESTED | No two-source audio experiment was performed. |
| Physical PS3 / Cobra compatibility | UNTESTED | No console or boot-plugin setting was accessed. |

## Verified RPCS3 evidence

The final harness log reported for each cycle:

```text
MODULE_COUNT_BASELINE value=6
MODULE_START_STAGE value=7
MODULE_DIAGNOSTIC_EVENTS value=17
MODULE_LOG_UNAVAILABLE value=<cycle>
RECEIVER_READY value=<cycle>
STOP_RESULT value=0
WORKER_STOPPED value=<cycle>
UNLOAD_RESULT value=0
MODULE_COUNT_AFTER_CYCLE value=6
CYCLE_PASS value=<cycle>
```

It finished with `HARNESS_PASS value=3`. Event mask 17 is `STARTED` plus
`LOG_UNAVAILABLE`. The app log is local emulator output and was not committed.
Earlier failed attempts are visible in RPCS3's TTY history; only the last
successful run is the passing lifecycle result.

## Known defects and limits

1. Runtime logging is unavailable to the PRX in RPCS3. The test-only
   diagnostics path works, but it does not solve production logging.
2. This proves only the loaded-module count returns to baseline. Socket handles,
   heap usage, and every external resource were not independently measured.
3. `sysThreadJoin` is called only after the worker signals cleanup completion,
   but the join itself has no timeout and was exercised only in RPCS3.
4. The resident has no latency-sized PCM playback queue, playback clock,
   resampling-to-audio-port loop, or `cellAudio` output backend; decoded PCM is
   discarded.
5. No synthetic RAOP packet flow, audio output, foreground source, or physical
   console was tested in this pass.

## Candidate artifacts and SHA-256

Artifacts are in ignored `build/rpcs3-candidate/`; generated binaries and logs
are not committed.

| Artifact | SHA-256 |
|---|---|
| `ps3-airplay-receiver.prx` | `427445fa5313779af8344d9005906d5423662aee519022cf1754fa4ec6f50ef0` |
| `ps3-airplay-receiver.sprx` | `20b5ea0827275a219b544d9ad00cea671445e3ae6c09c80f413ca3aaa52e39db` |
| `ps3-airplay-receiver.fake.sprx` | `5f21acdd1e7111ba5ad42bfe76f6aef2180ffaa7d89291b4b701a925fb3b1786` |
| `ps3-airplay-resident-harness.elf` | `abf4ff126450ab80ae891217194f2bc97200444ee9515fbf226ad789a2bb97ed` |
| `ps3-airplay-resident-harness.self` | `750eb723a913eccde1f5f085d44b11d670dbac149301dfdc05dad71694a8ec3f` |
| `ps3-airplay-resident-harness.pkg` | `50c2c7160d5d2c1679c96a56359477c599ec6806cbbe2e50911d2060192009a1` |

## Safe next steps

1. Keep the hardware gate closed until a real resident audio-output path exists
   and a controlled synthetic RAOP stream yields observable PCM/output evidence.
2. Add a bounded PCM producer/consumer queue and a separately timed playback
   worker using PS3 `cellAudio` APIs; retain 44.1-to-48 kHz conversion and
   bounded buffering, and make audio-port startup/teardown retry-safe.
3. First verify audio-port API behavior in RPCS3 with a controlled test tone;
   then feed synthetic encrypted RAOP audio and measure queue depth,
   underflows, and clean stop.
4. Attempt foreground-source coexistence only after resident output works. A
   successful pair of emulator streams would not prove game mixing on hardware.
5. After a separate readiness review, propose a nonpersistent diagnostic PS3
   run with recovery instructions. Do not enable automatic loading or edit
   Cobra configuration.
