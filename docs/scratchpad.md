# Active work

- **Current task:** Determine whether bounded Content-Type validation lets RPCS3 finish synthetic ANNOUNCE, then continue into SETUP/RECORD/RTP.
- **Latest replay:** AIRPLAY02 v0.1.27. RPCS3 startup, route, listeners, and OPTIONS 200 pass. ANNOUNCE SDP and encrypted offer fields parse (type 1, RSA key and IV); no Content-Type-check/validation/RSA/response event followed.
- **Latest package:** v0.1.27, 3,231,920 bytes, SHA-256 `0a3d1c409fb1044231256090cc3693da3e1aaa5abddd5bc6e00f29a9aa407b6c`, source `eebd588f4d4e7baa8da95d5f4127b9155d90b858`.
- **In progress:** v0.1.28 replaces `strlen` on the fixed-size Content-Type field with a bounded scan and adds events/assertions. Full Lima ASan/UBSan and PPU checks pass on the candidate diff.
- **Unverified:** RTP, ALAC on RPCS3, GameOS output audibility, iPhone interoperability, visible display, controller exit, and physical PS3. mDNS still fails in RPCS3; desktop remains locked.
- **Constraints:** Keep production RTSP at 5000; RPCS3 uses 5001. Keep boot plugin and game-audio work separate.
