# Active audit context

- Investigation completed on isolated `lily/sol61-forensic-audit`; production worktree preserved.
- v0.2.5 remains running; no console mutation or deployment in this audit.
- Strongest reproduced defect: 1,400-byte RTP/AES/ALAC caps reject valid 1,416-byte stereo ALAC. Copied-source 1,536-byte experiment decodes exact reference PCM.
- Snapshot: 33m29s runtime, 6,886 interval concealments (~54.963s assuming negotiated 352 samples), zero reported PCM underflows/ALAC failures. Raw bytes/metadata private under ignored work/forensic.
- Next development pass: shared bounded payload policy + negotiation constraints + oversized telemetry, then >16 recovery splitting/retries. Require controlled comparison; do not deploy from this audit.
- Remaining: quantify actual oversize contribution; clock sync, output deadlines, dim resume and audible quality remain unresolved.
