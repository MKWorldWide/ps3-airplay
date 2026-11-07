# v0.1.33 live audio follow-up

## Capture identity

- Captured: 2026-11-07 20:16:38 UTC (15:16:38 EDT).
- Active application identity: AIRPLAY02 v0.1.33 / SFO 00.33. Installed PARAM.SFO and EBOOT.BIN downloaded over FTP matched local v0.1.33 files byte-for-byte and did not match v0.1.34.
- v0.1.34 is a separate staged package: 3,236,016 bytes, SHA-256 `8828e032fea57ab1ce4e0041f0c802c45940f3c7abe93caf3d9969fa9f01e80f`. The remote staged file matched the local candidate byte-for-byte; it was not installed during this capture.
- Raw log snapshot is retained privately in the ignored `work/hardware/2026-11-07-v0134-live/` directory. No raw log or local network identifiers are included here.

## Hardware observations

The maintainer reports that Spotify music remains audible through AirPlay, with crackling/stuttering that becomes more noticeable while scrolling horizontally through XMB. The image flickers and appears dim after returning from XMB, with a frozen loading indicator. Exiting and relaunching the app restores a solid background. This separates a reproducible audio-quality problem from an inconsistent display-resume problem.

## Sanitized log findings

- The retained live portion records an encrypted 44.1 kHz RAOP stream.
- It contains thirteen 128-packet receive checkpoints: 1,664 received RTP packets and 88 sequence gaps. Four resend requests and five resend-recovery events were recorded. The per-checkpoint RTP queue depth was generally 60–90 packets.
- Three ALAC failure totals were zero. Sampled decoded ALAC frames were 352 samples. Therefore, the captured evidence does not point to repeated ALAC decoder failure.
- The log records one `PCM_UNDERFLOW` delta of 51,200 frames. Source inspection shows v0.1.33 renders from an empty PCM ring while the decoder/prebuffer is not ready; those expected silent blocks incremented the same counter. Treat this delta as invalid for judging active-playback starvation. v0.1.35 corrects the counter boundary and resets its baseline for each session.
- The log had wrapped near its 16 KiB cap and the snapshot ended on `SCREEN_FRAMES`, without `APP_STOP`; the app was still running at capture time. Startup and initial RTSP negotiation are absent.

## Supplied microphone sample

The user supplied a private 22.485-second mono 48 kHz recording made with the MacBook microphone. The source file remains outside the repository. Gross signal analysis measured a -23.9 dBFS peak and -40.7 dBFS RMS. It found no segment longer than 10 ms below -50 dBFS and no adjacent sample jump above 0.25 full scale. This does not establish that the recording is crackle-free: the sample is quiet and captured acoustically, so subtle crackles or short dropouts may be masked by room/speaker response. No voice transcription or music identification was performed.

An earlier attempted capture selected the iPhone input by default rather than the MacBook microphone. That partial file was excluded from analysis; microphone selection must be explicit in future captures.

## Assessment: Spotify lossless versus receiver pipeline

The strongest measurable receiver-side issue is RTP sequence loss: 88 gaps appear in the retained v0.1.33 receive batches while decryption and ALAC decoding continue successfully. The reported `PCM_UNDERFLOW` count cannot be treated as an active-playback dropout because v0.1.33 counts intentional prebuffer silence; v0.1.35 corrects this. The user's report that XMB movement worsens the sound makes realtime scheduling/resource contention a plausible contributor. The acoustic sample does not show gross digital dropouts, but its low capture level limits what it can establish. None of these observations proves a sole cause.

The receiver log reports the AirPlay transport format (44.1 kHz ALAC), not the Spotify file/source quality entering the iPhone's AirPlay sender. Spotify documents Lossless as up to 24-bit/44.1 kHz FLAC and notes that connection method and network stability can affect Lossless playback ([Spotify support](https://support.spotify.com/us/article/lossless-audio-quality/)). This evidence does not establish that Spotify's Lossless option is being delivered end-to-end over this AirPlay route or that it caused the RTP gaps. A controlled quality comparison is useful only after testing the existing v0.1.34 recovery candidate.

## Gate results

| Gate | Result |
|---|---|
| Installed application identity | PASS: exact v0.1.33 SFO/EBOOT match |
| Active AirPlay audio | PASS by user report |
| RTP continuity | DEGRADED: 88 gaps in 1,664 received packets |
| Decryption / sampled ALAC | PASS: active encrypted stream, zero sampled ALAC failures |
| PCM continuity | INCONCLUSIVE: audible crackling is user-confirmed, but the v0.1.33 underflow counter includes intentional prebuffer silence |
| XMB audio stress | DEGRADED by user report; crackling increases while navigating |
| Display resume | FAIL/inconsistent by user report |
| Clean app exit | Not established by this snapshot; the captured app was running |
| v0.1.34 hardware behavior | NOT TESTED: staged, not installed |
| v0.1.35 underflow-accounting change | HOST TEST PASS pending full build; hardware validation pending |

## Next action

Let the current v0.1.33 instance exit cleanly, then install v0.1.35, which includes v0.1.34's RTP resend recovery and corrected active-stream underflow accounting. Compare five-second RTP gap/recovery and PCM-underflow totals during a short stream with XMB idle, then repeat while scrolling. Only after this comparison, lower Spotify's AirPlay source quality for an A/B test if crackling remains. Keep display resume as a separate gate; do not run a long soak yet.
