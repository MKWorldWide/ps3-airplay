# Active work

- Current pass: remove the automatic five-second diagnostic tone; set app/package label to 0.2.0 and SFO APP_VER to 00.40, continuing the SFO sequence.
- Preserve the previous hardware evidence: v0.1.39 reached encrypted RTP, ALAC decode, and GameOS PCM submission but had reported crackle/dropouts and controller input issues. The app was exited through webMAN and console was at XMB.
- A new production PCM-ring quality test measures 44.1-to-48 kHz resampling. Initial output has exact 48,000-frame count and zero underflow, but high-frequency SNR falls sharply; this is a lead for the next resampler study, not proof of the audible defect.
- Do not modify, transfer, install, or launch anything on the physical PS3 without an explicit user request. Build and host validation only for this candidate.
- Built and validated finalized AIRPLAY02 0.2.0 / SFO 00.40 from commit `b8bab2015551a7044dba26717611e524ed43f4ec`; 3,231,472 bytes, SHA-256 `924a01ef0ef812e1badb8e94011930f5187c4dba91b253e578e16ab72de4f29b`. Local artifact: ignored `work/builds/0.2.0/`.
- Full Lima host/sanitizer suite and PPU component compile passed; standalone CellOS PPC64 ELF and finalized package validators passed. No emulator/hardware installation or launch occurred; physical console remains untouched.
- Next: commit package verification notes, then continue with bounded resampler alternatives and missing RTP-jitter/PCM/controller instrumentation. The measured linear interpolation response is a lead only, not an established cause of audible artifacts.
