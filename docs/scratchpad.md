# Active work

- **Current task:** Diagnose v0.1.10 mDNS failure and continue toward an iPhone RAOP session.
- **Verified:** v0.1.10 passed the Lima host suite and package validation, then installed/launched in RPCS3. It bound TCP 5001 and UDP 6000–6002, reported `MDNS_FAILED`, opened audio, and completed the five-second tone. Host TCP 5000 remains occupied. No mDNS discovery or AirPlay session occurred.
- **Current change:** Remove the blue top/bottom bands and `DISPLAY ACTIVE` text from the app screen. Keep the public background image and white START-to-exit prompt. Advance the next package to v0.1.11.
- **Next:** Commit/push the display cleanup, build v0.1.11 with port 5001, then add stage-specific privacy-safe mDNS error diagnostics and isolate the failing socket/multicast operation. Restore RPCS3's prior Disconnected mode after local testing.
- **Boundary:** No physical PS3 writes or boot plugin changes. RPCS3 evidence is not hardware evidence; do not claim AirPlay playback until a real session and audible streamed audio are confirmed.
