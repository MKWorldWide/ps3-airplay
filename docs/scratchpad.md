# Active work

- **Task:** Controlled launch test of the separately validated legacy-runtime candidate; preserve the known v0.1.2 package for rollback.
- **Source:** Branch `docs/v012-preflight-review`; local code/docs changes are not yet committed. Public hypothesis reference: `lucasdaddiego/ps3-health`, whose published result concerns HFW 4.93 + HEN rather than Evilnat.
- **Installed build:** AIRPLAY02 v0.1.2 remains installed and fails XMB launch with `80028F21`; installed EBOOT and SFO match its package.
- **Candidate:** AIRPLAY02 v0.1.3, 3,183,408 bytes, SHA-256 `98d8701ab018df882e30ba28f851a994c195d0a144dafe7c68c77d86d94126aa`. Built from the same receiver sources with PSL1GHT 2020 commit `6e565a70e927f55813babe86dc6f64149535f908`, matching Mbed TLS/GCC 7.2, and validated package contents. Local artifact: `build/lima-legacy-runtime/ps3-airplay-receiver-0.1.3.pkg`.
- **Host evidence:** Eleven existing host test executables passed, including receiver RTSP/RTP loopback. Earlier untraced timing-probe stall remains a flakiness concern. No PS3 test of v0.1.3 has occurred.
- **Next:** Commit and push the reproducible build/package changes and evidence. Then request approval to upload v0.1.3 as a new versioned file under `/dev_hdd0/packages/`; do not replace installed AIRPLAY02 or staged v0.1.2 without approval. Hardware launch, display, tone, START exit, and AirPlay remain unverified.
