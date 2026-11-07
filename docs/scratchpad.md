# Active work

- **Task:** Build and prepare the isolated native PS3DK launch-control app.
- **Source:** Branch `docs/v012-preflight-review`, starting commit `4808fad`.
- **AirPlay app:** AIRPLAY02 v0.1.3 remains installed and reportedly fails with `80028F21`; its SFO/EBOOT match the package and the 1,817-byte log is unchanged. No root cause is established.
- **Launch-control artifact:** `build/native-ps3dk-launch-control/ps3-airplay-native-control.pkg`, 192,688 bytes, SHA-256 `7bb8e2fbc12cc3da8f4b91ff0ae85c4dcb105209dba5d45765b1787d4015b6d2`; APP_VER 0.1.0, title ID `AIRTEST01`, content ID `UP0001-AIRTEST01_00-0000000000000000`.
- **Behavior:** cycles bright full-screen colors; START exits; 20-second frame limit; log path `/dev_hdd0/tmp/ps3-airplay-native-launch.log`. RSX polling is bounded. Hardware behavior is unknown.
- **Build finding:** PS3DK's reference sample and our app both failed with the old PSL1GHT linker. Building/installing the PS3DK forked `sprxlinker` fixed the local build. This is a Lima toolchain change; it does not modify PS3 firmware or boot plugins.
- **Next:** Complete privacy/diff review and commit source/docs. Before hardware transfer, ask approval with the exact package filename/hash and rollback steps. No package has been uploaded or installed.
