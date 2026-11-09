# AIR PS3 Resident Development Report

**Updated:** 2026-11-09 00:07 EDT
**Branch:** `experimental/resident-airplay`
**Source baseline:** `86781f5`
**Scope:** Hardware diagnostic harness package build/install; no Cobra settings changed.

## Independent signing and module-format comparison (2026-11-09 00:07 EDT)

The `00.03` package launched on the development PS3. Both generated comparison
modules returned `0x80010007` (`ENOEXEC`) from `sys_prx_load_module` before
`module_start`. The minimal probe had no imports; the SDK-layout reference had
the standard module info and export record. The reported module count remained
11 after each failed load (the baseline was 11), and all harness-held modules
were released back to the application baseline of 9. The `00.03` run stopped
on the first failed attempt for each candidate, so no start/stop/unload cycle
was observed.

The two probes were not an independent signing comparison: both used the same
PS3DK `make_sprx` route as the resident. To separate the signed-container
metadata from the raw module, compared a released webMAN MOD 1.47.50 plugin and
its public build command with the resident build. The released module and
current PS3DK SPRXs share auth ID, vendor ID, program type and SCE version, but
the SELF flags differ: the known-good-style scetool output uses key revision /
flags `0x000A`, while the current `make_sprx` output uses `0x0001`. Separately,
the extracted reference plugin has two load segments plus `PT_SCE_PPURELA`;
PS3DK's current `prx-gen` candidate has one RWX `PT_LOAD` plus that relocation
segment. These are concrete comparison differences, not yet a proven cause of
the hardware rejection. The reference source uses scetool with key revision
`0A` and the same public CEX module identity.

Built a `00.04` diagnostic package with two minimal/reference probes signed by
scetool using the public CEX plugin settings, while preserving the resident's
current SPRX unchanged. The harness now makes all three configured failed-load
attempts for each probe and records module count after each rejection. Extracted
package SFO reports `RPRX90002`, `APP_VER=00.04`, and `VERSION=00.04`; the PKG
contains the unchanged resident plus the two independently signed probes. The
two probe SELF headers are flags/type `0x000A/0x0001`. The hardware package and
probe hashes are below. The console could not be reached from the development
host when preparing this follow-up, so `00.04` has **not** been uploaded,
installed, or run. `00.03` remains the latest observed hardware run.

| Check | Status | Evidence |
|---|---|---|
| `00.03` minimal and SDK-layout probes | FAIL | Both load calls returned `0x80010007` before `module_start`. |
| Module count after rejected loads | PASS | Count remained at 11 after each; final cleanup returned to app baseline 9. |
| Independent known-good-style SELF comparison | PASS | webMAN MOD release plugin uses SELF flags/key revision `0x000A`; current `make_sprx` outputs use `0x0001`. |
| Raw PRX layout comparison | PASS | Reference plugin: two `PT_LOAD` segments + `PT_SCE_PPURELA`; current PRX: one RWX `PT_LOAD` + relocation segment. |
| `00.04` package, SFO and candidate signing | PASS | Package extracts; SFO and three module identities validate; probe metadata is `0x000A/0x0001`. |
| `00.04` hardware installation and runtime | BLOCKED | Development console was unreachable from the host; no upload/install was attempted. |
| Resident start/stop, RAOP, PCM or audible output | UNTESTED | The `00.03` resident and comparison probes never reached `module_start`; `00.04` was not run. |

`00.04` local ignored package SHA-256:
`af75bf0150629064143117d44f38c184f754704336c62e19c30f68e9e5770e2e`.
Probe SPRX SHA-256 values:

| Artifact | SHA-256 |
|---|---|
| `resident-loader-minimal.sprx` (scetool, key revision `0x0A`) | `62d2f94a07dc41dc984e52381683fcd59c537b642cd20a2536e046eacdbff0b7` |
| `resident-loader-reference.sprx` (scetool, key revision `0x0A`) | `d9c0579b80a199690089fd72732499a0d0d395b5282033cb10d158938d51a2b0` |

The next experiment is to install and run `00.04` on the dev PS3, compare each
probe's load/start/stop/unload and module counts, then change only the dimension
that the result isolates. Keep the receiver hardware gate BLOCKED until a
production resident completes a clean lifecycle cycle. No raw console log,
device address, key file, signing secret, or external package is tracked.

## SFO 00.02 reinstall and repeat loader result (2026-11-08 23:20 EDT)

Advanced both harness SFO fields to `00.02`, rebuilt the package, and
independently inspected its extracted SFO. The installed title now reports
`TITLE_ID=RPRX90002`, `APP_VER=00.02`, and `VERSION=00.02`. The installed
resident SPRX hash is `9fae9ed955c023aca2077654625510e80766f899c9eaa4998cad4af5f1588817`,
matching the production signed SPRX embedded in the package.

The `00.02` harness launched and wrote a fresh run ID. Its first resident load
again returned `0x80010007` (`ENOEXEC`); it stopped after cycle one and closed
the foreground audio port. This confirms the SFO version is accepted for the
app, while the resident module loader failure remains unresolved. No
`module_start`, resident worker, RAOP input, resident PCM, or audible output
was observed. Hardware readiness remains BLOCKED.

| Check | Status | Evidence |
|---|---|---|
| Package SFO | PASS | Extracted SFO reports title `RPRX90002`, `APP_VER=00.02`, `VERSION=00.02`. |
| Install and app launch | PASS | Installed SFO verified over FTP; fresh harness run ID recorded. |
| Installed SPRX identity | PASS | Installed SPRX SHA-256 matches the signed package payload. |
| Resident loader | FAIL | Fresh run returns `0x80010007` (`ENOEXEC`) at cycle one. |
| Audio cleanup after rejected load | PASS | Audio port stop, close and quit return zero. |
| Resident networking/audio/lifecycle | UNTESTED | Module never reached `module_start`. |

Local ignored package SHA-256:
`c070380805764adfef94d10ede277c240e84db6379bfd8c81ccbcbb92370fa94`.
Console log SHA-256:
`8501b5c10846ac2a12ee2a9d97621682dae59ff5c57c85a6d651e7f5e985784f`.
Raw logs and package binaries remain ignored in `build/resident-hardware/`.

## Preload resident import libraries before PRX load (2026-11-08 23:31 EDT)

The first `00.02` run showed the harness attempted to load the resident before
loading the `sys_net` and `cellNetCtl` system modules it imports. Updated the
test harness to load both modules before `sys_prx_load_module` and release its
own module references at every harness exit. The resident itself still retains
its own network initialization and cleanup behavior.

The rebuilt `00.02` package installed. The console log confirms both preload
calls returned zero and both matching unload calls returned zero. Despite that,
the resident load again returned `0x80010007` (`ENOEXEC`) on cycle one. This
eliminates missing `sys_net`/`cellNetCtl` module preload as a sufficient cause;
it does not identify the remaining loader issue. The audio port and harness
preload references were closed after failure. No resident worker, receiver, PCM,
or sound was exercised.

| Check | Status | Evidence |
|---|---|---|
| Host and PPU build suite | PASS | Build script completed host protocol/audio/sanitizer tests and rebuilt PPU outputs. |
| Package SFO and SPRX contents | PASS | `RPRX90002`, both versions `00.02`; embedded production SPRX hash matches. |
| Installed EBOOT identity | PASS | Installed EBOOT hash matches package-extracted EBOOT. |
| Network module preloads | PASS | `CELL_SYSMODULE_NET` and `CELL_SYSMODULE_NETCTL` load results are zero. |
| Resident SPRX load | FAIL | `0x80010007` (`ENOEXEC`) persists at cycle one. |
| Harness network-module cleanup | PASS | Both module unload results are zero after load failure. |
| Resident lifecycle/audio | UNTESTED | Resident module never reaches `module_start`. |

Updated package SHA-256:
`c96644d84c3e7b7b365f9a1a84342b624dd111c6557c9055ab49ed0a174431b1`.
Fresh log SHA-256:
`c74c095a9dd0dd3e8a7769ad0fa36168fe1cc8f74fdf37ffd04a9dbb6288f1f9`.

## Hardware harness SFO version correction (2026-11-08 23:00 EDT)

The first `RPRX90002` package installed, and its console-side SPRX matched the
fresh signed build hash. The user reported the same launch error as the prior
invalid-version case. The installed SFO used `APP_VER=0.1.0` and
`VERSION=0.1.0`, so both source values are now `00.01` per the user's
instruction.

The corrected package was rebuilt and its extracted `PARAM.SFO` parsed
independently: `TITLE_ID=RPRX90002`, `APP_VER=00.01`, and `VERSION=00.01`.
Corrected PKG SHA-256:
`54d5d299c6831a18e7c0474aac3577a8a02de443a7a3b98e03f64d242b9010b9`.

| Check | Status | Evidence |
|---|---|---|
| Corrected SFO fields | PASS | Extracted SFO reports title ID `RPRX90002` and both version fields `00.01`. |
| Initial package install | PASS | Console title directory appeared; installed SPRX SHA-256 matched the signed build. |
| Initial launch | FAIL (user-reported) | User observed the same launch error previously associated with the bad version string; no harness log was retrieved. |
| Corrected app install/launch | PASS (app) | The corrected title starts and writes its verbose harness log. |
| Resident module load | FAIL | `sys_prx_load_module` returns `0x80010007` (`ENOEXEC`) on cycle one; the PRX never reaches `module_start`. |
| Network receive, audible output, Cobra/autoload | UNTESTED | No RAOP session, sound check, or boot-plugin change was performed. |

The earlier hardware-package section is a checkpoint for the first package and
its hashes. This correction supersedes its SFO and PKG hash; the PRX, SPRX, and
harness SELF are unchanged.

## Corrected hardware run and SPRX loader failure (2026-11-08 23:09 EDT)

After removing the first four-file diagnostic title, the corrected package was
installed fresh. Its console-side SFO parses as `TITLE_ID=RPRX90002`,
`APP_VER=00.01`, and `VERSION=00.01`. The console-side SPRX SHA-256 matches
the signed build exactly:
`9fae9ed955c023aca2077654625510e80766f899c9eaa4998cad4af5f1588817`.

The app itself starts and writes
`/dev_hdd0/tmp/ps3-airplay-resident-harness.log`. On its first configured
cycle, `sys_prx_load_module` returns `-2147418105`, which is `0x80010007` and
maps to `ENOEXEC` in the pinned PS3DK errno definitions. The harness logs
`HARNESS_FAIL value=1`, then its foreground test audio port stops and closes
with result zero and `cellAudioQuit` returns zero. There is no `LOAD_OK`,
resident startup event, receiver-ready event, worker, or PRX teardown event.
The 50-cycle run therefore stops at cycle one.

| Hardware result | Status | Evidence |
|---|---|---|
| SFO identity/version | PASS | Installed title ID `RPRX90002`, `APP_VER=00.01`, `VERSION=00.01`. |
| Embedded module identity | PASS | Installed module hash equals the freshly built signed SPRX hash. |
| Harness launch and logger | PASS | Harness log opened and records cycle count, baseline, audio setup, and load result. |
| Resident PRX loader acceptance | FAIL | Loader returns `0x80010007` (`ENOEXEC`) before module startup. |
| Harness audio-port cleanup | PASS | Port stop, close, and audio quit return zero after load failure. |
| RAOP, resident PCM, audible output, 50-cycle cleanup | UNTESTED | Module did not load. |

The exact runtime log is local and ignored under `build/resident-hardware/`;
it contains no stream payload. Do not substitute the RPCS3 `.fake.sprx` for
this production SPRX without a separate reviewed hardware test decision. The
physical test is BLOCKED on understanding why this loader rejects the signed
SPRX.

## Fresh hardware package build (2026-11-08 22:49 EDT)

The temporary Linux build environment was restored with the pinned PS3DK
v0.27.0 Linux PPU compiler, Linux host tools, official v0.27.0 runtime
artifacts, and the project's pinned Mbed TLS 3.6.7. The resident built as a
signed production SPRX, not the RPCS3 fake. `prx-gen check` reports one load
segment, 6,300 relocations, one management export, and five imports: `sys_net`,
`cellSysmodule`, `cellNetCtl`, `cellAudio`, and `sysPrxForUser`.

A new `RPRX90002` package embeds exactly one `.sprx`, byte-identical to the
fresh signed SPRX. The package and payload were independently extracted and
compared. The CMake title-ID check was adjusted after the real CMake version
rejected `{9}` quantifier syntax for a valid nine-character ID.

| Check | Status | Evidence |
|---|---|---|
| Host crypto/protocol/audio suite | PASS | RAOP crypto, RTSP/session, mDNS, encrypted RTP, ALAC, PCM/SPSC/resampling and sanitizer tests passed in Lima. |
| Signed PRX/SPRX structure | PASS | `prx-gen check`; signed container type `0x00010001`; expected imports listed above. |
| Harness build | PASS | PPU SELF built with 50-cycle configuration and unique title ID `RPRX90002`. |
| PKG contents | PASS | PKG type 1; one production SPRX extracted and matched byte-for-byte. |
| Hardware install and launch | UNTESTED at this checkpoint | Fresh package has not yet been installed or run on the development PS3. |
| Resident RAOP-to-audio / audible output | UNTESTED | The package build does not provide runtime or listening evidence. |
| Cobra/autoload and game coexistence | UNTESTED | No boot-plugin settings were changed. |

SHA-256:

| Artifact | SHA-256 |
|---|---|
| `ps3-airplay-receiver.prx` | `3b61d7739b371a4ebcb12b8723a0b11e7a29fbd96a248e650bd01a52570e7986` |
| `ps3-airplay-receiver.sprx` | `9fae9ed955c023aca2077654625510e80766f899c9eaa4998cad4af5f1588817` |
| `ps3-airplay-resident-harness.self` | `013fda9e760b02172c478c5ffd0d2d5496930f211c53f02eb00d01165dd8bb52` |
| `ps3-airplay-resident-harness.pkg` | `62403e6b11795c6fecf2d5b8a896d213dbf1df11905d771af43448a78c886f4f` |

Artifacts are in ignored `build/resident-hardware/`; no binaries, local paths,
private network details, or raw device logs are added to Git. Installation and
runtime findings will supersede the UNTESTED rows above when independently
collected.

## Structured logging follow-up (2026-11-08 20:53 EDT)

The resident diagnostics structure now carries a 128-record bounded journal.
Each record has an atomic commit sequence, system monotonic microseconds,
internal event name, result/value fields, and byte count. Existing resident
events are copied into the journal independently of the PRX file logger;
logger-open/write failures, audio submission, and PCM drain are recorded.
The RPCS3 harness drains records while polling lifecycle events and before
unload, and records its own events with elapsed milliseconds. It reports the
number of dropped resident records. Records intentionally omit packet data,
RAOP credentials, device/peer identifiers, and stream metadata.

Validation: `scripts/test-resident-diagnostics.sh` passes with ASan/UBSan,
including 128 concurrent records, per-slot commit ordering/integrity, and
overflow accounting. The harness compiles in host syntax-only mode against
mock API declarations; the four SELF-container tests pass; the harness build
script passes `bash -n`; and `git diff --check` is clean. The PS3DK PPU
compiler and resident Lima environment are unavailable in this session. The
new shared ABI has not been built or run in RPCS3, and no fresh package was
installed. The previous three-cycle result does not test this logger.

Status: **PASS** for the host journal test and source checks;
**BLOCKED** for fresh PPU build and RPCS3 log-delivery verification. Physical
hardware remains untouched and NOT READY.

## Pre-hardware test-gate implementation (2026-11-08 19:31 EDT)

The harness source now defaults to 50 audio-enabled load/start/stop/unload
cycles. It rotates four modes: no seeded resident PCM, stop immediately after
resident PCM submission, stop after an audio-worker drain event, and a longer
foreground/resident two-port submission. Resident test PCM fills the bounded
16,384-frame queue. The resident diagnostic tone is 440 Hz and the foreground
tone is 660 Hz, making a future audio capture able to distinguish the two
signals.

These harness changes have **not** been built into a package or executed in
RPCS3 in this pass. The current environment has no PS3DK/Mbed TLS Lima checkout
or PPU compiler, and the previously used SSH/Lima endpoints are unavailable.
The last verified RPCS3 result remains the earlier three-cycle package; do not
attribute that result to the expanded harness source.

Added a container validator and four host unit cases. They verify the PS3DK
SELF header types `0x00010001` for the production SPRX and `0x80000001` for the
RPCS3 fake SPRX, and explicitly report target-loader compatibility as not
established. The RPCS3 package script now rejects a package containing more
than one SPRX and byte-compares its sole embedded fake SPRX with fresh build
output. The validator passed against the retained local container files, but
the production `.sprx` was not rebuilt with this source revision. Its format
check does not validate CFW/Cobra key acceptance, and no hardware-ready
resident package has been assembled.

RAOP packets were not sent through the resident PRX. RPCS3 still reports
`EADDRNOTAVAIL`; no captured or independently heard output was obtained. The
three-cycle result does not establish 50-cycle stress, resource cleanup beyond
the module count, or real audio mixing. `sysThreadJoin` remains a blocking call
after the worker signals cleanup completion; its bounded behavior under a
stuck exit path is not established. **Hardware readiness remains NOT READY.**

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
| Expanded 50-cycle harness source | PASS (static/host) / UNTESTED (RPCS3) | Source rotates no-seed, immediate-stop, event-confirmed drain, and extended two-port submission modes and uses distinct generated tones. It has not been rebuilt or run. |
| Resident container type validation | PASS (format only) | Four host unit cases pass; retained artifacts classify as production SELF type `0x00010001` and RPCS3 fake type `0x80000001`. Target loader acceptance is not established. |
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

## Last built artifacts before expanded harness source changes

These hashes identify the previously tested three-cycle candidate, not the
current working source after the expanded harness and diagnostic-flag changes.

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
