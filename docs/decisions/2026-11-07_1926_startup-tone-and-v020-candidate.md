# Remove startup tone and label the next package 0.2.0

## Timestamp

2026-11-07 19:26 America/New_York

## Scope of the pass

Remove the automatic diagnostic tone from AIRPLAY02 startup, retain the test-tone utility for deliberate host diagnostics, add an objective PCM ring resampling baseline, and advance default app/package version to 0.2.0 with SFO APP_VER 00.40.

## Why this pass was needed

The startup sound is no longer useful to the maintainer. Recent hardware evidence already confirms streamed PCM reaches GameOS; the next useful work is better signal-chain measurement. The user explicitly requested package label 0.2.0 and continuation of the SFO sequence.

## Files changed

- `prototype/standalone-receiver/source/main.c`
- `prototype/standalone-receiver/Makefile`
- `prototype/standalone-receiver/README.md`
- `scripts/build-lima-crypto.sh`
- `scripts/build-lima-standalone-receiver.sh`
- `scripts/package-lima-standalone-receiver.sh`
- `tests/audio/pcm_quality_test.c`
- `docs/VERSIONING.md`
- `docs/ROADMAP.md`
- `docs/TEST_MATRIX.md`
- `docs/memory.md`
- `docs/lessons-learned.md`
- `docs/scratchpad.md`
- `docs/reports/2026-11-07_v0200_candidate.md`
- this decision log

## Architecture / logic changes

The production audio-block renderer now submits silence until receiver PCM is available, then renders PCM at the explicit 48 kHz GameOS output rate. Tone state, startup/completion logging, and tone generation were removed from the app and its link list. The reusable tone utility and its host test remain.

The new host quality test invokes the production PCM ring at 44.1 kHz input and 48 kHz output. It reports six sine-wave frequency measurements against an aligned ideal reference.

## Runtime behavior changes

AIRPLAY02 no longer emits a tone on launch. The requested app/package label is 0.2.0 and SFO APP_VER is 00.40. No console package was transferred or installed.

## Validation performed

- Compiled and ran the PCM quality test with `-Wall -Wextra -Werror -fsanitize=address,undefined` on the Mac and in Lima.
- Each synthetic case rendered 48,000 output frames with zero underflows and no dropped input frames.
- Recorded SNR values: 92.30, 54.64, 26.78, 15.03, 8.47, and 4.17 dB at 100 Hz, 1 kHz, 5 kHz, 10 kHz, 15 kHz, and 20 kHz respectively.
- Complete Lima suite rerun passed: protocol/crypto/session/SDP/mDNS, RTSP/RTP loopback including slow-reader deadline, encrypted RTP→decrypt→ALAC→PCM, ALAC sanitizer mutations, logger, FFmpeg fixture decode, PCM ring quality and gain tests, diagnostic-tone unit test, and PPU component object compilation/architecture checks.
- Source inspection confirms no tone references remain in the app renderer or standalone link list; the production standalone package build is pending.

## Immediate results

Tone-free startup path is implemented. The measurement makes high-frequency interpolation attenuation visible; it does not prove that this is the cause of crackle/dropouts on hardware.

## Known limitations / follow-ups

- Continue with measured resampler comparisons and the missing controller/jitter/PCM telemetry.
- Assess a bounded higher-quality resampler with fidelity and CPU measurements before changing playback code.
- Add RTP arrival-jitter and PCM starvation/occupancy metrics; add controller button transition logging.
- Test package launch/exit and streamed audio on hardware only after a future explicit deployment request.

## Risks introduced

The diagnostic tone can no longer verify GameOS audio output on app startup. It was intentionally removed by request; host-only tone tests remain available. Changing package label to 0.2.0 may imply the stability milestone if viewed without this candidate note, so release/docs must preserve the pending acceptance status.

## Decision record

### Decision: Keep the tone utility but remove it from normal app startup

- **Context:** User no longer needs an automatic tone, while the tone generator remains useful in isolated tests.
- **Options considered:** Delete the utility entirely; keep it wired into the app; retain only the isolated component/test.
- **Why chosen:** This removes user-visible startup audio without deleting useful test code or changing the stream path.
- **Tradeoffs accepted:** Hardware audio initialization no longer receives a built-in audible startup check.

### Decision: Use app label 0.2.0 and SFO 00.40 while documenting candidate status

- **Context:** User explicitly requested 0.2.0; SFO sequencing should continue from 00.39.
- **Options considered:** Keep 0.1.x until stable acceptance; use 0.2.0 as a label and claim stable; use 0.2.0 as a candidate and keep acceptance pending.
- **Why chosen:** It follows the direct version instruction while maintaining accurate milestone claims.
- **Tradeoffs accepted:** Candidate label is ahead of the versioning document's usual stable milestone convention.

### Decision: Add measurement before altering the resampler

- **Context:** Crackle is reported and resampling is in the output path, but no component-level fidelity baseline existed.
- **Options considered:** Immediately replace linear interpolation; measure production behavior first.
- **Why chosen:** The test exposed frequency-dependent attenuation without introducing an unmeasured PS3 CPU/load risk.
- **Tradeoffs accepted:** No audible improvement is expected from this measurement-only change.

## Next recommended step

Continue improving measurable audio continuity and runtime diagnostics. Keep hardware unmodified until a separate explicit deployment request.
