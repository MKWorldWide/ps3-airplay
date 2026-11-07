# RTP gap handling and reference review

## Scope and source comparison

Reviewed the PS3 receiver's bounded RTP queue against two maintained/public AirPlay 1 receiver projects:

- [Shairport Sync](https://github.com/mikebrady/shairport-sync): its audio buffer is indexed by RTP sequence. The current source documents a 1,024-packet buffer, a 512-packet minimum (~4 seconds for 352-frame/44.1 kHz ALAC packets), and forwards retransmitted packets back into the player queue ([player.h](https://github.com/mikebrady/shairport-sync/blob/master/player.h), [rtp.c](https://github.com/mikebrady/shairport-sync/blob/master/rtp.c)). Its README and code cover synchronization against output-device timing and resampling/drift correction.
- [OpenAirPlay1](https://github.com/st3fan/openairplay1): its sequence jitter buffer uses 512 slots, reports missing sequences, and skips a gap after 128 packets of lead (~1 second) with an explicit lost-frame result for silence concealment. Its source includes sequence-wrap and recovery tests ([jitter.rs](https://github.com/st3fan/openairplay1/blob/main/openairplay1/src/jitter.rs), [robustness.rs](https://github.com/st3fan/openairplay1/blob/main/openairplay1/tests/robustness.rs)). Its clock model chooses a low-RTT clock offset and maps an RTP timestamp anchor to output/DAC time ([clock.rs](https://github.com/st3fan/openairplay1/blob/main/openairplay1/src/clock.rs)).

Our implementation already has sequence sorting, bounded resend tracking, a 256-packet RTP queue, a 128-packet startup prebuffer, GameOS PCM buffering, RTT probes, and numeric loss/decode/underflow diagnostics. A code inspection found that pop ignored the sorted queue's expected sequence: it released a later packet immediately, so a resend arriving afterward was classified late. This matches the physical v0.1.35 pattern of many missing/late packets, zero retained recovery, zero ALAC failures, and substantial active PCM underflow, although those observations do not prove this was the only audible-crackle cause.

## Change

- Hold the first observed sequence hole for 80 ms, allowing the existing retransmit path to insert a recovery before playout advances.
- After the deadline, return an internal zero-payload loss marker. The standalone app converts it to exactly one negotiated ALAC frame of stereo/mono PCM silence instead of sending an invalid encrypted packet to ALAC.
- Bound the number of silence frames for a discontinuity to 128. A larger sequence jump waits the same deadline, skips the accumulated missing range, and resumes on the next real frame; logs expose concealed and skipped counts.
- Advance package defaults to v0.1.39 / SFO 00.39 after v0.1.38 was built and tested in RPCS3.

The 80 ms delay is a starting value, not a measured iPhone/Spotify optimum. The host suite tests early resend recovery, timeout concealment, late-resend rejection, and large-gap bounds. The 128-packet synthetic emulator stream had no gaps, so it did not exercise loss recovery.

## v0.1.38 emulator evidence

- Package: AIRPLAY02 v0.1.38, SFO 00.38, emulator-only RTSP port 5001.
- Package: 3,236,976 bytes; SHA-256 `882e68768901bdbb4f36775c45d69387250364e3e3a0eeb971bdc10e31d17596`.
- This package includes bounded 80 ms sequence-gap recovery/concealment, but predates the additional 128-frame large-gap cap now in v0.1.39 source.
- RPCS3 installed the package and reported version 00.38. The fresh app log includes `PKG_VERSION_0.1.38`, 1280x720 screen initialization, successful tone completion, RTSP OPTIONS/ANNOUNCE/SETUP/RECORD status 200, RSA key unwrap completion, RTP decrypt success, ALAC decode, and PCM port submissions.
- The fixture reported 44.1 kHz stereo. At the sampled checkpoint: 0 missing packets, 0 recovered, 0 concealed, 0 ALAC failures; 68,835 PCM frames queued, 56,450 consumed, 61,440 submitted. This was a finite synthetic fixture, not Spotify or acoustic output.
- The screenshot showed the background image. mDNS failed interface setup with error 125 in RPCS3, so iPhone discovery did not pass.
- Keyboard Return mapped to START; the log records `USER_EXIT_START`, `AUDIO_CLOSE_DONE`, `RECEIVER_CLOSE_DONE`, and `APP_STOP`. This is an RPCS3 clean-exit pass only.

## Validation of current v0.1.39 source

The complete Lima `scripts/build-lima-crypto.sh` suite passed with ASan/UBSan halt-on-error, including crypto/session/SDP/mDNS, persistent RTSP/RTP socket tests, encrypted RTP-to-ALAC-to-PCM integration, 4,096 seeded and 256 random malformed ALAC inputs, FFmpeg reference decode, PCM ring/resampling/gain/tone, and PPU component compilation. Receiver integration tests now wait for a bounded missing-frame deadline before expecting silence. A separate RTP unit case verifies a 299-packet jump resumes on the received frame after the deadline instead of enqueueing minutes of silence.

The standalone v0.1.39 application link, finalized package, and RPCS3 replay are still pending at this report revision. Exact current source has not been tested on PS3. The console is off and no console files were accessed or changed.

## Remaining engineering gaps

1. Build v0.1.39 from its committed source revision; validate ELF, SFO, package, content ID, and assets; install only in RPCS3 and replay the synthetic sender.
2. Add an emulator/host packet-loss replay that confirms the target's `RTP_GAP_CONCEALED` and `RTP_GAP_SKIPPED` logs; the current passing sender fixture has no loss.
3. When hardware is back, compare recovery/concealment/underflow metrics on the PS3 and get the user's independent listening/display observations.
4. Measure resend arrival times from the actual iPhone before changing 80 ms. Keep latency and occupancy visible.
5. Implement minimum sender-clock-to-output mapping and drift correction based on real timing logs. RTT probes alone do not synchronize output.
6. Continue FLUSH, volume, pause/resume, reconnect, XMB resume/flicker, and 30-minute hardware validation after short-run behavior is stable.

No universal in-game mixing, stable hardware display, stable Spotify stream, or game-audio coexistence is claimed.
