# Active work

- **Task:** Resume AIRPLAY02 development from the earlier user-observed app-start/tone result and work toward a real iPhone RAOP session. AIRTEST01 v0.1.0 is parked.
- **Current candidate:** AIRPLAY02 v0.1.4, locally built with the 2020 PSL1GHT runtime, route-first IPv4 selection, a five-second NetCtl wait/fallback, and a log version marker tied to `APP_VERSION`.
- **Validation:** PPU ELF and PKG build/extraction passed. Package size 3,183,584 bytes; SHA-256 `0c1e3c74eea71f7b5829201673f4dd5211bcbf704ab00cb55e1b41cb87020ee4`. Host crypto/session/SDP/mDNS, listener/log, ALAC, PCM, and tone tests passed. Direct receiver timing-probe loopback hung again; unresolved.
- **Hardware:** Prior AIRPLAY02 v0.1.2/v0.1.3 and AIRTEST01 v0.1.0 attempts returned user-reported `80028F21`; the old AIRPLAY log did not change. No cause is established. v0.1.4 has not been transferred, installed, or run.
- **Next:** Before deployment, bound RSX waits and keep a responsive early START exit path. The maintainer has authorized package installation without another approval request; once the receiver build is safe to test and the user resumes, preflight the console, stage/install it, then test launch, visible screen, tone, and clean exit before any iPhone connection attempt. Do not alter boot plugin configuration.
- **Current boundary:** This goal is paused by the user. No v0.1.4 transfer/install has happened; do not continue hardware work until the user resumes.
