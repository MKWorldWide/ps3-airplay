# Active work

# Active work

- Current candidate: v0.1.36, built from the v0.1.35 baseline.
- Fresh read-only v0.1.35 evidence: 2,048 received RTP packets, 302 sequence gaps, 209 late packets, 58 resend requests, no recovered packets in retained totals, zero ALAC failures, and 102,200 active-stream PCM underflow frames. Log wrapped; capture does not isolate one exclusive cause.
- Implemented bounded acceptance of valid retransmissions that arrive after request tracking expires but before their sequence is played; added reason counters and a network integration regression.
- Full Lima ASan/UBSan host suite and PPU component compilation pass. Source/documentation commit is next; then build exact v0.1.36 ELF/PKG, validate, run RPCS3 checks, and stage without interrupting the current PS3 session.
- Graphics resume/flicker remains unproven and is not changed speculatively; verify actual XMB transitions on hardware after audio recovery is tested.
