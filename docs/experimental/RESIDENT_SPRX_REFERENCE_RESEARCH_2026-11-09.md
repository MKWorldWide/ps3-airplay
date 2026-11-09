# Resident SPRX reference research: webMAN MOD

**Timestamp:** 2026-11-09 01:13 EST
**Branch:** `experimental/resident-airplay`
**Purpose:** Compare a publicly released PS3 plugin and its published build configuration with the resident loader candidates. This is offline artifact analysis only.

## Source and artifact

Brewology’s listing identifies a 4.93-targeted webMAN MOD package and links to the project’s GitHub source. The GitHub release API identifies tag `1.47.50` (release name `1.47.50d`), whose installer asset is `webMAN_MOD_1.47.50_Installer.pkg`. The Brewology direct download endpoint returned HTTP 403 in this environment, so the binary examined here was downloaded from the project’s official GitHub release, not from Brewology.

- Brewology listing: https://store.brewology.com/ahomebrew.php?brewid=310
- Project and GPL-3.0 license: https://github.com/aldostools/webMAN-MOD
- Examined source tag: https://github.com/aldostools/webMAN-MOD/tree/1.47.50 (tag commit `d1d5b5271dc409677d101220e87024419fa59bcb`)
- Release asset: https://github.com/aldostools/webMAN-MOD/releases/download/1.47.50/webMAN_MOD_1.47.50_Installer.pkg
- PS3 SELF/Certified File format references: https://www.psdevwiki.com/ps3/Certified_File and https://www.psdevwiki.com/ps3/SELF_-_SPRX

The PKG was extracted offline using the relevant extraction functions from the public [PyKG project](https://github.com/AphelionWasTaken/PyKG); the helper and third-party package were kept under ignored local build output and were not committed. The installer is 14,906,656 bytes (SHA-256 `84539659e434a651602690fd9a07689b513a1a0ff0b44d4c21c2ffff74e346cc`). It contains four webFTP SPRX variants. The full variant selected for inspection is 354,081 bytes (SHA-256 `bbffdfbb998e40219cf9619cae7fe0bf121a7824866385f954c112945c8f6192`). Other extracted variants were: lite, 139,482 bytes (`5cd074dc4e52d1632cfcaf3d738a038ce24dd92d06357d46244f61877e1ab378`); non-Cobra, 152,981 bytes (`ed9501ade5234288c966c16254932d4a418588e71174dde45ce30c52a894ae43`); and REBUG Cobra PS3MAPI, 330,712 bytes (`43224034c37266fc63acb3159f81f58318321e0f0676666b0cbf954cafbdde49`). These are local, ignored research artifacts; none are included in the repository change.

The full SPRX has an `SCE\0` SELF header, version 2, attribute/key revision `0x000A`, category `0x0001` (SELF/SPRX), extended-header size `0x240`, embedded-data offset `0x580`, and declared data size `0x561A1`. These values are read from the public SELF header; this pass did not cryptographically validate the signature or decrypt the embedded ELF.

## Published build configuration

At tag `1.47.50`, the upstream makefile builds `webftp_server.prx` with the Sony CELL SDK (`ppu-lv2-gcc`/PS3 SDK make rules), strips it, then wraps it with `scetool`. The scetool invocation sets key revision `0A`, auth ID `1070000052000001`, vendor ID `01000002`, SELF type `APP`, and its target SCE/app versions. The linker command uses Sony SDK import stubs including `sysPrxForUser`, `sysmodule`, `net`, `netctl`, and VSH-facing libraries such as `vsh_export`, `vshcommon`, `vshmain`, `vshtask`, and `paf`. The `full` build flags enable `COBRA_ONLY` and `REX_ONLY` as well as the full edition features.

This gives us an authentic structural and build-pipeline reference, but not a drop-in module for our user application. The plugin is designed for the webMAN/VSH/Cobra environment and declares VSH-facing imports. It may depend on that loader context and privileges. Its success on supported CFW does not demonstrate that it should load through our harness’s `sys_prx_load_module` call.

## Comparison with AIR PS3 00.04

| Candidate | SELF attribute / key revision | Category | Result in the 00.04 harness |
|---|---:|---:|---|
| webMAN MOD full SPRX | `0x000A` | `0x0001` | Public comparison artifact; not run or installed |
| AIR PS3 resident SPRX | `0x0001` | `0x0001` | `sys_prx_load_module` returned `0x80010007` (`ENOEXEC`) |
| Minimal no-import probe, scetool-signed at `0x000A` | `0x000A` | `0x0001` | All three attempts returned `0x80010007`; module count remained at 11 |
| SDK-layout reference probe, scetool-signed at `0x000A` | `0x000A` | `0x0001` | All three attempts returned `0x80010007`; module count remained at 11 |

The 00.04 log shows module prerequisites and the foreground audio port initialized successfully. Because both diagnostic probes failed before `module_start`, the harness skipped its configured 50 resident lifecycle/audio cycles and submitted no test-tone blocks. Cleanup succeeded and returned the module count to its application baseline of 9. No resident worker, RAOP flow, or resident PCM submission occurred.

The controlled result is useful: changing the probe containers to attribute/key revision `0x0A` did not make either candidate load. Therefore that revision by itself is not a sufficient fix. It does **not** prove that revision is irrelevant: the test candidates were not produced by webMAN’s full Sony CELL SDK build pipeline, do not share its VSH imports or plugin loader context, and we did not validate/decrypt their complete signing metadata. The loader cause remains unknown.

The `0x80010007` and baseline counts are from the installed 00.04 diagnostic run. The downloaded webMAN artifact was never installed, executed, or sent to a console. No Cobra or boot-plugin setting was changed.

## Recommended next experiment

Keep the failing resident unchanged. Build a tiny no-import module with the same Sony CELL SDK PRX link and scetool signing pipeline as the reference, package it under the existing isolated diagnostic title, then use the same manual harness load path. If that candidate loads, compare its inner ELF program headers and module metadata with the PS3DK probes before incrementally adding dependencies. If the exact-pipeline no-import module also fails, investigate SELF authentication/loader-context differences rather than changing AirPlay code. This next hardware experiment was not performed in this pass.
