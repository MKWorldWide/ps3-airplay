# First PS3 hardware test — NPDRM failure and corrected staging

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
| Application installation | PASS | User installed title ID `AIRPLAY02` through XMB Package Manager |
| XMB display/name/icon | PARTIAL | Title is `PS3 AirPlay`; the first package did not contain `ICON0.PNG`; icon appearance not recorded |
| Application launch/clean exit | FAIL | Start operation returned `80029530`; launch did not reach a usable receiver |
| Five-second tone | NOT TESTED | No audibility claim; user confirmation is required |
| Diagnostic log | FAIL | No `ps3-airplay.log` appeared after the failed launch; this may indicate the app did not reach its logger, but is not conclusive |
| iPhone discovery / RTSP / RTP / ALAC | NOT TESTED | No sender connection attempted |
| Audible AirPlay or Spotify playback | NOT TESTED | No playback attempted |
| Crash or instability | NOT CONFIRMED | User reported a start error; no additional console behavior has been reported |
| Corrected PKG replacement (DEPLOY-02) | PASS | Remote size 416,240 bytes; downloaded-back file matches the new SHA-256 |

## Package details

- Filename: `ps3-airplay-receiver.pkg`
- SHA-256 of the first, failed-to-launch package: `1b797033a25bbf95f7deb8ec9e95f5d12c66edcf2fd00b2f6ceb4a9e990704c2`
- Staging path: `/dev_hdd0/packages/ps3-airplay-receiver.pkg`
- Remote size: 2,560,368 bytes
- Title ID: `AIRPLAY02`

The first staged PKG was downloaded back from the console and compared against the local artifact. The transfer was byte-identical. The installed `EBOOT.BIN` was also downloaded and matched the EBOOT inside that package, so the FTP transfer and installation preserved the original payload. The FTP server did not provide a remote SHA-256 command; local hashing of the downloaded-back files supplied the integrity check.

## Corrected package (staged; not yet installed)

- Filename: `ps3-airplay-receiver.pkg`
- SHA-256: `f1b17d07a05679030143e607c74b545592e92ad521135b860bc2aca677160cf2`
- Size: 416,240 bytes
- Content ID: `UP0001-AIRPLAY02_00-0000000000000000`
- Package listing includes `ICON0.PNG`, `PARAM.SFO`, and an NPDRM SELF `USRDIR/EBOOT.BIN`.
- The EBOOT was generated with PSL1GHT's `make_self_npdrm` using the same content ID supplied to the PKG tool. Package extraction, SFO title ID `AIRPLAY02`, title `PS3 AirPlay`, and the default icon comparison passed in Lima.
- Replacement was approved and transferred to the same staging path. The downloaded-back file is byte-identical and its local SHA-256 matches. The installed title still contains the first package and has not been updated.

## Launch failure investigation

The PS3 error reference classifies `80029530` as `SCE_NP_DRM_ERROR_FORMAT` and notes that a content-ID mismatch can cause a start failure ([PS3 Developer Wiki error codes](https://www.psdevwiki.com/ps3/Error_Codes)). The package script had wrapped the ELF with generic `fself`, while PSL1GHT's CEX package rules use `make_self_npdrm` and pass the same content ID used by the PKG ([PSL1GHT PPU rules](https://github.com/ps3dev/PSL1GHT/blob/master/ppu_rules)). This is the leading root-cause hypothesis; it has not yet been confirmed by launching a corrected package.

No startup log was found. The app opens the logger before network initialization, so the absence is consistent with a loader/NPDRM failure, but a log-file open failure could produce the same observation. The corrected package uses the bundled default `ICON0.PNG` and binds its EBOOT to `UP0001-AIRPLAY02_00-0000000000000000`.

## Target and safety notes

Existing project records identify the demonstration target as Evilnat 4.93 CFW, with Cobra 8.5 previously observed; the console model was not recorded. The `PS3_SYSTEM_VER` minimum is `01.8000`, which is below that reported firmware, but install and runtime compatibility remain unverified.

Only the standalone PKG was uploaded and installed. No SPRX was transferred, no plugin was installed or loaded, and no firmware, webMAN, audio, or boot plugin settings were changed. The package is not configured to start at boot.

## Install and remove

The first package is installed as `AIRPLAY02`, but its launch failed. The corrected package is now staged and verified. Use **Game → Package Manager → Install Package Files → Standard**, choose `PS3 AirPlay`, and update/reinstall. If the package manager asks whether to replace the existing title, accept; if it refuses, stop and report the exact message. The first successful app launch should start the five-second low-level tone automatically.

To remove an installed copy, highlight `PS3 AirPlay` in XMB, open its options, and choose **Delete**. If the app has created `/dev_hdd0/tmp/ps3-airplay.log`, that single log can be removed separately through FTP. Do not reboot without user approval; stop and report serious failures first.

## Next checks

1. User updates/reinstalls the corrected package and reports XMB display, clean launch/exit, and whether the tone is audible.
2. Retrieve `/dev_hdd0/tmp/ps3-airplay.log` after launch; inspect locally and redact before sharing.
3. Continue with iPhone discovery and RAOP only after the app launches without the NPDRM error.
