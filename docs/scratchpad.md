# Current state

- Isolated lily/fidelity-v027, source `39e918c` built through complete release gate. Production main and physical v0.2.6 remain unchanged.
- v0.2.7/SFO 00.47 candidate retained locally (3,273,456 bytes; hash in fidelity report/manifest); no deployment.
- Measured 20 kHz analytic SNR 4.17→75.07dB; music reference 33.79→63.75dB. FIR adds 0.703 ms delay and 40 KiB table. Fullband transition limits remain documented.
- 350 ms guard passes host resume mocks; physical sub500 ms restoration and PS3 audio service costs remain unverified.
- Latest physical snapshot 26m35s includes five connections, 122 reported concealments, one short reconnect underflow; newest window zero additional concealment/underflow.
- Next: review candidate, obtain explicit hardware-test authorization, compare same source and inspect new aggregate timing counters. Do not auto-deploy.
