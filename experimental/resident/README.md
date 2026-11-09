# Resident PRX feasibility candidate

`module.c` wires diagnostic-only `_raop._tcp` mDNS, bounded persistent RTSP
session handling, three UDP channels with RTP parsing/queueing, PS3 network and
netctl initialization, Mbed TLS crypto, bounded ALAC decode, and capped file
logging into a PS3DK PRX start/stop lifecycle. Decoded PCM is transferred over
a bounded SPSC queue to a separate audio worker, buffered in the existing PCM
ring, resampled to 48 kHz, and submitted through a `cellAudio` port. The current
RPCS3 diagnostic mode can seed a synthetic 440 Hz buffer to exercise this path;
that seed is not AirPlay receive evidence or proof of audible output.

The session parses the bounded SDP codec/key/IV offer and recovers the key for
`et=1` using the standard AirPort RSA-OAEP/SHA-1 profile. Key/IV memory clears
on TEARDOWN and disconnect. The worker uses the PS3 RNG adapter for RSA
blinding and gets the local MAC through netctl for Apple-Response signatures.
L16 decode, sender-clock mapping, and a latency-sized scheduled jitter buffer
remain absent. It sends timing probes and bounded resend requests;
matching 0xD6 responses are sequence-inserted into the small bounded RTP queue,
but there is no scheduled playback-recovery guarantee.

## Build result

The existing Lima VM built `ps3-airplay-receiver.prx` plus both PS3DK `SIGN`
containers (`.sprx` and `.fake.sprx`) using `scripts/build-lima-resident.sh`.
PS3DK v0.27.0 `prx-gen check` accepted the `.prx`'s single load segment,
6,094 emitted relocations, 1 management export record, and 4 import records
(`sys_net`, `cellSysmodule`, `cellNetCtl`, `sysPrxForUser`). It does not validate
the SELF containers or Cobra loading. The build includes ALAC decode, timing
probes and bounded resend requests. The 2026-11-08 experimental lifecycle
candidate hashes are recorded in
`docs/experimental/RESIDENT_OVERNIGHT_REPORT.md`. Build hashes belong to exact
artifacts; do not substitute the raw PRX hash for a signed-container hash.

The worker uses PS3DK's lower-level `sysThreadCreate` / `sysThreadExit` /
`sysThreadJoin` path. It avoids the higher-level `sys_ppu_thread_create`
wrapper, which pulls in PS3DK's TLS-bearing POSIX thread shim. `prx-gen`
rejects TLS in modules, so changing host CPUs or compilers would not remove
that target-format constraint. The passing project link map contains no
`librt.a(pthread.o)` or pthread TLS symbols, and the linked ELF has no `.tdata`
or `.tbss` sections.

The listener writes RTSP request events from its single worker thread. The
module's bounded atomic lock serializes its own lifecycle/error writes. On
stop, the module waits up to five seconds for the worker's cleanup-complete
signal before joining. If the wait or join fails, it returns
`SYS_PRX_STOP_FAILED` without closing the log or network state. Network
finalize/unload errors are logged and retain their resource flags so stop can
be retried; cleanup does not claim success while an owned module remains.

## RPCS3 lifecycle harness

`scripts/build-lima-resident-rpcs3-harness.sh` builds the diagnostic PRX and a
separate homebrew test package with title ID `RPRX90001`. RPCS3 rejected the raw
`.prx` with `0x80011148` (`CELL_PRX_ERROR_UNSUPPORTED_PRX_TYPE`), so the
emulator harness embeds PS3DK's `.fake.sprx` container. This container is for
the local emulator test only and is not evidence of Cobra or hardware
compatibility. The app uses ports 5001/6100 and defaults to 50 load/start/stop/
unload cycles; the last completed RPCS3 run used three. It checks diagnostic
events and compares the module-list count with its pre-test baseline after
every unload. It fails if the 256-entry list
capacity is reached, so a truncated count cannot pass as leak-free. Each
package build is extracted and its embedded SPRX is compared byte for byte
with the current build output, guarding against stale SDK overlays.

For runtime analysis, the module also copies its internal event records into a
bounded, 128-entry multi-producer journal in the harness-provided diagnostics
structure. The harness drains committed records while waiting for lifecycle
events and again before unload. Each line includes a sequence, system-time
microseconds, internal event name, numeric result/value, and byte count. Queue
overflow and file-log write failure are reported. Event names and fields are
internal and deliberately exclude packet contents, RAOP credentials, peer
addresses, MAC addresses, and stream metadata. The harness log adds elapsed
milliseconds to its own events and remains at
`/dev_hdd0/tmp/ps3-airplay-resident-harness.log`.

Run `scripts/test-resident-diagnostics.sh` for the host concurrency, commit
ordering, and overflow-accounting test. It uses ASan/UBSan and writes its
binary only under ignored `build/`. This validates the shared journal code, not
the PPU ABI or RPCS3 runtime behavior; a fresh package run is still required.

The last completed 2026-11-08 RPCS3 run completed three clean audio-enabled
load/start/stop/unload cycles.
Each cycle reached `RECEIVER_READY`, stopped its worker, unloaded successfully,
and returned the module count to baseline six. RPCS3 does not permit this
module's file logger to open its path; the harness reports `LOG_UNAVAILABLE`
through a diagnostics structure passed to the module. The separate test app
and resident each submitted through distinct `cellAudio` ports, but the last
run used 440 Hz for both generated sources and did not capture audio. It
therefore proves API submission only, not distinguishable mixed sound,
synthetic RAOP receive, Cobra compatibility, hardware loading, or game audio
coexistence. The harness source now defaults to 50 cycles alternating between
no seeded resident PCM, stop immediately after resident PCM submission, stop
after an explicit audio-worker drain event, and a longer dual-port submission. Its foreground test
tone is 660 Hz so a future audio capture can distinguish it from the resident
440 Hz seed. This expanded harness has not yet been rebuilt or run in RPCS3.
See the overnight report for exact status and artifact hashes.

`scripts/validate-resident-containers.py` verifies that a freshly built signed
SPRX has PS3DK SELF type `0x00010001` and the emulator-only `.fake.sprx` has
type `0x80000001`. The harness package build runs this check before packaging
and requires its package to contain exactly one SPRX byte-matching the fresh
fake container. This distinguishes the two container formats; it does not
establish that the production SPRX is accepted by a particular Evilnat/Cobra
key table. No hardware-ready resident package has been produced or tested.

## Mocked host lifecycle simulation

`bash scripts/test-resident-host-lifecycle.sh` compiles the actual resident
`module.c` against mocked PS3 system-module, thread, receiver, logging, and
network-call interfaces. AddressSanitizer and UndefinedBehaviorSanitizer run
startup rollback, worker-create failure, worker stop, bounded stop timeout,
worker poll-error exit, retry after thread-join failure, and retry after
injected NetCtl unload, network finalize, and network-module unload errors.
It also checks that the receiver starts and cleans up if NetCtl module loading
or initialization fails, and that receiver-open failure exits with a logged
worker-stop event and can be joined and cleaned up. It also injects a failure
writing `RECEIVER_READY` and verifies the opened receiver is closed before the
worker exits. The address-probe socket calls are stubbed and assert the
expected multicast target; the test sends no packets and opens no socket.
Eleven cases passed in four consecutive full runs.

This checks resident control flow and error-state retention with host mocks. It
does not check PS3 syscall semantics, PRX loading, the PS3 scheduler, real
network behavior, or hardware compatibility. Set `MBEDTLS_INCLUDE` to the
pinned Mbed TLS 3.6.7 include directory before running the script in Lima.

## Limits and safety gates

- A format-valid PS3DK PRX is not yet proven compatible with Evilnat/Cobra 8.5
  or loadable on this console.
- PS3DK warns that real-signed `.sprx` acceptance depends on the target loader's
  key table; generating the two containers is not evidence either loads here.
- The module handles a subset of persistent RTSP control, including
  Apple-Challenge and encrypted session-key recovery, accepts bounded UDP RTP
  packets, decrypts and decodes supported ALAC frames, and advertises
  diagnostic-only discovery metadata. It does not implement L16 decode,
  latency-sized jitter buffering, sender-clock mapping, or audio output.
- Its file logging and listener lifecycle have not been exercised on hardware.
- Do not install or load it until the console model and a tested recovery route
  are recorded. Do not edit boot plugin configuration.
- Game audio coexistence remains a separate feasibility gate.

Host-side protocol tests pass for RTSP transitions, Apple-Challenge/session-key
recovery, AES decrypt, SDP field parsing, malformed/valid RTP queue handling,
and a loopback TCP+UDP session. These tests establish host protocol behavior
only; the module has not been exercised on the PS3.

The build is repeatable with `scripts/build-lima-resident.sh`; see the latest
decision and build report for the evidence and validation boundary.
