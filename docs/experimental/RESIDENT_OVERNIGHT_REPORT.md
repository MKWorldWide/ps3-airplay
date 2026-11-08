# AIR PS3 Resident Overnight Report

**Report timestamp:** 2026-11-08 07:37 EDT

**Branch:** `experimental/resident-airplay`

**Receiver/source baseline:** `53e24116e8d9a26db80deeafd568be72bf8cf6d6`; resident module source SHA-256 `66e9352402a374eeab92e248d91316e0cfb7a69e460d536b690e6508fd733579`.
**Validation boundary:** host/Linux and PS3DK build tools only; RPCS3 execution and physical hardware were not exercised in this pass.

## Executive summary

The experimental resident PRX now has a bounded worker cleanup wait, preserves network-module ownership when finalization or unload fails, and provides a separate RPCS3 package/harness for three load/start/stop/unload cycles. The Lima build, host regression suite, PRX structural check, package build, and an extracted-payload comparison passed. The build script now rejects a package containing an out-of-date PRX.

**Hardware readiness: NOT READY.** The local Mac was locked, preventing installation or execution of the RPCS3 harness. The emulator lifecycle APIs, PRX startup/stop callbacks, and cleanup retries therefore have no runtime evidence. Do not install or activate this candidate on a physical PS3 yet.

## Test results

| Area | Status | Observed result |
|---|---|---|
| Branch isolation | PASS | All source and documentation changes are on `experimental/resident-airplay`. Stable `main` remained clean at the initial base commit. No release, tag, or physical console was changed. |
| Host crypto/session/discovery suite | PASS | Base64/AES/RSA, encrypted RTSP key recovery, session transitions, bounded SDP parsing, and mDNS packet construction passed. mDNS result: 395 bytes, five records. |
| RTSP/RTP loopback receiver | PASS | Persistent RTSP, RTP reception, timing, resend recovery/rejection, FLUSH, TEARDOWN, and bounded slow-reader disconnect passed in the full suite and later repeats. |
| Receiver observation timing | FAIL (one occurrence; not reproduced) | An earlier suite run failed `raop_receiver_test.c:331` at `atomic_load_explicit(&args.audio_seen, memory_order_acquire)` after its 500 ms observation deadline. The isolated rerun, next full suite, three prior repeats, and ten additional consecutive targeted runs passed. Cause remains unproven. |
| Encrypted receive/decode/PCM | PASS (host only) | Synthetic encrypted RTP was decrypted, ALAC-decoded, and compared with reference PCM. Exact fixture decode passed; FFmpeg reference comparison passed for 353 samples. Three additional encrypted pipeline repeats passed. |
| Malformed input and ALAC bounds | PASS (host only) | Boundary-size ALAC frames and sanitizer smoke passed, including 4,096 seeded mutations plus 256 random packets. |
| PCM ring and resampling | PASS (host only) | 44.1→48 kHz tests produced 48,000 frames per case with zero underflows; SNR ranged from 94.75 dB at 100 Hz to 75.07 dB at 20 kHz. Gain/mute/ramp tests passed. |
| RTP order/loss/recovery stress | PASS (host simulation) | Sequence wrap, reordering, delayed retransmission, concealment deadline, resend rejection, flush reset, a 10,000-packet jitter schedule, and bounded resend retries passed. A gap counter records temporary sequence gaps even when an out-of-order packet later fills them; interpret it as a gap observation, not definitive lost audio. |
| Repeated Linux forensic suite | PASS | A further AddressSanitizer/UndefinedBehaviorSanitizer run passed the RTP queue stress, long SRC, 10,000-packet jitter schedule, display-resume checks, ALAC parse/decode, resend bounds, aggregation, and timing probes. These remain host-side tests. |
| Targeted receiver repetitions | PASS | Ten consecutive runs of the existing RAOP receiver test passed persistent RTSP, UDP RTP loopback, and bounded slow-reader disconnect checks. This reduces, but does not eliminate, concern about the earlier 500 ms observation timeout. |
| Mocked resident lifecycle simulation | PASS (host only) | AddressSanitizer/UndefinedBehaviorSanitizer ran actual `module_start`/`module_stop` control flow with mocked PS3 APIs and no network. Eleven startup/stop/error-retry cases passed in four consecutive full runs, including receiver-open and receiver-ready log failures, NetCtl load/init recovery, and retry after a join error. This does not establish PS3 syscall or PRX runtime behavior. |
| Long PCM stress | PASS (host simulation) | 600-second-equivalent resampling workload completed with zero underflows and phase reset to zero. |
| Resident PPU build | PASS | PS3DK 0.27.0 / Mbed TLS 3.6.7 linked the PRX and emitted `.sprx` and `.fake.sprx` containers. |
| Final full build and package integrity | PASS | Re-ran the full Lima host/build/package workflow after the early-worker-exit diagnostic fix. Host regression tests, PRX structure check, harness SELF/PKG build, and extracted PRX byte comparison passed; hashes below are from this rebuilt candidate. |
| PRX structural validation | PASS | `prx-gen check`: one load segment, 6,094 relocations, one management export, four import libraries (`sys_net`, `cellSysmodule`, `cellNetCtl`, `sysPrxForUser`). |
| Harness package | PASS | Diagnostic package title ID `RPRX90001`; package listing contains the module. The extracted module compared byte-for-byte with the current built PRX. |
| Harness module-count baseline check | BLOCKED | The harness compiles a check that `sys_prx_get_module_list` returns to its pre-test count after each of three unloads, and fails closed on query errors or a full 256-entry list. The package was not run, so counts and runtime API behavior are unknown. This check does not measure sockets or all allocations. |
| RPCS3 install and 3-cycle runtime | BLOCKED | Computer-use reported that the Mac is locked. No package was installed or run in RPCS3. The installed 0.0.43-20570 build's matching source registers load/start/stop/unload APIs, but that code-level check is not runtime evidence. |
| RPCS3 command-line fallback | BLOCKED | A locally launched `rpcs3 --help` process produced no output or exit within ten seconds; only that probe process was terminated. The already-running RPCS3 instance and its settings were left alone, and no package was installed. This does not establish emulator incompatibility. |
| Resident PS3 audio output | UNTESTED | The current PRX decodes frames, logs counts, then clears PCM. It has no PS3 audio output path. Host PCM tests do not establish sound output from the PRX. |
| Background/game coexistence | UNTESTED | No PCM output queue or PS3 audio submission exists, so no coexistence experiment was performed. |
| Physical PS3/Cobra compatibility | UNTESTED | No physical console was contacted. |

## Verified findings

- `module_stop` signals the worker, waits at most five seconds for its cleanup-complete flag, then joins. On timeout or join failure it returns `SYS_PRX_STOP_FAILED` without closing resources that might still be used.
- Worker exits caused by receiver-open or readiness-log failure now emit `WORKER_STOPPED` before publishing completion, so lifecycle diagnostics cover those early exits as well as the normal poll/stop path.
- NetCtl/Net module unload and network finalize errors are logged. The ownership flag stays set and stop fails so the callback can retry. `cellNetCtlTerm` has a void API, so it cannot report a failure.
- The resident target uses a 64 KiB worker stack. The linked PPU image reports 719,914 bytes text, 24,432 data, and 594,200 BSS; runtime allocations and peak RSS were not measured.
- Host tests verify synthetic RAOP protocol/audio behavior only. They do not establish PS3 scheduler behavior, NetCtl behavior, PRX loader compatibility, physical discovery, or audible output.
- The mocked lifecycle simulation compiles the resident module's actual start/stop functions but replaces PS3 APIs, receiver, mDNS, logs, and address-probe socket calls. It verifies selected control-flow/error-retry paths only; it cannot prove that the PS3 or RPCS3 APIs behave the same way.
- The installed RPCS3 build corresponds to source commit `13a3cd4266158911a8c555dda59005bc20fd2a66`; its [`sysPrxForUser` implementation](https://github.com/RPCS3/rpcs3/blob/13a3cd4266158911a8c555dda59005bc20fd2a66/rpcs3/Emu/Cell/Modules/sys_prx_.cpp) registers PRX load/start/stop/unload. The harness APIs are therefore present in that build's source, while actual loading and callback behavior remain untested.
- The same build's [LV2 PRX loader](https://github.com/RPCS3/rpcs3/blob/13a3cd4266158911a8c555dda59005bc20fd2a66/rpcs3/Emu/Cell/lv2/sys_prx.cpp) resolves the supplied VFS path, decrypts/parses the module, initializes it, and returns explicit unsupported-type or illegal-library errors on relevant failures. This informed the harness error logging; it does not predict this candidate's result.
- The experimental harness uses a separate log and ports 5001/6100. The ordinary receiver defaults remain log `/dev_hdd0/tmp/ps3-airplay.log` and ports 5000/6000.
- A forensic queue test confirms the missing-packet counter increments when a sequence gap is first observed even if a later out-of-order packet fills it. This is a statistics limitation; the tested queue did not conceal that recovered packet.

## Known defects and blockers

1. RPCS3 has not installed or executed the harness. The three PRX lifecycle cycles, actual import resolution, worker start/stop, and cleanup are all unverified.
2. A successful build and PRX structural check do not establish signed-container loading, Cobra compatibility, or Evilnat compatibility.
3. The PRX has no playback clock, latency-sized jitter buffer, PCM output queue, or PS3 audio submission. It currently discards decoded PCM.
4. No real-network mDNS advertisement, Apple-device session, physical PS3 run, or game coexistence test was performed in this pass.
5. `sysThreadJoin` has no timeout in the used API. It is called only after the worker signals that receiver and discovery cleanup finished, but the join path still needs runtime validation.
6. One host receiver test failed once at a 500 ms observation deadline and then passed in isolated, full-suite, and repeat runs. Keep monitoring for recurrence rather than treating it as fixed.

## Candidate artifacts and SHA-256

Local artifacts are in the ignored `build/rpcs3-candidate/` directory and are not committed.

| Artifact | SHA-256 |
|---|---|
| `ps3-airplay-receiver.prx` | `999c69f55861191b90c76d5353885b28be142b601f9cbed98322bb4e433b6dd8` |
| `ps3-airplay-receiver.sprx` | `fd2983fda382a6dbe872c5bf4c0ca58390078a1675b07dee3957f4ff12f6dd2e` |
| `ps3-airplay-receiver.fake.sprx` | `e1c8a1b5c356f44f491d1311eada98772b8c0f52ca0725b91894cce76bf8a2bc` |
| `ps3-airplay-resident-harness.elf` | `427269f496c1795ee95c5757f8d2183915af21987cf6fb5d2cfd8dc899be34b5` |
| `ps3-airplay-resident-harness.self` | `fbe79c23ffb392cd4a5ebb7cd89aa7422df91d15afe41286bc722cf405fae85b` |
| `ps3-airplay-resident-harness.pkg` | `65aa04b3ca1a0925e2aec4d2d2d4a0c90122e4c8523fd0d2f4014b2bbb93e0a8` |

The PRX embedded in the PKG was extracted and matched the PRX hash above. Recompute hashes after any rebuild; signed package/container values may vary.

## Hardware readiness assessment

**NOT HARDWARE READY.** Structural PRX and package checks passed, but the required module lifecycle runtime test is unavailable, and cleanup behavior remains unobserved in RPCS3. There is also no audio output implementation to validate. The hardware gate is therefore not met.

### Proposed controlled test plan after the gates pass

1. First install and run the `RPRX90001` package in local RPCS3. Require three complete cycles with load, start, `RECEIVER_READY`, stop, worker cleanup, unload, and zero accumulated handles before considering hardware.
2. On the test day, confirm the console is Evilnat 4.93 and record its model and current recovery route. Keep boot-plugin/Cobra autoload settings untouched.
3. Install only the isolated diagnostic homebrew package. Do not configure a persistent boot plugin. Keep a known recovery path available before starting.
4. Run the harness once, collect app/module logs, and require all lifecycle events and a clean return to XMB. If load/start/stop/unload returns an error, stop; do not repeat or enable persistence.
5. If lifecycle passes, use a controlled synthetic sender on the local network to check discovery, RTSP negotiation, encrypted RTP, and decode logs. Stop after protocol verification; the PRX has no audible output to evaluate.
6. Remove the diagnostic title after the test if desired. Consider real AirPlay or background coexistence only in a later, separately approved phase after PCM output exists and has its own safety/recovery plan.

No hardware step above was performed during this overnight session.
