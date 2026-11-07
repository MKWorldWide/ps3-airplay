# v0.1.3 Launch Failure and SDK Alternatives

**Date:** 2026-11-07 (America/New_York, EST)
**Hardware:** PS3 with Evilnat 4.93 CFW; no model or unique console identifiers recorded.

## Result

After v0.1.2 returned XMB error `80028F21`, the user installed the approved AIRPLAY02 v0.1.3 package. The user reports the same launch error. The exact installed version was verified read-only: installed `PARAM.SFO` reports `APP_VER=0.1.3` and `TITLE_ID=AIRPLAY02`, and the installed EBOOT matches the EBOOT extracted from the v0.1.3 package.

The v0.1.3 package is 3,183,408 bytes with SHA-256 `98d8701ab018df882e30ba28f851a994c195d0a144dafe7c68c77d86d94126aa`. Its extracted/installed EBOOT is 402,928 bytes with SHA-256 `b3dbbdd236ba771b032bae9dbd2614b4a007f1c1973f586f4dd769021fbce451`. The pre-package SELF input is 402,880 bytes with SHA-256 `1b7a1e0b2430fc91c6b2969e3d7b6e61c57aa945e58833b33d0ddc699f0b9b82`; package extraction adds 48 bytes. The installed EBOOT matches the extracted package, so the package transfer/install identity is established.

The retrieved diagnostic log is 1,817 bytes and byte-for-byte unchanged from the prior copy. No new startup event was observed. This does **not** prove that execution failed before `main()`: logger open/write failure remains possible. The cause of `80028F21` is unknown.

## Test boundary

| Check | Result |
|---|---|
| Package hash and structure | PASS locally |
| Approved FTP staging | PASS; downloaded-back bytes matched |
| Installed AIRPLAY02 version | PASS; installed SFO says 0.1.3 |
| Installed EBOOT identity | PASS; matches extracted v0.1.3 EBOOT |
| XMB launch | FAIL by user report; same `80028F21` |
| Fresh application log | NOT OBSERVED; previous log unchanged |
| Display, controller exit, diagnostic tone | NOT RUN; app did not launch |
| RAOP connection, RTP, ALAC, streamed audio | NOT RUN |

No system settings, CFW files, boot plugins, or unrelated packages were changed during this verification. v0.1.2 remains staged as the prior rollback package. The resident SPRX remains outside this deployment.

## SDK alternatives

The [Thermal Bench project](https://github.com/mohasi/ps3-dev) says its PS3 apps and plugins are built with Sony's official SDK 4.75 inside a Windows 7 VM via MSBuild and SSH. This is a viable alternative only if a properly licensed SDK environment is available. It is not a drop-in compiler change: the app's PS3 APIs, startup/runtime assumptions, and NPDRM packaging need a deliberate port and hardware validation.

There is also a lower-friction open-source option. [PS3DK](https://github.com/FirebirdTA01/PS3DK) now has its own native runtime and cell-style headers while retaining PSL1GHT compatibility for existing homebrew. A minimal native PS3DK launch-control app in the existing Lima environment is the best next comparison: it can distinguish an app/runtime/package path issue without rewriting the AirPlay implementation. It will not by itself prove that every current AirPlay dependency or API has been ported.

Read-only Lima inspection confirms the VM is running and has the PS3DK source/toolchain staging and host package tools. The current `scripts/build-lima-smoke.sh` builds portable receiver objects plus a PS3DK PRX sample; it does not produce a standalone app suitable for an XMB launch comparison. The minimal standalone control therefore still needs to be implemented and validated locally.

The current `main.c` also labels startup as `PKG_VERSION_0.1.2` even in the v0.1.3 source. This did not cause the observed launch result, but it must be corrected before another receiver package so future logs identify the build accurately.

## Next steps

1. Build and statically validate a minimal native PS3DK launch-control artifact in Lima; do not deploy it without a separate approval for its exact artifact and rollback plan.
2. Inspect the current NPDRM packaging inputs and SELF conversion against the working Thermal Bench flow, without copying proprietary SDK files or making compatibility claims.
3. Correct the version marker and add an explicit logger-open failure path before another full receiver build.
4. If the native control launches, port the smallest receiver shell incrementally and test each boundary on hardware. If it fails, capture the best available loader/package diagnostics before considering a proprietary SDK port.

## Privacy and evidence

The report includes only the demo console's user-authorized firmware version and non-unique package/build hashes. It omits host paths, machine names, local IPs, peer data, and raw logs. The original log remains in ignored local hardware evidence and is not committed.
