# Active work

- Current release: 0.1.30, SFO 00.30. Correcting the historical 0.30 filename/log label only; runtime source is the recovered hardware-launch baseline.
- Confirmed hardware: app starts, tone audible, START exits while idle. AirPlay route visible but no music; START/PS unresponsive during attempted sender connection.
- Preserved failed-run log (2,774 bytes) ends at OPTIONS without response; no ANNOUNCE/SETUP/RECORD/RTP or exit events. Apple-Challenge/RSA blocking is the next hypothesis to instrument, not an established root cause.
- Next: commit/build/stage corrected-label package, preserve original 0.30 artifact and raw log. Next runtime fix must use a newer release and target OPTIONS responsiveness before extended streaming.
- No boot-plugin, firmware, or resident-module changes. User authorized automatic standalone package installation.
