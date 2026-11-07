# Standalone PS3 build and package pipeline

## Canonical path

Use the existing x86_64 Lima guest with the pinned PS3DK v0.27.0 PPU toolchain, the recorded PSL1GHT checkout, and the matching Mbed TLS PPU library. The legacy GCC 7.2/binutils 2.22 script is an ABI comparison experiment only; it is not the release build path.

The v01.01 reference decrypted by RPCS3 is an ELF64 big-endian PPC64 `ET_EXEC`, CellOS OSABI `0x66`, entry `0x10000000`. Its first four `PT_LOAD` flags are `0x400005`, `0x600006`, `0x400004`, `0x600006`, followed by the PS3-specific zero-sized segment. The v0.1.6 legacy-linker ELF had the generic first flags `0x5` and `0x6`. Their shared layout is evidence that the PS3DK path preserves the old artifact's executable contract; it does not prove the complete v01.01 build or runtime is reproducible.

The release path is:

1. Run `scripts/build-lima-crypto.sh` for portable protocol/audio tests and PPU object checks.
2. Export the pinned toolchain paths, exact source revision, and next unused `APP_VERSION`.
3. Run `scripts/build-lima-standalone-receiver.sh`. It builds Mbed TLS, clean-builds the receiver, and validates the resulting ELF against the v01.01 ABI/layout reference. It emits a build manifest with the source revision, compiler/linker versions, PSL1GHT revision, and ELF hash.
4. Run `scripts/package-lima-standalone-receiver.sh`. It verifies the ELF again, creates the NPDRM SELF and matching SFO/content ID, builds the package under a relative content-ID directory, extracts and checks the unfinalized package (including APP_VER and asset bytes), then finalizes a copy with PSL1GHT's package finalization step and validates its envelope. The bundled `pkg` reader cannot extract finalized NPDRM type `0x80000001`, so extraction is deliberately checked before finalization. The script refuses to overwrite a versioned package or intermediate.
5. Install and launch only in RPCS3 for the first gate. Keep physical console deployment separate and require the established recovery procedure and user authorization.

Example, with paths adjusted to the local Lima guest:

```sh
export PS3DK_SOURCE="$HOME/ps3-airplay-host/ps3-sdk-src-v0.27.0"
export PS3DEV="$PS3DK_SOURCE/stage/ps3dev"
export SOURCE_REVISION="<full SHA of the exact source commit copied into Lima>"
export APP_VERSION=0.1.21
# Optional emulator-only probe when the host's port 5000 is occupied.
export RTSP_PORT=5001
bash scripts/build-lima-crypto.sh
bash scripts/build-lima-standalone-receiver.sh
bash scripts/package-lima-standalone-receiver.sh
```

Keep the generated package, build manifest, and test logs outside Git. Record package SHA-256, size, source revision, and emulator outcome in a sanitized report. Never treat matching ELF headers or package extraction as proof of PS3 launch, visible display, AirPlay connection, or audible music.

## Current evidence and limits

- The preserved AIRPLAY02 v01.01 package and the modern PS3DK candidate share the validated ELF header and PS3 `PT_LOAD` flag pattern; they are not byte-identical.
- The v0.1.6 legacy runtime build fails the new layout validator, directly exposing a real ABI/layout regression in that path. This discrepancy is not yet proven to be the sole reason RPCS3 rejected v0.1.6.
- The PS3DK PPU compiler is GCC 16.1.0 in the current Lima toolchain; compiler and linker versions are captured per build.
- v0.1.7 (source revision `95f5924`) installed in RPCS3 and reached `APP_START`, `SCREEN_READY`, controller/network module initialization, and crypto initialization. It then logged RTSP bind error value `112`, `RECEIVER_OPEN_FAILED`, and `APP_STOP`. This is a successful loader/startup boundary followed by a network/socket failure; it does not establish display output or AirPlay operation.
- The RPCS3 network configuration was observed disconnected during this run. The relationship to bind error 112 is plausible but unproven; do not change emulator network settings without authorization. Physical PS3, visible display, clean XMB exit, iPhone RAOP, and streamed-audio results remain separate tests.

# v0.1.8-v0.1.13 follow-up

- v0.1.8 was built from the corrected PS3DK/Lima pipeline. It installed and reached `APP_START` in RPCS3, then failed RTSP bind with error value 112. RPCS3's network status was observed as disconnected; causation is unknown. The app stopped before receiver readiness.
- v0.1.9 moves the existing bounded local IPv4 readiness check ahead of receiver socket creation. In RPCS3, this produced `NET_IP_TIMEOUT` while networking was disabled and `NET_IP_ROUTE_READY` after enabling networking; the RTSP bind still failed with value 112.
- A wildcard host TCP listener occupied 5000 during the connected run. This is a port-conflict hypothesis, not a proven cause. The v0.1.10 build input permits a controlled test on 5001; the normal RTSP port remains 5000, and mDNS publishes the actual listener port.
- v0.1.10 bound RTSP on 5001 and RTP UDP on 6000-6002 in RPCS3, then logged `MDNS_FAILED`. Audio initialized and the diagnostic tone completed. No AirPlay discovery/session or music was established.
- v0.1.11 visibly rendered the supplied background and `PRESS START TO EXIT` in RPCS3 without blue bands or `DISPLAY ACTIVE`. It still logged `MDNS_FAILED` without a substage. The screen result is emulator evidence; the user's report separately confirms the app opens.
- v0.1.12 adds a numeric mDNS failure stage and errno to the privacy-safe `MDNS_FAILED` event. RPCS3 logged stage 4 (UDP bind) and errno 112 (`EADDRINUSE`). A second run with Lima stopped still failed: the host's system mDNS responder owns wildcard UDP 5353, and a wildcard bind probe returned `EADDRINUSE`. RPCS3 mDNS cannot bind on this host; this says nothing about native PS3 port availability.
- v0.1.13 was built from pushed source revision `70fa6654bd2d092a5f674dd526c05a2e5ccd4703` with the normal PS3 RTSP port 5000. All Lima host tests, PPU objects, ABI validation, PKG extraction, SFO/content-ID, asset equality, and finalized envelope checks passed. The package is 3,224,944 bytes; SHA-256 `f7d076e32520a552349ebb973894409d66fd429ff255d0919c66401fabe9f1e9`. Its RPCS3 and physical-console launch gates remain pending.
- v0.1.14 includes the best-effort SO_REUSEPORT request. The PS3DK header defines the option, and the PPU build compiled it. A host socket probe bound UDP 5353 and received multicast while the system mDNS responder remained active. RPCS3 installed and launched v0.1.14, but its fresh app log reached `NET_IP_TIMEOUT` before receiver startup; mDNS sharing was not exercised in the emulator. The physical PS3 remains untested for this change.
- v0.1.14 was built from source revision 6d9f28d0f079c445b96203695ea1912e8deed887. The 3,225,008-byte finalized PKG passed ELF, content ID, SFO, assets, and envelope validation; SHA-256 `5994cf4828976245f530b1c90d180fd4f0e80abddcfd0d09d3780c351ad006c1`.
- v0.1.15 was built from source revision `99abc5b`, including bounded RTSP FLUSH propagation through RTP and PCM buffering. The 2,758,576-byte CellOS PPC64 ELF validated against the v01.01 reference. The finalized PKG is 3,225,648 bytes; SHA-256 `230a2df0db8a7e561f3b048872c1a714ed0eb58f4b741587ee961f71834ffa59`. Package extraction, AIRPLAY02/APP_VER, content ID, assets, and finalized envelope checks passed.
- RPCS3 installed and booted v0.1.15. Its fresh log reached APP_START, SCREEN_READY, CONTROLLER_READY, NETCTL_READY, then `NET_IP_TIMEOUT` and APP_STOP result 1. RPCS3's own network layer reported no available address. No receiver sockets, mDNS, tone, or FLUSH-over-RAOP behavior was reached. The emulator's Internet setting is currently Disconnected; connected-route retesting is tracked separately from physical PS3 deployment.
- v0.1.16 was built from pushed source revision `0a204b04364803e2af9fcc8adf2f7420751ac83f`, including `SET_PARAMETER` volume/mute and a bounded PCM gain ramp. PS3DK v0.27.0 PPU + PSL1GHT produced a 2,780,096-byte CellOS PPC64 ELF, SHA-256 `968f9c3238e7c8d4479c0191452f378f4705d7d6585c68364cfcd7d792682f2a`; compiler 16.1.0, linker 2.42, PSL1GHT revision `dd4e8c9be744e3273d665c393fa6d97a87402e2e`. Final AIRPLAY02 package: 3,231,088 bytes, SHA-256 `3c69bff7ab626c433728ca131bc04ab0adbc117b8845d88c748e1dad1968bd3c`. ELF, APP_VER/SFO, content ID, five package items, background/icon assets, pre-finalization extraction, and finalized NPDRM envelope passed.
- v0.1.17 was built from pushed source revision `4a06c2a`. It removes the remaining text overlay from the standalone display. The 2,780,064-byte CellOS PPC64 ELF passed the ABI validator (SHA-256 `5e80257671409775d98884c7101acf15d7e116448498ee8c69148843a96896dd`). The 3,230,256-byte finalized AIRPLAY02 package passed SFO/APP_VER, content ID, asset equality, pre-finalization extraction, and NPDRM envelope checks (SHA-256 `2095046e20716d9f2768c847691515f1d1e922e5fbd96e15829f92c45816cf23`). Full Lima host tests and PPU object checks passed.
- v0.1.17 has not been installed or launched in RPCS3. The UI remains unavailable, so its visual output and runtime behavior are untested. The saved Internet setting was not changed.
- v0.1.18 adds five-second numeric PCM telemetry for ring occupancy, frames queued/consumed, and frames submitted to GameOS audio blocks. Full Lima host tests, PPU objects, the canonical 2,780,120-byte CellOS PPC64 ELF, and finalized package checks pass. ELF SHA-256 `ba7450441a8e2def2a1f55e17d523630d3d5af256520f6769154a347b09d3759`; 3,230,448-byte PKG SHA-256 `5548fde98bc08949e4f3a6739a95d5b3dce7dea0458131ec693c01be97766ae0`. Block submission is not audible-output evidence.
- v0.1.18 has not been installed or launched in RPCS3. UI access remains unavailable, no emulator setting changed, and no runtime log event is verified.
- Current package candidate is v0.1.21, which corrects the RAOP TXT server-version value. Both standalone build and package scripts default to 0.1.21. Every generated package consumes its version even when it fails a test.
