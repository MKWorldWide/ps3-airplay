# Lessons learned

## Instrumentation can mask receiver-test stalls

- **Problem:** A fresh receiver loopback run timed out while waiting for its first timing probe when run directly, but completed under `strace`.
- **Root Cause:** Not established. The trace-assisted run shows the loopback scenario can complete; instrumentation changes timing enough to mask the direct-run stall. This does not prove whether the cause is a test race, receiver scheduling, or the emulated host.
- **Fix:** No implementation fix yet. Record direct execution as unresolved and do not count the trace-assisted run as a normal passing test.
- **Prevention Rule:** Repeat timing-sensitive network tests without instrumentation; add an explicit test deadline and collect narrowly scoped diagnostics before claiming a pass.

## Discovery does not prove the advertised address is reachable

- **Problem:** The iPhone listed the receiver but reported that it could not connect; the available old log contained no RTSP accept event and may not match that attempt.
- **Root Cause:** Not established. Source inspection found the app selected its mDNS A record by routing a UDP socket toward multicast instead of using the PS3 network controller's assigned IPv4 address. Discovery can succeed while a wrong address or another listener problem still prevents TCP connection.
- **Fix:** Version 0.1.2 uses NetCtl's IP address only after `IPObtained`, records privacy-safe listener readiness/accept/method/status events, and keeps service/protocol compatibility changes separately testable.
- **Prevention Rule:** Treat the NetCtl change as a likely defect correction, not a proven cause; tie a new hardware log to the exact build and use the next failure stage to choose the next change.

## Make the bounded log rollover visible

- **Problem:** A capped diagnostic file could silently restart, making a retrieved log appear to begin in the middle of a session.
- **Root Cause:** The logger enforced its byte limit but did not mark a rollover.
- **Fix:** Add a `LOG_WRAPPED` event with the previous byte count, and test both the cap and marker.
- **Prevention Rule:** When bounded diagnostic history rolls over, preserve a compact explicit boundary event without exceeding the configured cap.

## Blank foreground screen and no clean app exit

- **Problem:** The receiver played its five-second tone, but the PS3 screen stayed black and the user needed a long power-button press to return to XMB. The iPhone listed the receiver but could not connect.
- **Root Cause:** Source inspection showed the standalone app had no RSX/video initialization and did not poll controller input. This explains why the app had no implemented screen or START-button exit; it does not prove the exact hardware cause of the black output or the separate RAOP connection failure.
- **Fix:** Add a bounded-size RSX background framebuffer, a visible START exit prompt, controller polling with ordered cleanup, and screen-stage log codes. Keep the in-game plugin and game mixing questions separate.
- **Prevention Rule:** Hardware readiness requires visible foreground UI, a documented controller exit, startup/shutdown log events, and a tested recovery path before extended network testing.

## A display-ready event is not proof of visible output

- **Problem:** A capped log snapshot recorded `SCREEN_READY`, while the user had reported a black display; package identity for that snapshot is unknown.
- **Root Cause:** The event confirms the RSX setup path returned after a flip, but it does not verify the selected framebuffer, pixel format, signal, or actual visible image.
- **Fix:** Keep hardware rendering marked unresolved; prime scanout with the upstream sample's spare-buffer-first sequence and draw a bright `DISPLAY ACTIVE` overlay so a new run can distinguish valid output from the intentionally dark background. Require a visible screen result tied to the candidate before calling the fix successful.
- **Prevention Rule:** Treat graphics initialization logs as stage evidence only; require a human-visible image confirmation before calling display output successful. A similar upstream PSL1GHT/Evilnat report is evidence of a compatibility risk, not proof of this project's root cause.

## Bind the EBOOT NPDRM content ID to the package

- **Problem:** The first standalone package installed but failed to launch with PS3 error `80029530`; no application startup log was created.
- **Root Cause:** Likely a content-ID mismatch: the package script wrapped the ELF with generic `fself` and passed a separate content ID only to the PKG builder. PSL1GHT's CEX package rule uses `make_self_npdrm` with the same content ID as the PKG and `PARAM.SFO`; corrected hardware launch is still needed to confirm.
- **Fix:** Generate `EBOOT.BIN` with `make_self_npdrm <ELF> <EBOOT.BIN> <CONTENTID>` and include the SDK's default icon in the package root.
- **Prevention Rule:** Treat the EBOOT, SFO title ID, PKG content ID, and icon as one package contract; validate the extracted contents before transfer and keep launch as a separate hardware gate.

## Keep queue-overflow fixtures distinct from duplicates

- **Problem:** The RTP queue test expected a retransmission of sequence 217 to count as a full-queue drop.
- **Root Cause:** Sequence 217 was already present in the full queue, so the implementation correctly classified it as a duplicate before checking capacity.
- **Fix:** Use sequence 10000 for the overflow case so the fixture exercises capacity rejection, not duplicate handling.
- **Prevention Rule:** Queue-capacity tests must use an item that is not already queued; test duplicate rejection separately.

## Separate validation claims

- **Problem:** A successful build or package can be mistaken for working console behavior.
- **Root Cause:** Cross-compilation and structural checks do not exercise a PS3 loader, runtime, network stack, or audio output.
- **Fix:** Report host tests, cross-builds, packaging, hardware load, sender interoperability, and playback as separate gates.
- **Prevention Rule:** State exactly which environment executed each check.

## Bound network work by total deadlines

- **Problem:** Fragmented requests can hold a listener longer than a per-read timeout suggests.
- **Root Cause:** A sender can trickle data often enough to reset an idle timeout.
- **Fix:** Apply an overall deadline to assembling each request and bound sessions and queues.
- **Prevention Rule:** Put limits on total work and lifetime, not only individual reads.

## Treat packet recovery as a trust boundary

- **Problem:** A valid retransmission request does not prove a returned packet is safe or useful.
- **Root Cause:** Responses must match the expected peer, port, outstanding sequence range, inner framing, and queue bounds.
- **Fix:** Validate each condition independently and reject malformed, duplicate, late, or unsolicited packets.
- **Prevention Rule:** Test positive and negative paths for each separately enforced condition.

## Keep diagnostic logging privacy-conscious

- **Problem:** Raw protocol logs can expose addresses, metadata, credentials, or cryptographic data.
- **Root Cause:** Logging full requests and payloads is convenient during debugging but unsafe to share.
- **Fix:** Log bounded event codes and numeric counters; keep content and identifiers out.
- **Prevention Rule:** Include privacy assertions in logging tests and review any attached logs before sharing.

## Verify third-party tooling interfaces

- **Problem:** Tools with similar names may use incompatible inputs or require different host prerequisites.
- **Root Cause:** Upstream packaging and interfaces differ across projects and versions.
- **Fix:** Pin tool versions, verify checksums, inspect expected inputs, and validate generated artifacts.
- **Prevention Rule:** Test each bootstrap and packaging stage independently before describing the setup as reproducible.

## Keep resident audio mixing as a separate question

- **Problem:** A system music toggle or successful resident module build could be read as proof of arbitrary PCM injection during games.
- **Root Cause:** Those mechanisms do not establish the same audio route or runtime context.
- **Fix:** Keep standalone output, resident lifecycle, and game coexistence as distinct tests.
- **Prevention Rule:** Report game-audio results only for titles actually tested.
