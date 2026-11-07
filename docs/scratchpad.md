# Active work

- **Current task:** Make the display test unmistakable and reduce RSX scanout startup ambiguity before resuming AirPlay testing.
- **Evidence:** Previous report and a 350-byte log show a user-heard tone, `SCREEN_READY`, controller/network/audio startup, and a black screen report; the run's package identity is unknown. `SCREEN_READY` is not proof of visible output. A similar PSL1GHT/Evilnat issue exists upstream; this project already uses `-O2`.
- **This pass:** Primed the spare framebuffer before the primary framebuffer, added a vivid `DISPLAY ACTIVE` banner and color bars over the public background, and logs width/height in separate numeric events. Version 01.02 builds and extracts locally; package SHA-256 is `ddedd7d07fcac3554275b331b821741615019ea2d3a69f9fc9965cbdfbaec94a` (3,223,664 bytes).
- **Next:** Review privacy-safe candidate metadata; request exact transfer approval before replacing the staged 01.01 package. Require visible banner and START-to-XMB confirmation before trying AirPlay again.
