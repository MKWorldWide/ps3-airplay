# Active work

- **Task:** Continue AIRPLAY02 work from the earlier user-reported startup/tone observation. AIRTEST01 remains parked; do not modify PS3 hardware or boot plugin settings during this RPCS3 pass.
- **Latest candidate:** v0.1.6; 3,183,600 bytes; SHA-256 `c71cf7ee0ec314496971ea32d831c05238746eada6796b017092b3873a2ecc33`.
- **v0.1.5 result:** Correct relative PKG paths and install identity validated. RPCS3 installed it, then rejected its ELF because OS/ABI was generic System V.
- **v0.1.6 result:** Post-link patch sets `EI_OSABI=0x66` (CellOS LV2). RPCS3 installed it but still rejected the executable as an invalid ELF type before application startup; no app log was written.
- **Build evidence:** Lima cross-build and package extraction/SFO/content checks pass. No host regression suite was run in this pass. No PS3, display, tone, iPhone, RTP, or playback test occurred.
- **Next:** Compare SELF inner ELF type, flags, and headers against an artifact that demonstrably starts in RPCS3; add the required PS3 post-link transformation, then increment to v0.1.7 before building. Keep 0.2.0 for a stable diagnostic app with settings/logging controls and 0.3.0 for stable verified AirPlay audio.
