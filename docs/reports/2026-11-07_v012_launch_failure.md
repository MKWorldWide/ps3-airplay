# v0.1.2 XMB launch failure

**Test timestamp:** 2026-11-07 03:17 EST (local evidence collection time)
**Source revision:** `a6b36cd43fe289b63549cbd06b888f984fe87936` (`main`; documentation review branch at `f7e1169ad015698c4067fdd7a69d238b4ac506fe`)
**Package:** AIRPLAY02 v0.1.2, 3,225,712 bytes, SHA-256 `923aebf6eca5693b21b494a9a4225c36629f13e8b316b2825d5deb3763226ebf`
**Console:** PS3, Evilnat 4.93 CFW; Cobra 8.5 observed

## Result

The user installed v0.1.2 through XMB and reported that launching the title returned error `80028F21`.

Read-only FTP checks confirmed:

- Installed `PARAM.SFO` reports `APP_VER=0.1.2`, `TITLE_ID=AIRPLAY02`, and `TITLE=PS3 AirPlay`.
- Installed `PARAM.SFO` SHA-256 is `999db390c73d46dec43c4884fe5ba34b0307ec22c5cf74e0c058f217b2b08be8`, matching the extracted v0.1.2 package file.
- Installed `USRDIR/EBOOT.BIN` is 445,232 bytes with SHA-256 `98601db89e5a23960aa561dedd8ddff1ed68db25b05aa222c1139bfcebda6ac8`, matching the extracted v0.1.2 package file.
- The PKG lists the expected `UP0001-AIRPLAY02_00-0000000000000000` content ID, a 445,232-byte NPDRM SELF, the SFO, icon, and background asset.
- The source ELF is a statically linked 64-bit big-endian PowerPC executable. This is build evidence, not proof of successful PS3 loading.

The console log remains 1,817 bytes with its last event at `2026-11-07T05:43:51Z`. Its contents are the earlier unidentified run ending in `APP_STOP`; it was not updated by this attempt and contains no version marker. It cannot establish v0.1.2 startup behavior.

## Test matrix

| Test | Result | Evidence |
|---|---|---|
| Package installation and identity | PASS | Installed SFO identifies AIRPLAY02 v0.1.2; SFO and EBOOT hashes match the extracted candidate |
| XMB launch | FAIL | User reported `80028F21` |
| Visible display | BLOCKED | App did not launch to a user-visible screen |
| Diagnostic tone | BLOCKED for v0.1.2 | Prior tone observation cannot be tied to this build |
| Controller START / clean XMB exit | BLOCKED | App did not launch; prior clean exit remains unproven |
| iPhone discovery / RTSP / RTP / ALAC / Spotify playback | NOT RUN for v0.1.2 | No receiver session was attempted after the launch failure |

## Interpretation and next action

The error occurs at XMB launch according to the user report. The installed files matching the candidate rules out a truncated or different installed EBOOT/SFO, but does not prove that the PS3 loader accepts the SELF or that application startup succeeds. The unchanged log only shows that this attempt produced no new application log; it does not distinguish loader rejection from failure before the log opens.

The [PS3 Developer Wiki error table](https://www.psdevwiki.com/ps3/Error_Codes) does not document `80028F21`. A [2017 forum thread](https://www.psx-place.com/threads/4-82-nobd-cfw-options-rebug-ferrox-overflow-unable-to-boot-apps-games-from-ps3-xmb.15739/page-3) contains an unrelated question reporting that code but no diagnosis. The code's cause is therefore unknown; do not change CFW, system settings, or boot configuration on this evidence alone.

Next, inspect the NPDRM SELF metadata and the SDK's application entry/startup contract against the installed artifact. If these checks do not identify the fault, prepare a minimal, reversible launch/display control and establish its recovery path before another hardware launch. Do not stage or install another PKG until the likely cause is isolated and the new artifact is validated.

No new package was transferred, no system setting or boot plugin was changed, and no SPRX was loaded. Raw logs and network identifiers remain local and are not included in this report.
