# Active work

- **Current task:** Use the fresh AIRPLAY02 v0.1.20 candidate to continue toward an iPhone RAOP session and audio output.
- **Candidate:** Source `8a4e641d114e1a613421b9ffb51f8615ad851827`; PKG SHA-256 `bfaa3f960de39daf95c2ab405bd361a031ed92bd35c76a99e667d4ffe8d19e87`, preserved locally at ignored `work/builds/0.1.20/`.
- **Host validation:** Full Lima crypto/protocol/audio suite and PPU objects pass. The ALAC sanitizer corpus now has 4,096 seeded valid-frame mutations plus 256 random inputs; encrypted RTP-to-PCM still matches reference PCM.
- **Emulator evidence:** RPCS3 is running on the Mac, but the UI bridge reports the desktop locked. The accessible app log is stale v0.1.15 and ends at NetCtl address timeout; it does not validate v0.1.20.
- **Next:** Restore UI access, install and run the exact v0.1.20 package in RPCS3, capture a fresh capped log, and follow the first failing layer. Do not treat this host fuzz run as sender compatibility or audible output.
- **Version:** Next package version is 0.1.21 after a runtime-directed fix. Keep boot plugin configuration unchanged; game-audio coexistence remains a separate feasibility gate.
