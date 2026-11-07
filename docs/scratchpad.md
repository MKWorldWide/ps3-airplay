# Active work

- **Task:** Diagnose the repeated AIRPLAY02 launch failure and choose the next SDK comparison.
- **Source:** Branch `docs/v012-preflight-review`, latest pushed commit `4ca1e35` before this hardware result.
- **Installed build:** AIRPLAY02 v0.1.3; user reports same XMB error `80028F21` as v0.1.2.
- **Identity/log evidence:** Installed SFO and EBOOT match v0.1.3 package extraction. The 1,817-byte capped log is byte-for-byte unchanged from the earlier run. No new startup event; this does not prove the app failed before `main()`.
- **Candidate:** v0.1.3 uses the 2020 PSL1GHT runtime. That change did not fix launch. Staged v0.1.2 remains available at `/dev_hdd0/packages/ps3-airplay-receiver.pkg`.
- **Additional logging defect:** `main.c` still writes `PKG_VERSION_0.1.2` in v0.1.3; correct this before the next package.
- **SDK alternatives:** Thermal Bench states it uses Sony's official SDK 4.75 in a Windows 7 VM/MSBuild workflow; this requires a licensed environment and a port. Existing Lima has open-source PS3DK, including a native runtime/API distinct from its PSL1GHT front end.
- **Next:** Build a minimal native PS3DK launch control locally. Check whether the user has an authorized Sony SDK/VM before attempting that route. Do not transfer a new package without separate approval.
