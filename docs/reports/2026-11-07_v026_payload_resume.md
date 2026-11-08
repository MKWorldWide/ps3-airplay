# v0.2.6 / SFO 00.46: payload, recovery and XMB resume candidate

Source: `bca0f94901acdd7eb97f3b824e372f53beee7ee3` on main.

## Changes

- Shared 1,536-byte RTP/AES/ALAC payload limit admits valid uncompressed 352-frame stereo ALAC. Added synthetic encrypted regression with an eight-byte clear AES tail; reference PCM is byte-exact.
- Split larger resend gaps into <=16-packet chunks; bounded queue, >=10ms request pacing, at most two retries >=100ms apart, one-second tracking lifetime. Received/played packets cease retry eligibility. Invalid input clears stale gap metadata.
- Five-second malformed-RTP and retry deltas retain append-only UTC history and the 5 MiB per-launch cap.
- Track DRAW_BEGIN/END and MENU callbacks. Pace flips for two seconds after resume, restore both images, then return to static scanout. No added blocking CPU waits.

## Validation

Complete `scripts/release-lima-standalone-receiver.sh` gate passed from the exact committed source: existing host ASan/UBSan crypto/session/RTSP/UDP/ALAC/PCM tests, 4,352 malformed-codec cases, full-cap append regression, additional payload/retry/jitter/SRC tests, production presenter mocks, PPU component/app compilation, CellOS ELF validation, NPDRM EBOOT, package extraction, identity/assets and SFO 00.46 checks.

Display mocks verify paced settling, static completion and bounded stalled-flip timeout. They do not verify physical VSH composition, brightness, controller support or HDMI pixels.

## Artifact

- AIRPLAY02 app/package `0.2.6`, SFO `00.46`.
- PKG: **3,233,072 bytes**.
- SHA-256: **`472915766a1b950f44558259a3583b5931c0bb3f8d3a8b0e5f1a044715927ca6`**.
- Retained locally under ignored `work/builds/0.2.6/`; guest build log also retained privately. Local retrieved package hash matches guest build.
- No console exit, install, launch, reboot or configuration change in this pass. Known-safe v0.2.4 artifacts/tag preserved.

## Hardware acceptance pending

Launch, open PS menu, resume and inspect brightness/loading overlay; verify START exit; repeat twice. Then same-song playback comparison using malformed/retry/missing/recovery/concealment deltas. The payload bug is reproduced and fixed in host tests; its contribution to physical stutter and the display candidate's visible success still require hardware evidence. Sender synchronization and higher-quality resampling remain later work.
