# AIR PS3 roadmap

This roadmap is the current plan for turning AIR PS3 into a stable, public AirPlay 1 audio receiver, then adding optional Now Playing and display features. The stable receiver is a focused milestone; metadata, controller controls, and visual customization do not block that release.

For detailed acceptance criteria, hardware boundaries, and release checks, see [PUBLIC_RELEASE_PLAN.md](PUBLIC_RELEASE_PLAN.md). Historical engineering evidence remains in [TEST_MATRIX.md](TEST_MATRIX.md) and the dated reports under `docs/reports/`.

## Current state — v0.2.8 / SFO 00.48

- Identity: repository `MKWorldWide/ps3-airplay`; application `AIRPLAY02`; XMB title **AIR PS3**; AirPlay receiver label **AirPS3**.
- The v0.2.8 candidate passed the pinned Lima host, PPU, ELF, SFO, artwork, and debug-PKG validation. It was installed on the development PS3; FTP download-back and installed SFO/EBOOT matched the validated package payload.
- A user-supplied summary of the development-unit log reports 10m41s of active-stream telemetry: 69,632 RTP packets, 17 missing-packet observations all recovered, no logged queue drops/concealment/skips/PCM underflows/ALAC failures, and queue/PCM occupancy within expected bounds. This is telemetry evidence, not subjective proof of uninterrupted audible quality.
- The same summary reports successful RTSP OPTIONS responses, one stream setup, AirPlay volume updates, one FLUSH, and repeated rejected timing-packet diagnostics. Timing-packet behavior remains open for investigation.
- No XMB menu/resume events appear in the summarized capture. The v0.2.8 display, menu composition/resume, clean exit, repeatability, and sustained 30-minute playback criteria are not yet established.
- The GitHub repository is public, `main` is the default branch, and there are currently no GitHub Releases (read-only check, 2026-11-08). Do not change repository visibility or publish a release as part of roadmap work.

## Release path

| Gate / phase | Goal | Current state |
|---|---|---|
| Gate 0 — v0.2.8 candidate acceptance | Verify artifact identity; foreground image; PS-menu open/close/resume; START exit; real iPhone discovery/session; audio and reconnect; fresh logs; safe rollback | Build and installation verified. Short telemetry run reported. Display/resume, user-observed audio quality, repeat sessions, clean exit, and 30-minute run remain open. |
| Phase 1 — Stable receiver | Bound or fix long stalls/full-queue recovery; keep diagnostics off the audio/control hot path; pass repeatable hardware playback/reconnect/exit criteria; document support and recovery | Release blocked on Gate 0 and the stable-release checklist. Keep candidate/beta language until evidence passes. Target stable receiver milestone: 0.3.0, subject to final version review. |
| Phase 2 — Now Playing | Add bounded title/artist/album/artwork handling based on verified sender behavior | Planned after the stable receiver milestone. |
| Phase 3 — DualShock controls | Add local volume/mute and only sender-supported transport controls | Planned after protocol capability and focus/button behavior are tested. Keep separate from AirPlay `SET_PARAMETER` volume handling. |
| Phase 4 — Display settings | Add safe dimming, backgrounds, fonts, layout, artwork size, transitions, and ambient mode | Planned after display and XMB resume are stable. Never draw over or assume ownership of VSH. |
| Phase 5 — Release publication | Complete exact-source/package/privacy/license checks, public installation guidance, checksums and release notes | Repository is already public; no downloadable GitHub Release exists yet. Publication remains a separate, evidence-gated action. |

## Priority order

1. Finish Gate 0 with a single controlled development-unit run and exact version/hash identification.
2. Establish a repeatable 30-minute receiver session, then extend the soak only after shorter runs pass; retain separate results for the development and primary test units.
3. Use fresh logs to classify stalls, RTP gaps/recovery, timing packets, PCM occupancy/underflows, logger queue pressure, and shutdown. Do not infer audible quality from counters alone.
4. Resolve remaining stable-receiver and public-distribution blockers; then prepare the focused 0.3.0 receiver release.
5. Start metadata, DualShock, and display-setting phases independently after the stable receiver gate.

## Scope boundaries

- The product is an iPhone-sent AirPlay 1 / RAOP receiver; Spotify authentication/playback stays on the phone.
- A successful host test, package validation, or healthy log counter is not proof of visible display output, audible quality, or stable hardware behavior.
- Resident SPRX loading and in-game audio mixing are a separate feasibility effort. This roadmap does not promise universal game audio coexistence.
- Raw logs, local build outputs, device identifiers, network data, and local machine details stay out of public files. Use aggregate, provenance-labelled evidence only.
