# Active work

- **Current task:** Diagnose iPhone discovery-without-connect while retaining the screen and safe-exit diagnostics.
- **Evidence:** The older 350-byte log has no RTSP accept event, but its package identity is unknown and may not correspond to the failed tap. Discovery does not prove that the advertised service address is reachable.
- **This pass:** Version 0.1.2 uses the address reported by PS3 NetCtl after `IPObtained`, adds verbose bounded RTSP/socket/SDP/RTP diagnostics, adds AirPlay response headers and PAUSE/resume, and builds/inspects locally. Candidate PKG SHA-256 is `923aebf6eca5693b21b494a9a4225c36629f13e8b316b2825d5deb3763226ebf` (3,225,712 bytes).
- **Next:** Request separate approval for this exact artifact before replacing the staged PKG. After XMB install, verify visible display and START exit, attempt iPhone connection once, then retrieve and review the capped log.
