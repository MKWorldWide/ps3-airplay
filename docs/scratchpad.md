# Active work

- **Current task:** Produce and RPCS3-test AIRPLAY02 v0.1.7 using the recovered PS3DK/Lima pipeline.
- **Verified:** v01.01 reference ELF and fresh PS3DK ELF share the expected PS3 executable and `PT_LOAD` layout; legacy v0.1.6 fails the layout gate. Host protocol/audio/log suite and PPU object checks passed in Lima.
- **In progress:** Commit pipeline scripts and docs. Then rebuild from that exact commit with `SOURCE_REVISION`, package v0.1.7 using PSL1GHT finalization and extraction checks, install and launch in RPCS3, and inspect a fresh emulator log.
- **Boundary:** Do not deploy to physical console or modify boot plugins. RPCS3 evidence is not hardware evidence. No AirPlay playback claim until the iPhone connects and streamed audio is heard.
