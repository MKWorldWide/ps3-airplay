# Resident PRX feasibility candidate

`module.c` wires diagnostic-only `_raop._tcp` mDNS, bounded persistent RTSP
session handling, three UDP channels with RTP parsing/queueing, PS3 network and
netctl initialization, Mbed TLS crypto, bounded ALAC decode, and capped file
logging into a PS3DK PRX start/stop lifecycle. It answers Apple-Challenge,
recovers encrypted AES session keys, decrypts and decodes supported ALAC RTP
frames, logs periodic sample/byte counts, then clears payload and PCM. It has
no audio output and cannot play music yet.

The session parses the bounded SDP codec/key/IV offer and recovers the key for
`et=1` using the standard AirPort RSA-OAEP/SHA-1 profile. Key/IV memory clears
on TEARDOWN and disconnect. The worker uses the PS3 RNG adapter for RSA
blinding and gets the local MAC through netctl for Apple-Response signatures.
L16 decode, sender-clock mapping, a latency-sized jitter buffer, PCM buffering
and output remain absent. It sends timing probes and bounded resend requests;
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
separate homebrew test package with title ID `RPRX90001`. It embeds the raw
`.prx` in `USRDIR`, uses a separate runtime log, and defaults to test ports
5001/6100 so it does not require changing the receiver's standard 5000/6000
ports. The app attempts three load/start/stop/unload cycles, checks module log
events, and compares the module-list count with its pre-test baseline after
every unload. It fails if the 256-entry list capacity is reached, so a
truncated count cannot pass as leak-free. Each package build is extracted and
its PRX is compared byte for byte with the current build output, guarding
against stale SDK overlays.

The 2026-11-08 pass passed host tests, PPU linking, structural PRX checks, and
package payload comparison. It did not install or execute the harness in
RPCS3 because the local desktop was locked. No emulator result, Cobra
compatibility, hardware loading, resident audio output, or in-game coexistence
is established. See the overnight report for exact status and artifact hashes.

## Mocked host lifecycle simulation

`bash scripts/test-resident-host-lifecycle.sh` compiles the actual resident
`module.c` against mocked PS3 system-module, thread, receiver, logging, and
network-call interfaces. AddressSanitizer and UndefinedBehaviorSanitizer run
startup rollback, worker-create failure, worker stop, bounded stop timeout,
worker poll-error exit, and retry after injected NetCtl unload, network
finalize, and network-module unload errors. The address-probe socket calls
are stubbed and assert the expected multicast target; the test sends no
packets and opens no socket.

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
