# AIRPLAY02 v0.2.6 first hardware playback snapshot

**Captured:** 2026-11-07 23:31 EDT (log timestamps are UTC)

## Build and session

- AIRPLAY02 v0.2.6 / SFO 00.46; source `bca0f94901acdd7eb97f3b824e372f53beee7ee3`.
- `APP_START` at 03:28:34Z; RTSP `RECORD` at 03:28:51Z; latest captured event 03:31:55Z. This is a 3m21s application session and about 3m04s of recorded streaming, not a soak test.
- Logs report encrypted AirPlay at 44.1 kHz, successful sampled 352-sample ALAC frames and ongoing PCM submission. The sender app is not identified by the log.

## Playback diagnostics

In the active session, 3,282 missing sequence numbers reconciled exactly as 3,174 retransmission recoveries plus 108 concealed packets (96.7% recovered; 3.3% concealed). At 352 samples per frame and 44.1 kHz, the concealed packets represent about 0.86 seconds of inserted silence across the captured stream. Retries were active. There were no malformed RTP deltas, ALAC failures, PCM underflow events or PCM drop events in this interval.

PCM occupancy stayed between 12,077 and 12,635 of 16,384 frames (~74-77%); RTP queue depth was 87-102 of 256. This does not show output starvation. It also does not measure all forms of audible instability or prove perfect synchronization.

For comparison, the retained v0.2.5 window had 20,108 recoveries and 6,886 concealed packets out of 26,994 missing sequence numbers (74.5% recovered, 25.5% concealed). The short v0.2.6 snapshot has a substantially higher recovery share, consistent with the user's report that obvious stuttering has stopped. The user still hears slight unsteadiness; remaining concealment, retransmission timing, source quality or sender-clock drift could contribute, but this log cannot identify which.

## Display resume boundary

The log records DRAW/MENU begin, end/close, then `SCREEN_RESUME_SETTLED` at 03:28:42Z. This confirms the bounded presenter state completed its resume path. It cannot establish that the physical display looked bright or stable.

## Result and next step

The receiver, decrypt, ALAC, PCM and GameOS submission paths are active. The available counters support a reduction in unrecovered packet gaps and show no PCM starvation in this short sample. Preserve the running app for a longer user-observed test if desired; compare another same-source interval and capture a fresh log before changing buffering or resampling. Raw log remains private under ignored `work/hardware/`.
