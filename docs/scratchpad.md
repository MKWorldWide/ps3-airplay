# Active work

- **Task:** Validate the isolated native PS3DK launch-control app on hardware after separate install approval.
- **Source:** Branch `docs/v012-preflight-review`, commit `dd8225b` contains the app and build report.
- **AirPlay app:** AIRPLAY02 v0.1.3 remains installed and reportedly fails with `80028F21`; its SFO/EBOOT match the package and the 1,817-byte log is unchanged. No root cause is established.
- **Launch-control artifact:** `build/native-ps3dk-launch-control/ps3-airplay-native-control.pkg`, 192,688 bytes, SHA-256 `7bb8e2fbc12cc3da8f4b91ff0ae85c4dcb105209dba5d45765b1787d4015b6d2`; APP_VER 0.1.0, title ID `AIRTEST01`, content ID `UP0001-AIRTEST01_00-0000000000000000`.
- **Staging:** After approval, uploaded as a new file at `/dev_hdd0/packages/ps3-airplay-native-launch-control-0.1.0.pkg`; downloaded-back bytes, size, and hash match exactly. No installation or launch has occurred.
- **Behavior:** cycles bright full-screen colors; START exits; 20-second frame limit; log path `/dev_hdd0/tmp/ps3-airplay-native-launch.log`. RSX polling is bounded. Hardware behavior is unknown.
- **Build finding:** PS3DK's reference sample and our app both failed with the old PSL1GHT linker. Building/installing the PS3DK forked `sprxlinker` fixed the local build. This is a Lima toolchain change; it does not modify PS3 firmware or boot plugins.
- **Next:** Ask separate approval before installing through XMB Package Manager. AIRPLAY02 remains separate; if AIRTEST01 is installed and causes problems, uninstall only AIRTEST01 through XMB Package Manager. Then test launch, color visibility, START exit, timeout, and log independently.
