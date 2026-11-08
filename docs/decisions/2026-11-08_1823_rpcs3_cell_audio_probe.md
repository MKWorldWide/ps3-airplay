# RPCS3 cellAudio capability probe

## Timestamp in EDT

2026-11-08 18:23 EDT

## Scope of the pass

Added a controlled stereo `cellAudio` tone probe to the separate `RPRX90001`
test application. It initializes the audio library, opens and starts a
two-channel port, submits a five-second 440 Hz tone in bounded 256-sample
blocks, then stops, closes, and quits the audio library. The resident PRX
source was not changed in this pass.

## Why this pass was needed

The resident currently discards decoded PCM. Before adding a resident output
backend and a second playback thread, the available PS3 audio API needed an
emulator capability check.

## Files changed

- `experimental/resident/harness/CMakeLists.txt`
- `experimental/resident/harness/source/main.c`
- `experimental/resident/README.md`
- `docs/experimental/RESIDENT_OVERNIGHT_REPORT.md`
- `docs/memory.md`
- `docs/scratchpad.md`
- `docs/decisions/2026-11-08_1823_rpcs3_cell_audio_probe.md`

## Architecture / logic changes

The harness links PS3DK's `audio` stub and `libm`, then submits a low-level
test signal through `cellAudioAdd2chData`. The probe always attempts stop,
close, and quit after a successful initialization, including after later
errors. It runs before the unchanged three-cycle resident lifecycle test.

## Runtime behavior changes

Running the diagnostic app submits a low-volume, five-second stereo test tone
through the app's audio port before loading the PRX. This is a test-only
behavior. It does not alter the resident module's PCM-discard behavior.

## Validation performed

- Full Lima host protocol/crypto/audio regression and PPU/package validation
  passed.
- The `RPRX90001` package was installed and run in RPCS3 0.0.43-20570.
- `AUDIO_INIT_RESULT`, `AUDIO_PORT_OPEN_RESULT`,
  `AUDIO_PORT_START_RESULT`, `AUDIO_PORT_STOP_RESULT`,
  `AUDIO_PORT_CLOSE_RESULT`, and `AUDIO_QUIT_RESULT` each returned zero.
- `AUDIO_BLOCKS_SUBMITTED` reported 940 blocks, representing five seconds at
  48 kHz with 256 frames per block.
- The same run completed all three PRX load/start/ready/stop/worker-exit/
  unload cycles, returning module count to six after every cycle.
- The tone was submitted through RPCS3's audio backend, but it was not
  independently captured or measured acoustically.

## Immediate results

RPCS3 accepted and completed the app-level `cellAudio` lifecycle and all tone
submissions. This provides evidence that the tested API path is available in
the emulator's foreground app. It does not establish that a resident PRX can
own or share a port, that the user heard sound, or that game audio can mix
with it.

## Known limitations / follow-ups

The resident module still has no cellAudio backend, jitter-buffered playback,
bounded PCM queue, or synthetic RAOP-to-audio integration. A five-second host
app tone does not validate long-running timing, decoder output handoff, or
background coexistence. Hardware readiness remains NOT READY.

## Risks introduced

The diagnostic app intentionally emits sound when run. RPCS3 may route that
test tone to the host audio device at a low level. The port is always stopped
and closed before resident lifecycle cycles begin.

## Decision record (embedded)

### Decision: Probe app-level audio before resident integration

- **Context:** The resident currently discards PCM and RPCS3 can execute its
  lifecycle, but PRX audio API availability was unknown.
- **Options considered:** Add `cellAudio` to the resident immediately; treat
  host PCM tests as sufficient; exercise a controlled app-level port first.
- **Why chosen:** The API can be exercised with a known signal and explicit
  cleanup before introducing shared threading, buffering, and port ownership
  to the resident.
- **Tradeoffs accepted:** The probe is only a feasibility check; it does not
  establish resident module behavior or physical console mixing.

## Next recommended step

Implement a single-producer/single-consumer bounded PCM queue and a separately
timed resident playback worker around `cellAudioAdd2chData`. Keep audio-port
startup and teardown retry-safe, prebuffer before playback, and validate
synthetic RAOP-to-PCM output in RPCS3 before attempting a foreground coexistence
experiment.
