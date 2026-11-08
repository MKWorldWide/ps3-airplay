# Public stable release and feature plan

**Status checked:** 2026-11-08
**Current candidate:** AIRPLAY02 v0.2.8 / SFO `00.48`, XMB title `AIR PS3`, RAOP name `AirPS3`
**Repository:** `MKWorldWide/ps3-airplay` is public; `main` is the default branch. No GitHub Releases are listed. These GitHub facts were checked read-only; this plan does not change them.

This plan keeps the first stable release focused on receiving AirPlay audio. Now Playing, controller features, and visual settings can follow as separate releases.

## Evidence and current release blockers

### v0.2.8 package and build

The exact-source Lima gate passed for source `9f7af0027b7c3e9470a1583bbfbf676f9525f72d`. The CellOS PPC64 ELF, SFO, debug PKG, extracted assets, and package envelope were validated. The package is AIRPLAY02 v0.2.8 / SFO `00.48`; its SHA-256 is `e70d957de4a0fe2b0f8a7bd3a87f63cabaab2f12726e6341508a5f9b8c5de383`. Its exact build metadata is in [VERSIONING.md](VERSIONING.md) and the [v0.2.8 engineering report](reports/air_ps3_v028_reliability_branding.md).

The package was installed on the development PS3. The FTP upload/download-back was byte-identical to the validated package; installed SFO and EBOOT matched the extracted installation payload. This proves transfer and installation identity, not launch or playback quality.

### Development-unit stream telemetry

The following figures come from the user's supplied summary of a read-only log capture. The raw log is not included here or in the public repository:

| Observation | Reported result | Interpretation boundary |
|---|---:|---|
| Active stream window | 10m41s, 07:05:05Z–07:15:46Z | Shorter than the required 30-minute acceptance run. |
| RTP | 69,632 packets; 17 missing-packet observations, all recovered | No reported packet loss remained unrecovered in this capture. |
| RTP queue | 79–102 of 256 packets | Occupancy range only; not a latency or audible-quality measurement. |
| PCM ring | 12,056–12,635 of 16,384 frames | No underflows were reported in the summary; ring occupancy alone does not prove sound reached the TV/speakers. |
| Audio service/render | Maximum service interval 15.33 ms; maximum render 1.754 ms | Logged maxima; physical audio quality still requires human listening. |
| RTSP and controls | 330 OPTIONS requests returned 200; one stream setup; 18 AirPlay volume updates; one FLUSH | Does not test DualShock volume/mute or transport buttons. |
| Timing packets | Repeated timing-packet rejects and one accepted timing reply | Investigate against sender behavior; no audio loss was reported alongside these events. |
| XMB/display | No menu-open, menu-close, or resume events in capture | Display appearance and resume were not tested by this log. |

This summary establishes active receiver telemetry. It does **not** independently establish that the entire interval was audibly uninterrupted, that the display was stable, or that XMB resume worked.

### Earlier v0.2.7 evidence, kept separate by test unit

- **Development unit:** a 30-minute v0.2.7 run reported 26,814 PCM underflow frames, maximum audio-service interval 35.842 ms, and RTP queue average near 136/256 with a peak of 177.
- **Primary test unit:** a later v0.2.7 run after AirPlay reconnection lasted 17m05s and reported no queue drops, concealment, underflows, or ALAC failures, with a 16.79 ms maximum service interval.
- **Primary test unit earlier failure:** one run showed an approximately 5.44-second RTSP/audio-service stall, a full RTP queue, drops, concealment, and PCM underflows. A new AirPlay session restored normal operation. The cause was not proven; recovery after a new session does not prove automatic recovery within a stalled session.

Do not combine these separate units or runs into one performance claim. Preserve each raw log privately and associate future summaries with unit role, package version/hash, source revision, test interval, and acquisition provenance.

### Blocking items before a stable receiver release

1. **Display and VSH lifecycle:** no v0.2.8 evidence yet for stable foreground pixels, exact XMB title/art presentation, PS-menu open/close, repeated resume, or expected restoration. The application does not own VSH composition.
2. **Human-confirmed playback quality:** fresh telemetry has no reported decode/underflow failures, but it does not replace listening confirmation through the intended output path.
3. **Duration and repeatability:** the supplied v0.2.8 run is 10m41s, below the 30-minute target. Repeat launches, reconnects, and clean exit need evidence.
4. **Stall and recovery behavior:** exercise RTSP stalls, full RTP backlog, logger queue pressure, writer failure, and session reconnect with bounded fault-injection and then hardware evidence. Do not assume the logger or resync change is proven from host tests alone.
5. **Timing packet compatibility:** identify why timing packets are rejected, determine whether the sender expects a supported response, and correlate any changes with playback. Do not accept or parse arbitrary timing traffic just to silence the log.
6. **Public distribution readiness:** inventory source and asset licenses, third-party notices, build tooling, CFW/PKG requirements, install/removal instructions, and recovery procedure. The code has an MIT license; the artwork's reuse terms and all build/distribution dependencies still need explicit review.

## Gate 0 — Complete v0.2.8 candidate acceptance

Keep this as one controlled, measured development-unit validation before a longer soak.

1. **Record exact identity:** source commit, PKG SHA-256, SFO APP_VER/VERSION, title ID, XMB title, and installed EBOOT/SFO comparison. Retain a known-good rollback package for the same unit.
2. **Foreground display:** user confirms the artwork and title are visible, stable, and free of unintended overlays.
3. **XMB lifecycle:** while idle, press PS to open/close the system menu, resume, repeat the cycle at least twice, then press START and confirm clean return to XMB. Record screen brightness/flicker/loading-indicator observations separately from log callbacks.
4. **Receiver route:** with an iPhone on a compatible local network, record whether it discovers AirPS3, completes RTSP setup/RECORD, streams, receives volume changes, pauses/resumes, flushes, disconnects, and reconnects. Record sender/app capability; test Spotify where practical. Discovery alone is not a connection.
5. **Short playback and listening:** have the user confirm actual audio through the intended HDMI/TV/speaker path and note crackle, gaps, volume behavior, and XMB interaction. The diagnostic and pipeline counters are supporting evidence only.
6. **Duration:** after clean launch/exit and short playback pass, run at least 30 minutes, then perform repeat sessions. Extend to a longer soak only if shorter runs show no hang, display failure, or worsening audio.
7. **Log review:** retain the original private log; publish no raw log. Analyze RTSP stage times, service intervals, RTP gaps/recovery/drop/resync, queue occupancy, conceal/skips, PCM occupancy/underflows, ALAC failures, logger depth/drops/write maxima, volume/FLUSH, timing packets, disconnect, and cleanup. Keep development and primary test-unit evidence in separate folders and reports.
8. **Recovery:** document expected START/webMAN exit path, package rollback, and what to do if the app hangs or fails to show. Do not alter firmware, boot plugins, or system video settings for this gate.

**Human observation required:** visible display and XMB behavior, controller operation, iPhone sender actions, and audible quality.
**Hardware action boundary:** this planning pass performs none. Any new installation, replacement, restart, or settings change must follow the user's current authorization and an established recovery route.

Gate 0 passes only when package identity is exact, display/lifecycle behavior is repeatable, streamed audio is human-confirmed, the 30-minute run completes, reconnect and exit behave, and logs reveal no unresolved reliability issue. A clean counter sample is necessary but not sufficient.

## Phase 1 — Stable public receiver release

Keep this release focused on reliable foreground AirPlay audio.

### Acceptance

- Gate 0 passes on the exact release candidate; repeat install/launch and session results on the intended supported setup.
- The known multi-second stall/full-queue condition is reproduced or otherwise bounded with fault-injection; recovery and drop behavior are deterministic, bounded, and visible in logs.
- Audio/control servicing remains responsive when the log queue is full, the writer is slow, or the sink fails. Queue loss and shutdown-tail limits are documented.
- AirPlay discovery, RTSP setup/record, RTP receive, decrypt, ALAC, PCM output, pause/resume, FLUSH, AirPlay volume, disconnect/reconnect, controller exit, and XMB return are tested end to end where supported.
- At least 30 minutes of user-confirmed continuous playback and repeat sessions complete without hang or forced shutdown; longer soak follows the progressive stability plan.
- Display, controller, audio, and log outcomes are recorded separately. No host-only result is labeled hardware pass.

### Deliverables

- Exact source revision and package hashes, version-aligned SFO/title/content identity, reproducible build/test instructions, and verified included assets.
- Supported PS3/CFW requirements and clear limitations, including that the debug PKG is not promised to install on retail firmware.
- XMB installation, normal exit, removal, rollback, and safe recovery instructions.
- Concise release notes and known-issues list; public raw logs and machine/device/network identifiers excluded.
- License/notice review for project code, artwork, PS3DK/PSL1GHT, crypto/codec libraries, and any redistributed build tools. Confirm the exact debug PKG format and all included files are suitable for public distribution.
- Staged beta/RC label until all acceptance criteria pass. Target stable receiver milestone is 0.3.0, pending version review after evidence is complete.

## Phase 2 — Now Playing metadata and artwork

- First capture actual metadata behavior from each target sender; distinguish Spotify/iPhone behavior from protocol capability in general.
- Support bounded title, artist, album, and artwork parsing with UTF-8/text sanitization, missing-field fallbacks, maximum image dimensions/bytes, decode limits, and cache eviction.
- Handle stream changes and missing/corrupt artwork without blocking RTSP, RTP, or PCM service. Keep metadata work asynchronous and memory bounded.
- Add fixtures for supported metadata formats and sender-specific tests. Demonstrate audio remains stable under frequent metadata/artwork changes.
- Acceptance: correct displayed metadata/art for tested senders, graceful fallback for unsupported senders, no audio stalls or unbounded memory growth.

## Phase 3 — DualShock playback controls

- Keep local DualShock volume/mute distinct from the existing AirPlay `SET_PARAMETER` gain path; define gain bounds, mute semantics, ramping, and visible feedback.
- Investigate play/pause/previous/next only where sender controls are available and honored. Expose sender capability or document when a control is unavailable; do not promise Spotify behavior without testing it.
- Define control mapping, focus/navigation, button-conflict policy, idle/streaming behavior, and a way to reach exit/settings.
- Acceptance: controls work on the physical controller in repeated idle and streaming sessions, with no regressions to PS-menu, resume, START exit, or audio servicing.

## Phase 4 — Display settings and ambient mode

- Provide selectable full-screen dimming: off, 50%, and 95%; allow adjustment toward 90% if 95% proves too dark on hardware. Apply dimming consistently to artwork, text, controls, and background.
- Offer adaptive album-derived color, solid backgrounds, and user-provided wallpapers; include invalid-image fallback and strict decoded-image/memory bounds.
- Offer three font choices, seven layout positions, and small/medium/large artwork sizing.
- Offer transitions off/fast/smooth and a configurable idle timeout for ambient Now Playing view. Visual effects must not interrupt audio.
- Persist validated settings with safe defaults and recovery from corrupt or unsupported data. Controller navigation must remain operable.
- Respect GameOS/VSH ownership: never draw over system UI or assume the app can choose XMB composition. Test PS-menu/resume after each renderer change.
- Acceptance: all choices persist, invalid input falls back safely, and repeated XMB cycles restore a visible responsive screen without audio interruption.

## Phase 5 — Package and GitHub Release checklist

Before publishing a downloadable release from the already-public repository:

1. Build from a clean exact commit in the pinned Lima toolchain; run host sanitizer/regression suite and PPU/ELF/SELF/PKG/SFO checks.
2. Extract the exact package and verify title ID, version, content ID, EBOOT, icon, background, and artwork; publish PKG and ELF hashes.
3. Check code/package version alignment and create release notes describing tested hardware/software scope and known limitations.
4. Include install, exit, uninstall, rollback, and recovery instructions plus the tested CFW/PKG format requirements.
5. Audit all release files/assets for secrets, local paths, personal/device/network identifiers, raw logs, and unlicensed content. Inspect the full diff and exact release assets; preserve public-safe architecture, generic firmware, and demonstration details where useful.
6. Review all reachable branches, tags, history, LFS/submodules, issues/comments, pull requests, discussions, wiki, workflows, packages, and linked release surfaces for private information. A clean current tree does not sanitize old history or other GitHub surfaces.
7. Verify the remote/repository/default branch and release target. Keep existing repository name, public visibility, and `main` identity unchanged. Release publication is a separate action from roadmap planning.

## Exclusions

AirPlay video/mirroring, Spotify credentials or Spotify Connect, resident SPRX installation, firmware changes, boot-plugin changes, LV2 modifications, and universal in-game audio mixing are outside this receiver roadmap. Game coexistence requires a separate feasibility decision after standalone playback is stable.
