# Active work

- **Current task:** Get a current receiver build into RPCS3 and use fresh evidence to continue AirPlay debugging.
- **Source/artifact:** AIRPLAY02 v0.1.20 from commit `8a4e641d114e1a613421b9ffb51f8615ad851827`; finalized PKG SHA-256 `bfaa3f960de39daf95c2ab405bd361a031ed92bd35c76a99e667d4ffe8d19e87`. Local ignored copy: `work/builds/0.1.20/`.
- **Validation:** Full Lima host suite under ASan/UBSan halt-on-error, 256 malformed ALAC inputs, encrypted RTP-to-PCM reference test, PPU link, ELF validation, and PKG/SFO/content/assets/NPDRM checks pass.
- **Runtime boundary:** Latest Computer Use reports the Mac locked. v0.1.20 is not installed/run in RPCS3. No current physical PS3 test or audible AirPlay playback.
- **Next:** Restore desktop access, install/run this exact PKG in RPCS3, capture a fresh version-matched log, then proceed according to the earliest failing receiver layer. Physical-console changes remain separately gated.
- **Version:** Next package is 0.1.21 after a distinct implementation pass. Keep boot plugin configuration unchanged; game audio mixing remains unproven and out of scope for this standalone milestone.
