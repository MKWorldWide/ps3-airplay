# AIRPLAY02 v0.1.9 network boundary test

**Date:** 2026-11-07 06:55 EDT
**Source revision:** `b1ebacb`
**Package:** AIRPLAY02 v0.1.9, 3,225,904 bytes
**SHA-256:** `8e523dcb3e12135eb0b097cc383b397060f79f18ab791fa19b9a3e2bdba6d870`
**Test boundary:** Lima build plus RPCS3 only; no physical console changes.

## Results

The host suite passed crypto, session, SDP, mDNS, RTSP/RTP loopback, capped logging, ALAC decoding, PCM/resampling, diagnostic tone, and PPU object checks. The 64-bit big-endian PPC64 ELF passed the project ABI/layout validator. The finalized NPDRM package installed in RPCS3 and the emulator title list showed AIRPLAY02 v0.1.9.

With RPCS3 Internet status Disconnected, the fresh app log showed `NET_IP_WAIT_START`, `NET_STATE_WAIT`, `NET_IP_TIMEOUT`, and `APP_STOP` after five seconds. This verified the bounded startup failure path.

The local RPCS3 Internet status was then set to Connected for a second test; PSN remained Disconnected and UPnP remained disabled. The fresh app log showed `NET_IP_ROUTE_READY` and `CRYPTO_READY`, followed by `RTSP_BIND_FAIL` value 112, `RECEIVER_OPEN_FAILED`, and `APP_STOP`.

At the time of the connected run, a wildcard host listener occupied TCP 5000; no host listeners were found on UDP 6000-6002. A port collision is plausible but not proven as the source of the guest error. Visible display output, tone, mDNS, iPhone connection, RTP, ALAC stream decode, PCM playback, and music were not demonstrated.

## Next test

Build v0.1.10 with the new test-only `RTSP_PORT=5001` input and check whether the TCP listener, mDNS service, and RTP UDP sockets become ready. The default receiver port remains 5000. Restore the emulator's previous Disconnected setting after the local network test series.
