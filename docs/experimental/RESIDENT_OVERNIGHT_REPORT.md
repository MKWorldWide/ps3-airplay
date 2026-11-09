# AIR PS3 Resident Development Report

**Updated:** 2026-11-08 19:05 EDT
**Branch:** `experimental/resident-airplay`
**Source baseline:** `53e24116e8d9a26db80deeafd568be72bf8cf6d6`
**Scope:** RPCS3 lifecycle and diagnostic harness; no physical PS3 was contacted.

## Latest audio-backend checkpoint (2026-11-08 17:56 EDT)

This checkpoint supersedes earlier statements below that the resident had no
audio backend. The experimental PRX now contains a bounded 16,384-frame SPSC
decoded-PCM queue, the existing PCM ring/resampler, a 2,048-frame prebuffer,
and a `cellAudio` output worker. A diagnostics-only 440 Hz seed verifies the
queue-to-output path; no RAOP packets were sent, so this does not prove received
AirPlay PCM playback. Host lifecycle mocks and queue/concurrency tests pass.

RPCS3 accepted the fake-SPRX package and both the harness and resident opened
separate `cellAudio` ports. The resident signaled that it submitted its
diagnostic PCM while the foreground harness submitted 940 blocks. Cycles 1 and
2 stopped and unloaded cleanly. Cycle 3 **FAIL**: RPCS3 terminated the
`ps3-airplay-audio` thread with a fatal null-address execution fault. The GUI
log reports a module location of `0x001201c8`, but relocation means a direct
comparison with the raw PRX `.text` address is not enough to identify a named
function. An earlier attribution of this offset to the `sysThreadExit` stub is
withdrawn. The exact call site and root cause remain unknown. No cycle-3
stop/unload or module-baseline result was recorded.
The three-cycle two-port run therefore fails and audio-thread teardown remains
unresolved. A prior lifecycle-only harness run completed three clean cycles;
that result does not qualify the new audio-thread path.

The emulator also reported `CELL_AUDIO_ERROR_ALREADY_INIT` on the resident's
borrowed mixer path, as expected after the harness owns audio initialization,
and `sys_net` reported `EADDRNOTAVAIL` when the resident attempted local
network discovery. Neither error explains the audio-thread fault. Acoustic
output was not independently heard or captured. No hardware test was made.

Two-port run hashes: PRX `9a558a5b994f68e109e0bed8c4564e7d54b4c5463e1190d449df9571d7c1e5b2`,
fake SPRX `7bfea5421d546fec46a9020ecedd4fafae61cf03594b85480087d1dd20aa6120`,
and harness PKG `86db3830dfbd08c73337ea5a4a8ec19d416cee919783f17360b788602bd49934`.
The installed package's embedded fake SPRX matches the reported fake-SPRX
hash. Artifacts remain in ignored local build folders.

## Trace-instrumentation build (2026-11-08 19:07 EDT)

Added harness-only stdout phase markers around audio-worker exit, receiver
worker join, audio exit observation, thread join, and audio resource release.
The Lima host lifecycle/audio tests pass again. The PPU build and `prx-gen
check` pass with five import libraries, one load segment, and 6,298
relocations. After explicitly removing the old ignored PKG output, the
`RPRX90001` package rebuilt successfully; its embedded fake SPRX matches the
new build exactly.

Trace candidate hashes: PRX
`629f430bf33b5f8020e525cbfac27d25694829133fa835e8b7eb9db3fad976df`, fake
SPRX `7e7b0e42d7ac7857bac20691bd979395e525e706b42ab97553ce9c41d70c0d39`, and
PKG `c91a85999d21c18f2bce244e7c41b2c5e4aa7c5c605bbe202e6aeafb3d1a24ff`.
The package is in ignored local output and has not yet been installed or run.
The RPCS3 desktop reported the Mac locked, so no new emulator result exists
for these markers. The previous two-cycle pass / third-cycle crash remains the
latest runtime evidence. The exact crash call site remains unknown.

## Audio-thread startup publication follow-up (2026-11-08 19:11 EDT)

Source review found `audio_thread_started` was written by the module-start/stop
thread and read by the receiver worker without atomic synchronization. The
worker now reads it with acquire semantics, and startup publishes the ready
state with release semantics after queue initialization and before thread
creation. Thread-creation failure and post-join cleanup clear it atomically.
This closes a real cross-thread data race; it does not establish the cause of
the prior third-cycle crash.

Host lifecycle/audio and SPSC concurrency tests passed again. The PPU build and
`prx-gen check` passed with five imports, one load segment, and 6,298
relocations. The rebuilt package's overlay fake SPRX matches the new build.
Latest artifacts: PRX
`5ebf2ab703df8986d3a62864db03adcc9989d3eb9b39b6507eacce0865fe7355`, fake
SPRX `e4507877dce48fcf4f7874de352e90ead57c283a84a47fc66b4504603ddb9c14`,
PKG `d803bfd2daf94e6dbf7a61434330077f5e5b55418831d389a12ffbf0fb888a3c`.
The earlier trace-only package is preserved in ignored output. RPCS3 remains
unrun for both candidates because the desktop is still locked.

## Host receive-to-output integration (2026-11-08 19:17 EDT)

Extended the Lima RAOP socket integration test so a successfully decrypted and
ALAC-decoded frame passes through the bounded SPSC queue, into the production
PCM ring, and through its 44.1-to-48 kHz float renderer. The build script now
links the queue implementation into this test. The targeted binary passed
under ASan/UBSan: exact PCM comparison passed, 353 decoded frames were queued
and consumed, 256 output frames rendered with no underflow, and output peak
exceeded the non-silence threshold. This is host receive-to-PCM-render evidence;
it does not exercise PS3 `cellAudio`, the resident PRX, audible output, or
RPCS3.

The complete `scripts/build-lima-crypto.sh` gate was rerun after synchronizing
the edited files into Lima and verifying matching SHA-256 values. All RAOP
crypto, RTSP/SDP, mDNS, socket loopback, encrypted ALAC, logging, PCM ring,
resampling/gain, diagnostic tone, ASan/UBSan, and PPU component-object checks
passed. The initial attempt had run against stale Lima copies and is excluded
from the passing result.

At that checkpoint, the RPCS3 UI was inaccessible while the Mac reported
locked. The next checkpoint records the subsequent unlock and emulator rerun.

## Audio worker stack correction and RPCS3 retest (2026-11-08 19:05 EDT)

The prior crash investigation measured `sizeof(raop_pcm_ring)` at 65,864 bytes.
The audio worker also held a 4,096-byte staging array and a 2,048-byte output
array on its configured 65,536-byte stack, for at least 72,008 bytes of local
buffers before ordinary call frames. The ring, staging, and output buffers are
now static module storage owned by the single audio worker. This is a
well-supported stack-overflow hypothesis, but the change alone does not prove
the old fault's exact instruction.

After the user unlocked the Mac, the rebuilt `RPRX90001` package was installed
and run in RPCS3. Its embedded fake SPRX matched the intended build. The fresh
`/dev_hdd0/tmp/ps3-airplay-resident-harness.log` contains three complete
audio-enabled cycles: each cycle loaded and started the module, reached
`RECEIVER_READY`, reported resident PCM submission and two active audio ports,
then returned stop and unload results of zero and module count six, matching
the initial baseline. The final event is `HARNESS_PASS value=3`. The app-level
foreground probe submitted 940 blocks; the resident API submission event was
also observed. This verifies concurrent API submissions and clean emulator
module teardown for these three runs. It does not verify audible sound, RAOP
packets reaching the PRX, real game mixing, or PS3/Cobra behavior.

RPCS3's accumulated GUI log still contains a fatal audio-thread fault from an
earlier run. The isolated latest harness log is complete and passes. Network
startup continues to encounter RPCS3 `EADDRNOTAVAIL`, and the resident file
logger remains unavailable; the harness diagnostics channel supplies lifecycle
evidence. Current hardware readiness remains NOT READY.

Current ignored artifact hashes: PRX
`c5fd511eb4617aedd9ad7e1949ae25a562cb6f854ff2017180206163ae1fc8ed`, fake
SPRX `5954cf7a340d3684ea205ce86bd7268ee7127fcf6e75d5be0c228b29f90b1bab`,
harness ELF `abf4ff126450ab80ae891217194f2bc97200444ee9515fbf226ad789a2bb97ed`,
SELF `750eb723a913eccde1f5f085d44b11d670dbac149301dfdc05dad71694a8ec3f`, and
PKG `d5385a1b8c0acf1f21b7bbad1804486118ddbd02a510fe4871dce9ae77dfaf9d`.
The installed fake SPRX hash was recomputed and matched the local artifact.

**Hardware readiness: NOT READY.** The emulator lifecycle gate now passes.
Next validate synthetic RAOP through resident decode and output, then obtain
independent evidence of audible output before proposing a nonpersistent
hardware test.

## Current result table

| Area | Status | Current observation |
|---|---|---|
| Resident bounded PCM path | PASS (structural/host) | Decoded PCM queue, existing resampler, prebuffer, and cellAudio output worker are implemented; host API mocks and a receive-to-render pipeline test pass. |
| RPCS3 fake-SPRX load/start | PASS | The harness package loads and the PRX reaches ready state. The raw PRX is not accepted by this RPCS3 configuration. |
| Audio submission in RPCS3 | PASS (API submission only) | Resident diagnostic PCM and foreground tone submissions succeeded through separate ports. Sound was not independently confirmed. |
| Three-cycle audio teardown | PASS | Rebuilt candidate completed three audio-enabled cycles; each stop and unload returned zero and each worker reported stopped. |
| Module-list baseline | PASS | Harness log recorded initial count six and count six after each of three unloads. This is not a full socket or heap leak check. |
| Two-port coexistence | PASS (submission only) | Harness submitted 940 foreground blocks while the resident reported PCM submission through its separate port. Acoustic mixing was not verified. |
| Synthetic RAOP decode-to-output | PASS (host PCM render) / UNTESTED (RPCS3) | Lima socket test sends encrypted RTP through decrypt, ALAC, SPSC, PCM ring, and 44.1-to-48 kHz rendering without underflow. No PRX/RPCS3 receive-to-audio run is established. |
| Physical console / Cobra / in-game audio | UNTESTED | No physical PS3 was contacted. |

## Earlier lifecycle-only checkpoint (superseded)

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
| PPU PRX build and structure | PASS | Latest PS3DK 0.27.0 build emitted the PRX and both SPRX containers; structural check reports one load segment, 6,298 relocations, one management export, and five import libraries. |
| Package payload identity | PASS | `RPRX90001` package extracted and its embedded `.fake.sprx` matched the current PRX build output byte for byte. |
| Raw PRX load in RPCS3 | FAIL | `sys_prx_load_module` returned `0x80011148`, `CELL_PRX_ERROR_UNSUPPORTED_PRX_TYPE`; the loader reported “Failed to decrypt file.” |
| Fake-SPRX module start | PASS | The `.fake.sprx` loaded, returned resident status, reached startup stage 7, created the worker, and published receiver-ready. |
| Three RPCS3 lifecycle cycles | PASS | Latest audio-enabled rerun completed all three load/start/ready/stop/worker-exit/unload cycles. Stop and unload returned zero each time. |
| Module-list baseline | PASS | RPCS3 reported six modules before the run and after each unload; no module entry accumulated. This does not measure all sockets or memory allocations. |
| Resident file logger in RPCS3 | BLOCKED | Each run published `MODULE_LOG_UNAVAILABLE`; lifecycle diagnostics still worked through the harness-provided structure. The underlying filesystem/logger incompatibility remains unresolved. |
| RPCS3 cellAudio port probe | PASS (API submission) | App-level `cellAudioInit`, port open/start, 940 stereo blocks at 48 kHz, stop/close, and `cellAudioQuit` returned zero. The foreground probe submitted a 5-second 440 Hz tone; the PRX separately submitted seeded diagnostic PCM. Acoustic audibility was not independently recorded. |
| Synthetic RAOP session in RPCS3 | UNTESTED | The harness does not yet drive a synthetic sender through the resident's RTSP/RTP path. |
| Valid PCM from resident PRX | UNTESTED | Host decoder tests produce valid PCM, but the RPCS3 lifecycle run did not send audio packets. |
| Resident audio output | PASS (API submission only) | Resident diagnostic PCM reached `cellAudioAdd2chData`; RAOP-produced PCM in the PRX and acoustic output remain untested. |
| Foreground/resident audio coexistence | PASS (API submission only) | Two ports accepted submissions in the same run. No foreground game or acoustic mixing validation was performed. |
| Physical PS3 / Cobra compatibility | UNTESTED | No console or boot-plugin setting was accessed. |

## Verified RPCS3 evidence

The latest harness log reported for each cycle:

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
4. The former third-cycle audio-worker crash followed a local buffer footprint
   larger than the configured worker stack. Moving those buffers to static
   storage is followed by three clean emulator cycles, supporting (but not
   proving) stack exhaustion as the cause. Output submission is not proof of
   audible output.
5. Host synthetic RAOP receive-to-PCM render is tested, but no synthetic RAOP
   packet flow has yet run through the resident PRX in RPCS3. No physical
   console was tested.

## Candidate artifacts and SHA-256

Artifacts are in ignored `build/rpcs3-candidate/`; generated binaries and logs
are not committed.

| Artifact | SHA-256 |
|---|---|
| `ps3-airplay-receiver.prx` | `c5fd511eb4617aedd9ad7e1949ae25a562cb6f854ff2017180206163ae1fc8ed` |
| `ps3-airplay-receiver.fake.sprx` | `5954cf7a340d3684ea205ce86bd7268ee7127fcf6e75d5be0c228b29f90b1bab` |
| `ps3-airplay-resident-harness.elf` | `abf4ff126450ab80ae891217194f2bc97200444ee9515fbf226ad789a2bb97ed` |
| `ps3-airplay-resident-harness.self` | `750eb723a913eccde1f5f085d44b11d670dbac149301dfdc05dad71694a8ec3f` |
| `ps3-airplay-resident-harness.pkg` | `d5385a1b8c0acf1f21b7bbad1804486118ddbd02a510fe4871dce9ae77dfaf9d` |

## Safe next steps

1. Drive a controlled synthetic encrypted RAOP session through the resident
   PRX in an environment with working networking; verify decoded PCM reaches
   its bounded queue and `cellAudio` submission path.
2. Capture or listen for the resident output independently. Do not treat API
   submission events as acoustic evidence.
3. Measure socket/heap cleanup and test repeated stop/restart under network
   failures; module-list stability alone is not a full leak check.
4. Only after those results, review a nonpersistent physical PS3 diagnostic
   plan with recovery instructions. Do not enable automatic loading or edit
   Cobra configuration.
