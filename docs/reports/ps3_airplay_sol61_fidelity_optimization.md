# PS3 AirPlay fidelity, clock stability and XMB resume investigation

## Scope and provenance

Production main remains unchanged. Candidate source is on `lily/fidelity-v027` in a separate worktree. AIRPLAY02 v0.2.6 / SFO 00.46 is the physical baseline; packaged source is `bca0f94901acdd7eb97f3b824e372f53beee7ee3`, not the later documentation commit. Installed EBOOT SHA-256 `1e5ce7111e74b9c89924ad8c29d874ecb8780b4745282096f5a341072186e4dd` (452,592 bytes) matches the retained package extraction exactly. Annotated `baseline-v0.2.6` pins that source; `safe-v0.2.4` is preserved. No console exit, deployment, reboot or setting change occurred.

## Read-only hardware evidence

Acquired 2026-11-08T03:50:14Z: 2,195,622 bytes, SHA-256 `d431975b0f279914bd716249e26008972e169215f28bc87fae07b4a39b0c9397`.
At the user's request, reacquired **2026-11-08T03:54:30.747906Z**: **2,336,774 bytes**, SHA-256 **`4ba0297a3a84f8f57dfdc0f2bb3f727d7ac89289db04bc9d7f8c519fea71fa34`**. Both originals are retained separately under ignored local work. Live FTP snapshots are not transactional; the device clock is roughly 35 seconds ahead of acquisition time, so use device-relative durations.

Current v0.2.6 session: APP_START 03:28:34Z, latest event 03:55:09Z, **26m35s** application uptime. There are five connection/RECORD epochs and initial FLUSH boundaries. This is not one uninterrupted stream. No cap/wrap occurs in this session; older appended sessions include historical wrapping/capping. All lines parse.

The stated preliminary seven-minute-28-second boundary is independently reproduced: through 03:36:02Z, periodic missing observations 4,833, recoveries 4,725 and concealments 108, recovery share 97.77%. The last 108th concealment was reported at 03:30:05Z; later preliminary intervals report zero. High-water missing observations can include reordered packets; they are not measured physical network loss.

Latest aggregate: missing observations **15,336**, accepted recoveries **15,220**, concealed packets **122**, malformed RTP **0**, ALAC failures **0**, pending drops **0**, PCM drops **0**. There is **one 3,457-output-frame underflow (~72 ms at 48 kHz)** at 03:37:25Z, shortly after reconnect/initial FLUSH. These totals span resets and differing end boundaries: 15,220 + 122 does not equal 15,336, and must not be forced to reconcile. Queue gauge mean/min/max is 100.23/53/118. PCM gauge mean/max 12,325.70/12,640; its minimum zero is near a teardown. In active earlier periods it stayed 12,058–12,640. Zero event totals do not prove flawless physical output.

Newest window since the first snapshot: missing deltas 3,016, recoveries 3,017, zero additional concealed packets, underflows, malformed RTP or ALAC failures. A teardown at 03:51:09Z and RECORD at 03:51:30Z explain the interval's extra epoch. Playback remains active at capture EOF.

## Confirmed defects and ranked explanations

1. **Linear interpolation spectral error (confirmed, strongest fidelity evidence).** 20 kHz amplitude was about 0.482 of expected and analytic sine SNR 4.17 dB. This can dull or roughen sound without ALAC failure or PCM starvation; it does not explain every timing symptom.
2. **Residual concealment/discontinuities (confirmed).** First 108 markers cluster in the first 74 seconds of streaming, not solely initial prebuffer. Fourteen more appear later, including reconnect. Each marker inserts zero PCM, creating possible abrupt boundaries. No malformed-packet or late-resend delta supports the old payload cap or universal late recovery as the remaining cause. The logs do not identify why particular requests went unanswered; no new concealment DSP or resend tuning is justified yet.
3. **Audio notification service delays (unmeasured on PS3).** Healthy PCM does not prove the correct GameOS block was written before its deadline. Shared-loop network, decode, callbacks and synchronous logging can delay service. Measure before changing threads or deadlines.
4. **Independent clock drift (unmeasured, lower current attribution).** RTP timestamps are parsed but unused for playout; sync 0xd4 is ignored. RTT is not sender/local clock mapping. No drift correction is implemented or introduced here. Stable buffers and exact nominal ratio contradict a gross rate error, but cannot rule out independent oscillator mismatch.

## Quantitative clock audit

Output follows GameOS notification pacing, with an exact nominal 147/160 input-consumption ratio. Sender arrival controls input availability but not the output clock. RTP sequence/timestamp consistency and notification presentation times are not recorded, so real sender/receiver drift or jitter in microseconds cannot be estimated from old one-second log timestamps.

First four epochs from the earlier snapshot (completed five-second intervals only):

| Epoch | PCM queued | PCM consumed | 48 kHz frames submitted | queued minus consumed | consumed minus submitted ×147/160 |
|---|---:|---:|---:|---:|---:|
| 1 | 21,948,608 | 21,936,165 | 23,876,096 | 12,443 | +1.8 |
| 2 | 17,934,400 | 17,921,888 | 19,510,272 | 12,512 | -3,174.4 |
| 3 | 5,344,768 | 5,332,221 | 5,803,776 | 12,547 | +1.8 |
| 4 | 9,429,728 | 9,417,174 | 10,249,984 | 12,554 | +1.2 |

Queued minus consumed equals each final PCM gauge. Epoch 2's difference is explained by 3,457 silent underflow output frames ×147/160 plus lookahead/phase rounding. The other epochs agree within two lookahead frames. These are submitted copies, not HDMI sample measurements. Independent drift ppm is **not measurable** from this record: batching, RTP queue variation, resets and timestamp quantization exceed the signal being estimated. No buffer-driven pitch adjustment is added.

Candidate adds aggregate service-gap maximum, render-cost maximum, observed block count, repeated/skipped modular read-index observations, main-log-call maximum and monotonic audio-stat interval milliseconds. No per-block disk I/O is added. Service counts include deliberate silence. Modular indices cannot detect whole-buffer-cycle lapses; repeat/skip counters are observations, not definitive xruns. MAIN_LOG_US_MAX measures main wrapper timestamp/format/write/flush, excluding protocol calls that bypass that wrapper. Incoming RTP timestamp consistency and full sender-clock mapping remain explicit follow-ups.

## Resampler experiment and selection

Reference: independent double-precision SciPy `resample_poly`, using a 40,961-tap Kaiser beta-12 prototype and explicit alignment of candidate delay. [SciPy documents its FIR polyphase resampling](https://docs.scipy.org/doc/scipy/reference/generated/scipy.signal.resample_poly.html); windowed-sinc FIR is also an established audio approach in [Android's sampling documentation](https://developer.android.com/ndk/guides/audio/sampling-audio).

Analytic 20 kHz comparison: linear 4.17 dB SNR; four-point cubic 5.09 dB; Blackman sinc 32 taps 22.84 dB, 48 taps 44.66 dB, 64 taps 75.12 dB. The candidate uses **64 taps ×160 rational phases**, generated with the Python standard library. Runtime uses static float coefficients and a 64-frame stereo history; no allocation, transcendental math, locks or floating clock accumulation. Integer phase retains exact 160:147 behavior. 48 kHz input retains its direct path.

Added coefficient storage 40,960 bytes, history 256 bytes plus index; fixed causal delay **31 input frames = 0.703 ms**. Stereo load ~6.144 million multiply-accumulates/second. No jitter buffer increase. Filter reset follows existing FLUSH/underflow state, with edge extension on fresh input. Intersample peaks can exceed original integer sample peaks; float output and existing conservative gain remain in use.

| Frequency | Candidate SNR vs independent quantized-input reference | Gain error dB | THD+noise dB |
|---|---:|---:|---:|
| 100 Hz | 116.27 | -0.000001 | -93.48 |
| 1 kHz | 106.89 | -0.000028 | -93.40 |
| 5 kHz | 103.65 | -0.000045 | -93.23 |
| 10 kHz | 110.90 | -0.000014 | -93.48 |
| 15 kHz | 95.96 | +0.000092 | -91.62 |
| 18 kHz | 80.85 | +0.000592 | -83.17 |
| 20 kHz | 75.09 | -0.001079 | -78.02 |

Quantized-input reference SNR excludes input quantization error; analytic tone SNR includes it (candidate 75.07 dB at 20 kHz). THD+noise includes quantization and interpolation images, not isolated analog THD. Measured tone passband gain span is ~0.0017 dB; a continuous sweep/complete stopband guarantee is not claimed. Full-band white-noise and impulse reference SNR are only **25.35/25.20 dB**, because this short filter has a wider transition near input Nyquist than the much longer reference. Bandlimited-noise reference SNR is **90.33 dB**. This is a measured limitation, not a hidden failed test.

Ten seconds of the FFmpeg public codec-test music sample [luckynight.flac](https://samples.ffmpeg.org/A-codecs/lossless/) gives **63.75 dB** reference SNR after 44.1 kHz s16le preparation. Audio stays local and is not redistributed. The former user microphone file was unavailable. Chunk sizes 1/256/480 and fresh stream restarts produce bit-identical results. A 600-second production-ring test retains exact sample conservation and no underflow.

## Scheduling and cost

Same-host optimized stereo benchmark: linear median **0.426 ms/audio-second**, FIR **3.754 ms/audio-second**, worst observed FIR **5.518 ms/audio-second** across 20 runs. This includes ring fill/copy and startup; it is host cost, not PS3 deadline evidence. FIR is materially more expensive but remains well below realtime in this environment. PS3 costs must be measured by the new counters before a long soak.

Existing synchronous format/write/fflush test: mean 2.1 µs, median 2 µs, p99 5 µs, max 221 µs over 10,000 events on host temporary storage. This cannot establish GameOS HDD latency; buffering policy is preserved to retain crash diagnostics. No threading or logger redesign is justified by host costs. The new maximum counters may miss hangs before their next aggregate emission and do not reveal every latency distribution.

## XMB restoration

v0.2.6's two-second interval is a paced-flip guard for VSH fade/composition, not a blocking CPU sleep. After expiry it restores both buffers and returns static. Flip completion only indicates scanout progress; there is no demonstrated API signal for completion of VSH's fade. Therefore a purely state-only completion would repeat the earlier premature-static risk.

Candidate retains the same bounded mechanism with a **350 ms guard**, then waits for required double-buffer flip completion. Mock tests exercise 250/350/500/750 ms guards; repeated/rapid menu cycles and paired/out-of-order close-vs-draw events are covered. Default settles within 400 ms under simulated 60 Hz service, including buffer restoration. A stalled flip retains the existing 500 ms timeout and never blocks the input/audio loop. An early test falsely assumed 30 ticks sufficed for every guard; increasing the test horizon by the selected window corrected it. No production failure was concealed.

**Expected default completion below 500 ms is a host-mock result.** Physical bright/stable HDMI, PS menu, START exit and audio under VSH must be checked twice on hardware. If 350 ms truncates the real compositor fade, use the preserved v0.2.6 baseline and evaluate a longer guard; do not declare visible success from SCREEN_RESUME_SETTLED alone.

## Validation and release boundary

Host ASan/UBSan RTP, PCM, exact long-run ratio, reorder/jitter, payload/decrypt/ALAC fixtures, presenter timeout/cycle mocks and callback state tests pass. The offline independent-reference test and host benchmarks pass their stated thresholds. The complete Linux release gate and artifact identity results are recorded below after the exact source is committed and built. No candidate hardware behavior or reliable long soak is claimed. Candidate v0.2.7 / SFO 00.47 is prepared only if that gate passes; it must not be deployed automatically.
