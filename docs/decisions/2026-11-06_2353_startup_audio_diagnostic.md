# Add a startup audio diagnostic

**Timestamp:** 2026-11-06 23:53 EDT

## Scope of the pass

Added a bounded five-second, 440 Hz stereo diagnostic to the standalone app, a host unit test for its waveform and duration, build integration, and public-facing documentation of its status.

## Why this pass was needed

The requested first hardware workflow depends on a short audio-path check, but the existing standalone receiver had no built-in tone. The prior package is structurally valid but cannot provide this test.

## Files changed

- `src/audio/diagnostic_tone.c`
- `src/audio/diagnostic_tone.h`
- `tests/audio/diagnostic_tone_test.c`
- `tests/protocol/raop_rtp_test.c`
- `prototype/standalone-receiver/source/main.c`
- `prototype/standalone-receiver/Makefile`
- `prototype/standalone-receiver/README.md`
- `scripts/build-lima-crypto.sh`
- `scripts/package-lima-standalone-receiver.sh`
- `docs/TEST_MATRIX.md`
- `docs/ROADMAP.md`
- `docs/memory.md`
- `docs/scratchpad.md`
- `docs/lessons-learned.md`
- `docs/decisions/2026-11-06_2353_startup_audio_diagnostic.md`
- `docs/decisions/README.md`

## Architecture / logic changes

A new audio helper emits exactly 240,000 stereo float frames at 48 kHz, using a 440 Hz sine wave with a 50 ms linear fade at each edge and a five-percent full-scale peak. The standalone audio event loop submits those blocks through the existing GameOS output port. After the tone completes, the same block renderer returns to the PCM ring for AirPlay audio. Start and completion events are flushed through the existing capped log.

The tone is automatic at application startup so the initial hardware path can be checked without another control interface. The receiver remains standalone; no resident PRX or boot plugin changes are involved.

## Runtime behavior changes

Launching the standalone receiver now produces a quiet five-second startup tone before normal PCM output. The app then continues its receiver loop. The log records tone start and generated-frame completion; neither proves that a listener heard sound.

## Validation performed

- The tone waveform test, full crypto/session/SDP/receiver/ALAC/PCM host test script, standalone PPU build, and PKG extraction/metadata validation pass.
- Additional standalone mDNS, RTP queue, RTSP session, RTSP parser/logging, and RTSP listener tests pass. The first RTP overflow assertion exposed an old fixture that retried a sequence already in the queue; the fixture now uses a distinct sequence.
- The first ad hoc mDNS test compile omitted the feature macro that exposes Linux multicast/select interfaces; the rerun uses `_DEFAULT_SOURCE`. This was a test-command setup issue, not a product source change.
- The resulting standalone executable is a 64-bit PowerPC PPU ELF. Package extraction confirms `PARAM.SFO` with title `PS3 AirPlay`, title ID `AIRPLAY02`, category `HG`, and an NPDRM SELF `EBOOT.BIN`; package SHA-256: `1b797033a25bbf95f7deb8ec9e95f5d12c66edcf2fd00b2f6ceb4a9e990704c2`.
- Read-only FTP checks found no responsive console FTP endpoint; hardware installation and playback have not been attempted.

## Immediate results

- The rebuilt package has title `PS3 AirPlay`, title ID `AIRPLAY02`, category `HG` (Game), and a structurally validated EBOOT. It is the package prepared for possible transfer.
- `PS3_SYSTEM_VER` is `01.8000`, below the target's reported Evilnat 4.93 firmware; this is metadata compatibility evidence only, not a successful install or runtime check.
- The existing receiver log path is `/dev_hdd0/tmp/ps3-airplay.log`; each log event calls `fflush` and the file is capped at 16 KiB.
- No console connection, installation, launch, audible tone, iPhone discovery, or Spotify playback is verified.

## Known limitations / follow-ups

- Find the powered-on console and verify its FTP service, free space, title conflict, and log path.
- Before transfer, provide package name, digest, destination, and conflicts and obtain approval.
- The RAOP route remains diagnostic; sender interoperability and audibility remain hardware gates.

## Risks introduced

- The automatic tone may surprise the user at launch, though its peak is limited and both edges fade over 50 ms.
- Audio output block scheduling and HDMI audibility remain unverified on the console.
- A client connecting during the five-second startup tone could queue RTP before normal PCM rendering resumes.

## Decision record

### Decision: Render a fixed startup tone through the GameOS audio block path

- **Context:** The hardware workflow needs to isolate console audio output before testing iPhone discovery.
- **Options considered:** Keep a separate tone-only package; add a manual controller control; play one automatic diagnostic in the receiver app.
- **Why this option was chosen:** It exercises the complete app's existing output port and requires no new input controls or second installable artifact.
- **Tradeoffs accepted:** The five-second sound is automatic at each launch, and simultaneous RAOP input during that interval is not a validated scenario.

## Next recommended step

Resume read-only console discovery. Once FTP, free space, title conflict, and destination are verified, present the package name and digest for approval before any transfer.
