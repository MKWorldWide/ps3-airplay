# RPCS3 Network and RTSP Probe

**Timestamp:** 2026-11-07 12:15 EST (America/New_York)

## Test identity

- **Test ID:** RPCS3-NET-25
- **Package:** AIRPLAY02 v0.1.25, 3,231,152 bytes
- **Package SHA-256:** `420971af3601d0a9813d4853f389f7e4ceb1c9f4a6077e27fcc3b81b292cbcd4`
- **ELF SHA-256:** `848b698f2feaa832cddd88f1610452174a42018cdd0a126ca9d35a5abd724dde`
- **Source revision:** `a59aa937401d432b5cc6f740eeda15bf0db091ba`
- **Build:** PS3DK v0.27.0 PPU + PSL1GHT; emulator-only `RTSP_PORT=5001` override. Normal builds retain port 5000.

## Results

| Check | Result | Observation |
|---|---|---|
| Package/identity validation | PASS | SFO identifies AIRPLAY02 v0.1.25; finalized NPDRM package and extracted payload validate. |
| Application startup | PASS | Fresh log records `APP_START`, `PKG_VERSION_0.1.25`, display initialization, controller initialization, and network initialization. |
| NetCtl route | PASS | With RPCS3 Internet set to Connected, app records `NET_IP_ROUTE_READY`. When disconnected, v0.1.24 instead timed out before receiver startup. |
| RTSP/RTP socket readiness | PASS | RTSP TCP 5001 and RTP UDP 6000–6002 report ready. TCP 5000 was occupied on the host, causing v0.1.24 to fail at bind. |
| mDNS | FAIL | `MDNS_FAILED` stage 6, error 125: multicast interface setup failed. No discoverable service was established in this run. |
| Diagnostic tone | PARTIAL | App log records five-second tone start and completion. Audible output was not observed. |
| RTSP OPTIONS | PASS | Local probe receives `RTSP/1.0 200 OK`; fresh log records connect, OPTIONS, status 200, peer close, and disconnect. |
| Display pixels / controller START | NOT VERIFIED | The RPCS3 desktop remained locked. `SCREEN_READY` only records initialization. |
| iPhone / RTP audio / decode / streamed music | NOT RUN | No AirPlay sender session was attempted. |
| Physical PS3 | UNCHANGED | No console files, settings, or boot plugin configuration were modified. |

## Procedure and evidence

1. Verified the v0.1.24 package and discovered its EBOOT in the extracted payload had harmless trailing zero padding; the package stage and extracted payload match over the full staged EBOOT length. The direct RPCS3 test uses the package's extracted title structure.
2. Preserved the existing RPCS3 AIRPLAY02 title files and app log locally before staging v0.1.24. The user's authorized RPCS3 network setting was changed from Disconnected to Connected and its prior config was backed up locally.
3. v0.1.24 obtained a route but failed TCP 5000 bind. Host port inspection showed another host process owned that port. No unrelated process was stopped.
4. Built and validated v0.1.25 using the repository's existing `RTSP_PORT=5001` override, then copied the validated package extraction into RPCS3 title data. The exact package file remains preserved locally.
5. Started RPCS3 headlessly, inspected the fresh capped app log, confirmed TCP/UDP listeners, and sent one localhost OPTIONS request. The application answered 200. The emulator was then stopped; the log did not record the app's own clean-exit event.

The raw emulator/app logs remain local and are not included in this public report. No package install through the graphical installer occurred; application files were staged from the validated extraction because the GUI was unavailable.

## Conclusion

This pass verifies the standalone app can acquire an RPCS3 route, open its RTSP/RTP listeners on a free host port, complete the diagnostic-tone code path, and answer a basic RTSP request. It narrows the mDNS failure to the emulator multicast-interface step. It does not verify visible pixels, audible output, an iPhone connection, RAOP negotiation, RTP reception, ALAC decode, PCM rendering, or Spotify playback.

## Next corrective action

When desktop access is available, inspect the v0.1.25 screen and exercise controller START. Then pursue the emulator multicast-interface limitation without changing host-wide mDNS services, or test discovery on the physical PS3 after its safe recovery and deployment gates are met. Continue with a controlled ANNOUNCE/SETUP/RECORD test before claiming a full AirPlay session.
