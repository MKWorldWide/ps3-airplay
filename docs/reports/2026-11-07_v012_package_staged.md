# v0.1.2 package staged for XMB installation

**Timestamp:** 2026-11-07 03:09 EDT
**Source revision:** `a6b36cd43fe289b63549cbd06b888f984fe87936`

## Staging result

Following the user's explicit approval for staging only, the local candidate `build/lima-0.1.2/ps3-airplay-receiver-0.1.2.pkg` replaced the staged file at `/dev_hdd0/packages/ps3-airplay-receiver.pkg` over the existing FTP connection.

- Version / application ID: 0.1.2 / AIRPLAY02
- Size: 3,225,712 bytes
- SHA-256: `923aebf6eca5693b21b494a9a4225c36629f13e8b316b2825d5deb3763226ebf`
- Downloaded-back file hash and size match the local candidate.
- The prior staged 01.01 package (3,222,640 bytes, SHA-256 `7824a4b15e4757afad3213a3f55cd3bea816b33795403da12bfd7ede29f93db4`) remains preserved locally under ignored `work/hardware/2026-11-07-preflight/`.

## Scope boundary

This was staging only. The currently installed AIRPLAY02 title remains 01.01. No install, launch, SPRX load, firmware or settings change, or boot-plugin modification occurred. No other console files were touched.

The v0.1.2 source still contains unbounded RSX label/flip waits. The package has not been hardware-tested. The prior log does not prove visible display, normal exit, or AirPlay connection. The first user test after XMB installation must check the screen, tone, and START exit; stop if the image is black or exit fails.

## Rollback

To restore the staged file, upload the verified local 01.01 backup back to `/dev_hdd0/packages/ps3-airplay-receiver.pkg` and verify its expected size and SHA-256. If AIRPLAY02 is later installed and must be reverted, return to XMB and reinstall the restored 01.01 package. The existing emergency power-off/reboot path is for an unresponsive app only.
