# Active work

- **Current task:** Isolate the v0.1.26 RPCS3 ANNOUNCE stall with finer session events, then continue to SETUP/RECORD/RTP.
- **Latest package:** AIRPLAY02 v0.1.26, emulator-only RTSP 5001; PKG SHA-256 `82e1407b92a1218ec3085f57bd8f6c35e97255569b92a44249e14239108fc246`. Source commit `6f8d6cb6f75e331d989419aaf51bd1d147704cad`.
- **Verified:** Host/ASan/UBSan suite and PPU object compilation pass. RPCS3 starts, obtains a route, opens RTSP/RTP, answers OPTIONS 200, and logs SDP parse success on ANNOUNCE. No response follows; RSA unwrap did not start.
- **In progress:** v0.1.27 adds ANNOUNCE field, validation, and response-construction events. The next replay should identify which boundary is last reached.
- **Unverified:** SETUP/RECORD/RTP, iPhone interoperability, actual audio, visible pixels, controller exit, and physical PS3 behavior. Desktop remains locked; PS3 and boot plugins unchanged.
- **Constraints:** Production RTSP port 5000; RPCS3 uses 5001. Keep resident SPRX/game-audio mixing separate.
