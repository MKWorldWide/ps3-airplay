# First PS3 hardware test — package staging checkpoint

**Date:** 2026-11-07 (America/New_York)

## Result summary

| Check | Status | Evidence |
|---|---|---|
| Build commit | PASS | `4037671a135a74998801205b66024e6b93a0c3db` |
| Standalone PPU build | PASS | 64-bit PowerPC ELF built in the existing Lima guest |
| Package structure and metadata | PASS | PKG listing and extraction verified; title `PS3 AirPlay`, title ID `AIRPLAY02`, category `HG` (Game), minimum system version `01.8000` |
| Console FTP service | PASS | webMAN FTP responded to read-only listing and transfer |
| Pre-transfer storage/conflict check | PASS | 9.2 GB free; `/dev_hdd0/packages/` was empty; no `/dev_hdd0/game/AIRPLAY02` installation found |
| PKG transfer | PASS | Remote file size matched at 2,560,368 bytes; downloaded-back file compared byte-for-byte and SHA-256 matched |
| Application installation | NOT TESTED | User-operated XMB install is pending |
| XMB display/name/icon | NOT TESTED | Package metadata says `PS3 AirPlay`; no project icon was present in the package, so icon appearance is unknown |
| Application launch/clean exit | NOT TESTED | App has not been launched |
| Five-second tone | NOT TESTED | No audibility claim; user confirmation is required |
| Diagnostic log | NOT TESTED | `/dev_hdd0/tmp/` is available; no pre-existing `ps3-airplay.log` was present |
| iPhone discovery / RTSP / RTP / ALAC | NOT TESTED | No sender connection attempted |
| Audible AirPlay or Spotify playback | NOT TESTED | No playback attempted |
| Crash or instability | NONE OBSERVED | The application was not installed or run |

## Package details

- Filename: `ps3-airplay-receiver.pkg`
- SHA-256: `1b797033a25bbf95f7deb8ec9e95f5d12c66edcf2fd00b2f6ceb4a9e990704c2`
- Staging path: `/dev_hdd0/packages/ps3-airplay-receiver.pkg`
- Remote size: 2,560,368 bytes
- Title ID: `AIRPLAY02`

The staged PKG was downloaded back from the console and compared against the local artifact. The transfer is byte-identical. The FTP server did not provide a remote SHA-256 command; the digest match comes from hashing the downloaded-back file locally.

## Target and safety notes

Existing project records identify the demonstration target as Evilnat 4.93 CFW, with Cobra 8.5 previously observed; the console model was not recorded. The `PS3_SYSTEM_VER` minimum is `01.8000`, which is below that reported firmware, but install and runtime compatibility remain unverified.

Only the standalone PKG was uploaded. No SPRX was transferred, no plugin was installed or loaded, and no firmware, webMAN, audio, or boot plugin settings were changed. The package is not configured to start at boot.

## Install and remove

From XMB, open **Game → Package Manager → Install Package Files → Standard**, choose `PS3 AirPlay`, and install. Verify that it appears under Game, launches, remains responsive, and exits cleanly. The first launch starts the five-second low-level tone automatically.

To remove an installed copy, highlight `PS3 AirPlay` in XMB, open its options, and choose **Delete**. If the app has created `/dev_hdd0/tmp/ps3-airplay.log`, that single log can be removed separately through FTP. Do not reboot without user approval; stop and report serious failures first.

## Next checks

1. User confirms the title and icon shown on the TV, successful launch, clean exit, and whether the tone is audible.
2. Retrieve `/dev_hdd0/tmp/ps3-airplay.log` after the launch test; inspect locally and redact before sharing.
3. Only after successful launch/audio confirmation, leave the receiver running and test iPhone discovery and RAOP negotiation.
