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
export APP_VERSION=0.1.11
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

# v0.1.8-v0.1.11 follow-up

- v0.1.8 was built from the corrected PS3DK/Lima pipeline. It installed and reached `APP_START` in RPCS3, then failed RTSP bind with error value 112. RPCS3's network status was observed as disconnected; causation is unknown. The app stopped before receiver readiness.
- v0.1.9 moves the existing bounded local IPv4 readiness check ahead of receiver socket creation. In RPCS3, this produced `NET_IP_TIMEOUT` while networking was disabled and `NET_IP_ROUTE_READY` after enabling networking; the RTSP bind still failed with value 112.
- A wildcard host TCP listener occupied 5000 during the connected run. This is a port-conflict hypothesis, not a proven cause. The v0.1.10 build input permits a controlled test on 5001; the normal RTSP port remains 5000, and mDNS publishes the actual listener port.
- v0.1.10 bound RTSP on 5001 and RTP UDP on 6000-6002 in RPCS3, then logged `MDNS_FAILED`. Audio initialized and the diagnostic tone completed. No AirPlay discovery/session or music was established.
- Current package version is 0.1.11. Every generated package consumes its version even when it fails a test.
