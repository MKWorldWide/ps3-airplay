# Native PS3DK Launch-Control Build

**Date:** 2026-11-07 (America/New_York, EST)
**Source base:** `4808fad` on `docs/v012-preflight-review`; implementation files in this report are the current working-tree changes.
**Hardware:** None. No package was sent to or installed on the PS3.

## Purpose

Build a minimal native PS3DK standalone control to separate XMB launch/display behavior from the AirPlay receiver. The test app has its own title ID, starts controller input before graphics, displays bright full-screen colors, accepts START to exit, and has a bounded automatic exit. It writes allowlisted startup and cleanup events to `/dev_hdd0/tmp/ps3-airplay-native-launch.log` when launched on hardware.

## Implementation

Source: `prototype/native-ps3dk-launch-control/`. It uses PS3DK's native build/runtime and follows the cellGcm initialization/render flow in PS3DK's `hello-ppu-cellgcm-clear` sample. It does not include receiver networking, ALAC, RAOP, or audio output. The application ID `AIRTEST01` prevents replacement of AIRPLAY02.

The first local post-link failed with `elf does not have a prx parameter section`. The same result occurred when building the upstream PS3DK cellGcm sample, so it was not specific to this app. PS3DK's `scripts/build-runtime-lv2.sh` explains that the older PSL1GHT `sprxlinker` does not handle the SDK's 64-byte `.sys_proc_prx_param`; it directs builds to the SDK fork under `tools/sprx-linker`. The fork was built and installed into the Lima guest's staged toolchain, after which the launch-control ELF, SELF forms, and package built successfully. This changed only the local Lima toolchain staging.

## Local artifact evidence

| Item | Value |
|---|---|
| App version | 0.1.0 |
| Title ID | `AIRTEST01` |
| Content ID | `UP0001-AIRTEST01_00-0000000000000000` |
| PKG filename | `ps3-airplay-native-control.pkg` |
| PKG size | 192,688 bytes |
| PKG SHA-256 | `7bb8e2fbc12cc3da8f4b91ff0ae85c4dcb105209dba5d45765b1787d4015b6d2` |
| PPU ELF size | 463,576 bytes |
| PPU ELF SHA-256 | `e02fdd56a78bea23c2aa2c64c98609999862f343104203c4a3a591ac78e01f6d` |
| Fake-signed SELF SHA-256 | `cc1e777d2df178d6a2114531ed53efc1eb4973e1b19a8a95b3912f4b9f077be1` |
| Toolchain | PS3DK v0.27.0 staging in existing Linux Lima guest; PPU GCC 16.1.0 |

The package tool validated the PKG container and reported the expected content ID. `sfo` inspection confirmed `APP_VER=0.1.0`, `TITLE_ID=AIRTEST01`, and the intended title. `pkg --list` showed the icon, SFO, and NPDRM `USRDIR/EBOOT.BIN`. Extraction matched the generated SFO byte-for-byte; the extracted EBOOT matched the generated package-stage EBOOT through all 136,528 stage bytes and contained 32 additional trailing zero bytes in the extracted package copy. These are local package checks, not XMB installation evidence.

The `build/` artifact directory is ignored and excluded from the commit; the source, CMake recipe, SFO template, instructions, and hashes are recorded here. No host unit tests were run in this pass. The cross-build and package inspection were the requested validation for this app.

## Test boundary and next action

| Behavior | Result |
|---|---|
| PPU ELF compile/link | PASS locally with PS3DK linker fork |
| SELF generation | PASS locally |
| NPDRM PKG generation | PASS locally |
| Package title/content identity and extraction | PASS locally |
| XMB launch on Evilnat | NOT RUN |
| Visible HDMI output | NOT RUN |
| START exit / automatic return to XMB | NOT RUN |
| Fresh hardware log | NOT RUN |
| AirPlay or audible streaming | NOT APPLICABLE to this control |

Next, request approval to transfer this exact package with rollback details. If installed, ask the user to check XMB launch, visible cycling colors, START-to-XMB exit, and the automatic timeout separately. Do not replace AIRPLAY02 or claim that this proves receiver compatibility.
