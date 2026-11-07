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
5,993 emitted relocations, 1 management export record, and 4 import records
(`sys_net`, `cellSysmodule`, `cellNetCtl`, `sysPrxForUser`). It does not validate
the SELF containers or Cobra loading. The build includes ALAC decode, timing
probes and bounded resend requests. The latest sequence-buffer PRX SHA-256
from 2026-11-06 is
`c5be7a6b47d648bd64891dc03b2d9b1e5500fa6d63e579bb7107d4863d5ecb13`.
Earlier container hashes from the 2026-11-06 21:50 America/New_York build:

| Artifact | SHA-256 |
|---|---|
| `.prx` | `c9847b6fe3e66be58fcaff5a0790505ef794011adde54c19e303b036fe1efc21` |
| `.sprx` | `c805fc00a12b764245210fffd30bc8b91dc61bc939e9e1d8e42fd9c9d4d8eec8` |
| `.fake.sprx` | `efc2422bb1ef242af7e03c02407cfc1db0f4024597c5f9ae1a386b3847d0a747` |

The raw `.prx` and `.fake.sprx` hashes were stable across two clean builds of
the final candidate. The real-signed `.sprx` hash changed between those builds;
the cause was not isolated, so treat that container output as nondeterministic
and record the hash of the exact artifact being evaluated.

The worker uses PS3DK's lower-level `sysThreadCreate` / `sysThreadExit` /
`sysThreadJoin` path. It avoids the higher-level `sys_ppu_thread_create`
wrapper, which pulls in PS3DK's TLS-bearing POSIX thread shim. `prx-gen`
rejects TLS in modules, so changing host CPUs or compilers would not remove
that target-format constraint. The passing project link map contains no
`librt.a(pthread.o)` or pthread TLS symbols, and the linked ELF has no `.tdata`
or `.tbss` sections.

The listener writes RTSP request events from its single worker thread. The
module's bounded atomic lock serializes its own lifecycle/error writes. On
stop, a successful worker join precedes final logging and cleanup. If join
fails, the module returns `SYS_PRX_STOP_FAILED` without closing the log or
network state; it deliberately skips logging that failure because the listener
may still be writing to the same stdio stream. This keeps resources resident
instead of tearing them out from under a live worker.

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
