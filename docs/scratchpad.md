# Active work

- Current hardware baseline: AIRPLAY02 v0.1.35 / SFO 00.35 is installed. Latest private FTP capture shows 195 RTP sequence gaps and zero recorded resend recoveries alongside 86,106 active-stream PCM underflow frames; ALAC failures are zero.
- Leading audio hypothesis: unrecovered RTP loss explains most observed dropout duration. v0.1.36's bounded late-resend acceptance is included in the v0.1.37 candidate; hardware comparison remains necessary.
- Display change: replace continuous static-image flips with a resume redraw and inactive-buffer refresh. The user-reported flicker has no matching presentation-error event, so hardware confirmation is required.
- Source edits are present locally. Finish the decision/memory records, privacy review, commit/push, then run Lima sanitizer/PPU/package validation and RPCS3 synthetic replay.
- PS3 package directory is empty. Do not replace AIRPLAY02 while it is running; verify a safe exit before deployment and collect a version-matched log after the test.
