# Active work

- **Current task:** Verify the corrected full package pipeline with AIRPLAY02 v0.1.8, then diagnose RPCS3's socket-bind boundary.
- **Verified:** v0.1.7 (3,225,904 bytes; SHA-256 `fc8899dd3f62cbd9678bf3f27bdb542770fe9516ad142d37235ea3f958e09674`) installed in RPCS3 and reached APP_START. It then failed RTSP bind with value 112 and logged APP_STOP. RPCS3 network status was Disconnected; causality remains unproven.
- **In progress:** Extract/verify the unfinalized package before NPDRM finalization; validate finalized envelope separately. Version 0.1.7 is consumed; next package is 0.1.8.
- **Boundary:** Do not deploy to physical console or modify boot plugins. Do not change RPCS3 network settings without authorization. RPCS3 evidence is not hardware evidence; no AirPlay playback claim until the iPhone connects and streamed audio is heard.
