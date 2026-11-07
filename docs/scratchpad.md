# Active work

- Physical console has production AIRPLAY02 v0.1.39 / SFO 00.39 installed and running; it was left open for user observation.
- Verified production PKG: 3,236,976 bytes, SHA-256 `9f67340331eaae9e5ecbe94adaee03a0201e79e652a6bf45fa14cf6922877840`; installed SFO verified through FTP.
- Fresh log reaches listener, mDNS, audio, and tone-complete checkpoints. Those events do not prove visible pixels or audible output.
- Same-LAN browse found RAOP. TCP OPTIONS connected; receiver logged response status 200 after successful send, but probe client received EOF. Investigate this mismatch before broader AirPlay claims.
- No v0.1.39 iPhone session, RTP, streamed decode, Spotify playback, user-confirmed display/tone, or clean-exit result yet.
- Next: collect the user's screen/tone/exit observations; then one controlled iPhone attempt and immediate fresh-log retrieval. Keep raw logs local/ignored.
