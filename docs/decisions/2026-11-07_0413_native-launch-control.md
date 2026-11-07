# Build Native PS3DK Launch Control

**Timestamp:** 2026-11-07 04:13 America/New_York (EST)

## Scope of the pass

Implemented and built a separate native PS3DK standalone app for XMB launch/display diagnosis. Added build instructions, package evidence, current project status, hardware gates, and the decision record. Corrected the local Lima PS3DK post-link tool mismatch.

## Why this pass was needed

AIRPLAY02 v0.1.2 and v0.1.3 both produce user-reported XMB error `80028F21`, and the current receiver logger has no fresh startup event. A small independent app can test the PS3DK runtime/package/display path without installing another receiver build or changing the existing AIRPLAY02 app.

## Files changed

- `prototype/native-ps3dk-launch-control/CMakeLists.txt`
- `prototype/native-ps3dk-launch-control/README.md`
- `prototype/native-ps3dk-launch-control/sfo.xml`
- `prototype/native-ps3dk-launch-control/source/main.c`
- `docs/DEV_SETUP.md`
- `docs/RISKS.md`
- `docs/ROADMAP.md`
- `docs/TEST_MATRIX.md`
- `docs/lessons-learned.md`
- `docs/memory.md`
- `docs/scratchpad.md`
- `docs/reports/2026-11-07_native_launch_control_build.md`
- `docs/decisions/2026-11-07_0413_native-launch-control.md`

## Architecture / logic changes

No AirPlay implementation changed. The new `AIRTEST01` test app uses PS3DK v0.27.0's native build/runtime and cellGcm display APIs. It initializes controller input first, uses the active video mode, cycles six bright colors, records bounded privacy-safe stage events to a separate log, polls RSX labels/flips with finite limits, exits on START, and automatically exits after 1,200 frames (about 20 seconds at 60 Hz). The package has APP_VER 0.1.0 and a separate title ID, so installation will not replace AIRPLAY02.

The initial post-link failure was resolved using evidence in the PS3DK source: its runtime build script says the older PSL1GHT `sprxlinker` does not support the PS3DK process-parameter section and instructs use of its own fork. Building/installing that fork in the local Lima staging made both the upstream cellGcm sample and this app build successfully.

## Runtime behavior changes

None on the PS3. This pass changed local project files and the Lima guest's staged `sprxlinker` only. The app has not been transferred or installed. No firmware, HDMI configuration, LV2, boot plugins, or other console files were touched.

## Validation performed

- Read `docs/memory.md` and `docs/scratchpad.md` before implementation.
- Built the control with PS3DK v0.27.0 / PPU GCC 16.1.0 under the existing Lima guest.
- Verified a reproducible failure using the old linker with the app and the upstream `hello-ppu-cellgcm-clear` sample.
- Built/installed the PS3DK `tools/sprx-linker` fork as directed by `scripts/build-runtime-lv2.sh`; rebuilt the app successfully with `-Wall -Wextra -Werror`.
- Generated ELF, real/fake SELF forms, and the NPDRM PKG.
- Inspected ELF architecture, PKG listing, content ID, SFO APP_VER/title ID, and package extraction; stage SFO matched extracted SFO, and EBOOT prefix matched stage output with 32 trailing zero bytes in the extracted copy.
- Recorded the PKG SHA-256; kept build outputs in ignored `build/`.
- No host unit tests run. No PS3 hardware test run.

## Immediate results

- Native launch-control PKG: 192,688 bytes; SHA-256 `7bb8e2fbc12cc3da8f4b91ff0ae85c4dcb105209dba5d45765b1787d4015b6d2`.
- Local PS3DK build is successful after toolchain correction.
- XMB launch, HDMI pixels, controller exit, and log writing remain unknown until a separately approved hardware test.

## Known limitations / follow-ups

- The app displays color fields only; it does not draw on-screen text or test audio.
- A completion event after local graphics setup does not prove HDMI pixels are visible.
- The log can fail to open; the app records `LOG_OPEN_FAILED` only to stdout in that case.
- The control does not validate receiver startup, networking, codecs, AirPlay, audio, or the old receiver package's dependencies.
- `main.c` still has a hard-coded receiver package-version event in the AIRPLAY02 source; fix it separately before the next receiver build.

## Risks introduced

- A PS3 test could still show no picture or fail to exit, despite bounded host-side polling. The separate title ID prevents replacing AIRPLAY02 but does not eliminate generic app-launch risk.
- The PS3DK forked `sprxlinker` replaced the older binary in this local Lima staging only; other environments need their own matching toolchain setup.
- Hardware installation requires approval of the exact PKG and rollback plan; no transfer is authorized by this implementation request.

## Decision record

### Decision 1: Test a tiny standalone app before another receiver package

- **Context:** Two receiver versions failed with the same launch error, without fresh app logs.
- **Options considered:** Rebuild the receiver again; switch immediately to the proprietary Sony SDK; isolate XMB/runtime/display with a small PS3DK-native app.
- **Why chosen:** The control tests a narrower boundary and avoids replacing the installed AIRPLAY02 app.
- **Tradeoffs accepted:** It cannot establish AirPlay, audio, or receiver dependency compatibility.

### Decision 2: Use separate title ID and automatic exit

- **Context:** The test must not overwrite AIRPLAY02 and should limit time spent in a potentially unstable foreground app.
- **Options considered:** Reuse AIRPLAY02 identity; use a new ID and START exit only; use a new ID with START and an automatic frame cap.
- **Why chosen:** `AIRTEST01` isolates installation and the 1,200-frame cap gives a fallback exit path.
- **Tradeoffs accepted:** The automatic frame cap approximates 20 seconds at 60 Hz; behavior still requires hardware confirmation.

### Decision 3: Keep the output silent

- **Context:** Current failure happens at XMB launch; adding audio would add another subsystem without helping isolate the display/startup path.
- **Options considered:** Include a diagnostic tone; keep the app display/controller only.
- **Why chosen:** Isolate launch, video, input, and exit first.
- **Tradeoffs accepted:** This test gives no evidence about GameOS audio initialization.

## Next recommended step

After privacy review and commit, request approval to upload the exact 192,688-byte `AIRTEST01` PKG (SHA-256 above) as a new file, preserving the staged v0.1.2 rollback. Then test one XMB launch, visible colors, START exit, automatic timeout only if needed, and the fresh `/dev_hdd0/tmp/ps3-airplay-native-launch.log`. Do not install or load the resident SPRX.
