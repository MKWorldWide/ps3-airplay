# Lessons learned

## A sequence-sorted RTP queue still needs playout-gap policy

- **Problem:** RTP packets were inserted in sequence order, but the player popped the head immediately when the next expected sequence was missing. Retransmitted frames arriving afterward were rejected as late; physical logs showed many RTP gaps and no retained recovery.
- **Root Cause:** Queue ordering alone does not defer playout or give the resend path time to fill a sequence hole.
- **Fix:** Hold a hole for a bounded 80 ms. If the frame arrives, play in order; otherwise insert exact ALAC frame-duration silence. Cap silence concealment at 128 frames and skip a larger sequence discontinuity after the same bounded wait.
- **Prevention Rule:** Every jitter queue must define expected sequence, resend deadline, wrap behavior, loss concealment, and maximum forward jump. Test recovery-before-deadline, late recovery, timeout, wraparound, and pathological jumps. Use real sender timing before changing wait/prebuffer values.

## Emulator transport success is not loss-recovery or sound-quality proof

- **Problem:** RPCS3 successfully handled a 128-packet encrypted RAOP fixture, which could be mistaken for proof of improved dropouts.
- **Root Cause:** That fixture contained no RTP sequence gaps and did not capture sound acoustically.
- **Fix:** Report zero-gap, encrypted negotiation/decode/PCM separately; exercise gaps in deterministic queue tests and a future explicit lossy sender replay.
- **Prevention Rule:** Do not call recovery verified unless a resend fills an induced hole or a timed loss marker is observed; do not claim audio quality without listening evidence.

## Log the failing mDNS setup stage

- **Problem:** RPCS3 v0.1.10 and v0.1.11 logged `MDNS_FAILED` after TCP and RTP sockets became ready, but the event had no failure detail.
- **Root Cause:** The startup helper collapsed socket creation, multicast configuration, and announcement send failures into a single boolean result.
- **Fix:** Preserve a bounded failure-stage enum and errno through cleanup; store only stage and numeric error in the capped diagnostic log. Drop multicast membership when setup fails after joining.
- **Prevention Rule:** Network startup failures should report a privacy-safe stage and numeric platform error at the point of failure.

## Keep a working background image unobstructed

- **Problem:** The diagnostic screen's blue bands, color stripes, and `DISPLAY ACTIVE` label obscured the background after the maintainer reported that the application now opens and the image is visible.
- **Root Cause:** The overlay was retained from the earlier black-screen diagnosis even after visibility became sufficient for the user.
- **Fix:** Remove the bands and active label, keep the background image, and retain a small white START-to-exit instruction.
- **Prevention Rule:** Diagnostic overlays should have a clear removal gate; once the user confirms visible output, reduce the screen to the requested product UI.

## Separate emulator host-port conflicts from receiver failures

- **Problem:** v0.1.9 reached a usable emulated route but still failed the RTSP bind in RPCS3.
- **Root Cause:** Not proven. A wildcard host listener already occupied TCP 5000 during the run; UDP 6000-6002 were free. RPCS3 can expose socket operations to native host networking, so this is a plausible conflict to isolate with a second port.
- **Fix:** Add a configurable RTSP port with a default of 5000; test v0.1.10 in RPCS3 on 5001. Keep the standard receiver port and let mDNS advertise the actual selected listener port.
- **Prevention Rule:** Check host port ownership before interpreting an emulator socket failure as an AirPlay protocol defect. Confirm a suspected conflict with a controlled alternate-port test before calling it the cause.

## Establish network readiness before binding receiver sockets

- **Problem:** v0.1.8 reached application startup in RPCS3 but failed while opening the RTSP listener with numeric error 112. The emulator's network status was disconnected during the run.
- **Root Cause:** Not established. The main routine opened receiver sockets before calling the existing bounded local IPv4 resolution path, so a missing route or pending NetCtl state could not be distinguished before socket setup.
- **Fix:** Move the existing route-first lookup and bounded five-second NetCtl wait before crypto/listener initialization. Preserve the route fallback and emit only state/error codes, never local addresses.
- **Prevention Rule:** Resolve or explicitly time out network readiness before creating receiver listeners. Treat emulator network status and socket errno as correlated observations, not proof of causation.

## Extract the debug package before NPDRM finalization

- **Problem:** The v0.1.7 package was successfully finalized, but the packaging script then failed when the bundled PKG reader tried to extract it.
- **Root Cause:** PSL1GHT's finalizer changes the PKG type to `0x80000001`; this older `pkg` reader can list/extract the debug type but reports `Unsupported Type` for the finalized NPDRM type.
- **Fix:** Verify paths, SFO identity/version, EBOOT, and assets by extracting the unfinalized package. Finalize a copy afterward and validate its package header, type, content ID, and declared size with a small independent checker.
- **Prevention Rule:** Treat package creation, pre-finalization extraction, finalization, and final-package envelope validation as separate stages. Do not interpret a tool's unsupported final type as proof that finalization itself failed.

## Preserve PS3-specific program headers from the known working build

- **Problem:** RPCS3 rejected v0.1.6 before application startup even though its ELF OS/ABI byte had been corrected to CellOS.
- **Root Cause:** The legacy GCC/binutils path produced generic initial load-segment flags (`0x5`, `0x6`) and a different memory layout than the decrypted v01.01 reference. This structural regression is verified; whether it alone caused RPCS3's error remains unproven.
- **Fix:** Make the PS3DK v0.27.0 PPU/Lima path canonical and validate the inner ELF header and PS3-specific load flags before packaging. Keep the legacy path for comparison only.
- **Prevention Rule:** Never use successful compilation or an OS/ABI byte patch as the release gate. Compare executable and program-header structure against the known-good artifact, then verify package install and application launch separately.

## Keep commit privacy checks brief and complete

- **Problem:** Repeating broad manual review made routine commits slow and token-heavy; stopping at the first privacy match also added avoidable human intervention.
- **Root Cause:** The review lacked a compact scan-and-remediate loop with a clear threshold for when human judgment is actually needed.
- **Fix:** Use one pass over the full staged diff and changed files, run whitespace and already-available secret scans, search for local paths/private identifiers/raw logs, and manually inspect matches. Automatically sanitize or redact findings and rerun affected checks. Do not install a scanner solely for a routine commit.
- **Prevention Rule:** Keep every privacy gate active and continue the goal after routine sanitation. Only exclude content and report a narrow blocker when safe remediation requires guessing or could change intended meaning. Summarize findings or unavailable scanners briefly.

## Validate the linked ELF ABI before packaging

- **Problem:** RPCS3 accepted and installed v0.1.4 and v0.1.5 packages but rejected the EBOOT before `main()`. v0.1.5 reached the PPU loader, which reported an invalid ELF OS/ABI.
- **Root Cause:** The linked big-endian PowerPC64 ELF advertised generic System V OS/ABI (`EI_OSABI=0`) rather than CellOS LV2 (`0x66`). A successful cross-link and SELF/PKG extraction did not make the ELF bootable.
- **Fix:** Add a guarded post-link patch that validates ELF64, big-endian encoding, and PowerPC64 before changing only `EI_OSABI` to `0x66`; use a new package version for the fix. v0.1.6 applies it, but RPCS3 still reports an invalid ELF type, so this is a partial correction rather than a launch fix.
- **Prevention Rule:** Inspect the inner ELF header and PS3-specific sections before signing. Treat each loader validation stage separately, and do not call the package fixed until RPCS3 reaches application logging.

## Package entry paths and app boot are separate checks

- **Problem:** An earlier package used absolute entry paths and RPCS3 rejected the archive; corrected relative paths allowed installation but did not guarantee boot.
- **Root Cause:** Package parser acceptance verifies container layout only. It does not verify the PPU executable contract.
- **Fix:** Stage files under the content-ID directory, reject absolute package-list paths, and verify installation and launch independently.
- **Prevention Rule:** Maintain separate gates for archive extraction, SFO identity, EBOOT parse/load, application startup, and functional behavior.

## Keep a diagnostic detour separate from the receiver goal

- **Problem:** Work shifted from AIRPLAY02, which had a user-confirmed startup and five-second tone, to a silent AIRTEST01 v0.1.0 launch-control app. The user then reported the same XMB error and no receiver behavior.
- **Root Cause:** The launch control was intended to isolate PS3DK runtime compatibility, but it displaced the requested receiver path and did not retain the tone or AirPlay capabilities. AIRTEST01 is a separate title and did not replace AIRPLAY02, but pursuing its install was the wrong next priority.
- **Fix:** Park AIRTEST01. Return to AIRPLAY02 and preserve the earlier successful startup/tone as the baseline, while keeping exact package identity uncertainty explicit.
- **Prevention Rule:** A diagnostic app must answer a specific blocking question and must not replace the user's working product path without clear evidence that the experiment is the best next step.

## Preserve the route-based startup fallback

- **Problem:** The receiver source had become dependent on NetCtl already reporting `IPObtained` at the first check, even though the earlier code used a UDP route lookup to determine the local address.
- **Root Cause:** The service-address correction changed startup gating and removed the route-based fallback. Delayed NetCtl readiness could terminate startup before discovery, even if the network route was usable.
- **Fix:** The v0.1.4 candidate tries the earlier route-based lookup first, polls NetCtl for at most five seconds, and retries the route lookup once. The PPU cross-build succeeds; the PS3 runtime behavior is not yet tested.
- **Prevention Rule:** Network readiness should be bounded and recoverable. Keep a known-good address discovery path when adding stricter interface checks, and log the selected path without logging the address.

## Installed package identity does not explain a launch rejection

- **Problem:** The user installed v0.1.2, but selecting AIRPLAY02 returned XMB error `80028F21` and no new diagnostic log appeared.
- **Root Cause:** Unknown. Read-only checks show the installed SFO reports AIRPLAY02 / 0.1.2 and its SFO and EBOOT hashes exactly match the extracted candidate. The old log is unchanged, so it gives no evidence for this attempt. Public error-code references do not establish a cause for `80028F21`.
- **Fix:** No code or console change yet. Record a launch failure, preserve the exact package/log evidence, and inspect the NPDRM SELF metadata and startup contract before creating another build.
- **Prevention Rule:** Verify installed identity and classify launch, early startup, and in-app failures separately. Do not infer a loader cause from an undocumented error or from a stale app log.

## A newer console runtime can fail before application logging starts

- **Problem:** The exact v0.1.2 app files are installed, but XMB returns `80028F21` and the app log is unchanged.
- **Root Cause:** Unknown. An independent PS3 project reports pre-`main()` launch failure for PSL1GHT 2021+ runtime builds on HFW 4.93 + HEN and success with its 2020 runtime build. This is relevant evidence, but it does not establish behavior on Evilnat CFW or isolate which runtime change matters.
- **Fix:** Build a separate v0.1.3 candidate with PSL1GHT commit `6e565a70e927f55813babe86dc6f64149535f908`, rebuilding its crypto dependency with the same GCC 7.2 toolchain. Host tests and package structure pass; hardware launch is pending approval.
- **Prevention Rule:** Preserve the failing package, vary one toolchain axis at a time, and require exact hardware results before calling a compatibility hypothesis a fix.

## Instrumentation can mask receiver-test stalls

- **Problem:** A fresh receiver loopback run timed out while waiting for its first timing probe when run directly, but completed under `strace`.
- **Root Cause:** Not established. The trace-assisted run shows the loopback scenario can complete; instrumentation changes timing enough to mask the direct-run stall. This does not prove whether the cause is a test race, receiver scheduling, or the emulated host.
- **Fix:** No implementation fix yet. Record direct execution as unresolved and do not count the trace-assisted run as a normal passing test.
- **Prevention Rule:** Repeat timing-sensitive network tests without instrumentation; add an explicit test deadline and collect narrowly scoped diagnostics before claiming a pass.

## A stop event does not identify the tested build or exit path

- **Problem:** The current PS3 log records `APP_STOP`, but it lacks a package-version marker and `USER_EXIT_START`.
- **Root Cause:** The earlier build did not write a unique version marker on every run, and the generic stop event is emitted by cleanup regardless of which exit path reached it.
- **Fix:** Preserve the original log, report only allowlisted event summaries, and leave package identity and clean user exit unconfirmed.
- **Prevention Rule:** Include a build/version marker in every startup log and treat `APP_STOP` as cleanup evidence only; require `USER_EXIT_START` plus a user-confirmed XMB return for clean-exit PASS.

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


## Legacy runtime alone does not explain the XMB launch failure

- **Problem:** v0.1.3, rebuilt with the 2020 PSL1GHT runtime, produces the same XMB error as v0.1.2.
- **Root Cause:** Unknown. Installed SFO and EBOOT match the extracted package, but the 1,817-byte application log is unchanged. This points to failure before successful logger startup or a log-open failure; it does not prove the failure is before `main()`.
- **Fix:** Do not repeat the same runtime-only change. Compare a minimal native PS3DK launch control and consider the Sony SDK route only if a licensed toolchain is available.
- **Prevention Rule:** A change that reproduces the same hardware failure is evidence against it being a sufficient fix. Tie each launch attempt to installed-file identity and fresh log evidence.

## PS3DK app post-link must use the PS3DK sprxlinker fork

- **Problem:** Both a PS3DK native launch-control app and the upstream cellGcm sample failed after linking with `elf does not have a prx parameter section`.
- **Root Cause:** Lima's `PS3DEV/bin/sprxlinker` was the older PSL1GHT tool, which does not understand PS3DK's 64-byte `.sys_proc_prx_param` section.
- **Fix:** Built and installed PS3DK's `tools/sprx-linker` fork into the staged toolchain's `PS3DEV/bin` and `PS3DK/bin`; the app and NPDRM PKG then built successfully.
- **Prevention Rule:** When an SDK post-link tool reports missing process parameters, inspect that SDK's build/runtime scripts and ensure host tools match the installed SDK revision before changing application code.

## Attribute RPCS3 mDNS bind failures to the host

- **Problem:** v0.1.12 reached its RTSP and RTP listeners in RPCS3 but failed to bind UDP 5353, preventing discovery.
- **Root Cause:** The same failure reproduced with Lima stopped. The host's system mDNS responder owns wildcard UDP 5353; Lima's host proxy can be an additional holder while Lima runs.
- **Fix:** Keep the system mDNS service untouched and record the RPCS3 limitation. Validate mDNS on the PS3 separately before changing receiver behavior.
- **Prevention Rule:** Isolate emulator networking from host services before changing protocol code; a host bind failure does not establish a target-console failure.

## Reuse generated package assets only after matching their source

- **Problem:** A fresh Lima checkout could not regenerate the diagnostic RGB background because Pillow was not installed in the guest.
- **Root Cause:** The image conversion script has an optional Python dependency that is not part of the pinned PS3DK toolchain.
- **Fix:** Reused the existing generated RGB file only after confirming the source JPEG hashes matched and the copied RGB bytes compared exactly.
- **Prevention Rule:** Keep generated image assets out of source control; when reusing one, verify both its source asset identity and byte-for-byte output. Install the documented Pillow dependency when a new conversion is actually required.

## Probe host socket sharing before changing mDNS architecture

- **Problem:** RPCS3 could not bind UDP 5353 while the host system mDNS responder owned the wildcard port.
- **Root Cause:** SO_REUSEADDR alone did not permit this host's mDNS socket to share the port.
- **Fix:** A temporary socket probe showed SO_REUSEPORT allowed bind and multicast reception. The receiver now requests it best-effort when the platform headers provide it; app-level discovery is not yet verified.
- **Prevention Rule:** Validate the actual platform socket behavior with a bounded probe, then keep the option best-effort and verify target behavior before claiming discovery.

## Keep the RTSP/RTP loopback hang visible

- **Problem:** The full host build script intermittently stopped at the persistent RTSP/RTP loopback test without completing.
- **Root Cause:** Not established. The untraced test exceeded a bounded timeout; the same test passed under strace with LeakSanitizer disabled, so timing or sanitizer interaction is plausible but unproven.
- **Fix:** Ran the rest of the host tests separately and retained the loopback test result as timing-sensitive. The PPU build and package validation proceeded independently.
- **Prevention Rule:** Do not silently skip or count a traced pass as proof the untraced hang is fixed. Keep a bounded timeout and investigate the schedule-sensitive test separately.

## Clear every buffered layer on RTSP FLUSH

- **Problem:** A successful FLUSH could leave pre-seek RTP packets in the bounded receiver queue or kernel UDP socket, and already-decoded PCM/resampler history could still play.
- **Root Cause:** The RTSP handler acknowledged FLUSH without coordinating the receiver's RTP queue with the standalone application's PCM queue.
- **Fix:** On successful FLUSH, clear the RTP queue and retransmission/sequence epoch, drain pending audio datagrams with a fixed packet bound, and increment a generation observed by the app loop. The app clears PCM and interpolation history while preserving cumulative diagnostics. A zero-length UDP datagram is consumed and counted as a datagram rather than prematurely ending the drain.
- **Prevention Rule:** Treat seek/track discontinuity as an end-to-end buffer invalidation across socket, packet queue, decoder output, and resampler state; keep every drain bounded and report when a cap is reached.

## Do not confuse emulator screen startup with receiver readiness

- **Problem:** RPCS3 displayed a transient image while launching v0.1.14, which could be mistaken for a running receiver.
- **Root Cause:** Screen and controller initialization precede network address acquisition; the app can render successfully and then terminate before opening its listeners.
- **Fix:** The fresh capped log was summarized by allowlisted event and numeric fields. It reached `SCREEN_READY`, then `NET_IP_TIMEOUT` and `APP_STOP` result 1, with no crypto, RTSP, RTP, mDNS, or audio-ready event.
- **Prevention Rule:** Use the earliest missing readiness event to locate startup failure. Screen visibility, title launch, network route, listener readiness, AirPlay session, and audio output are separate test gates.

## Verify the emulator's virtual network before changing receiver code

- **Problem:** AIRPLAY02 v0.1.14 and v0.1.15 both stopped before listener setup in RPCS3.
- **Root Cause:** The fresh app log reached `NET_IP_TIMEOUT`; RPCS3 logged `EADDRNOTAVAIL`, and its Internet setting is `Disconnected`.
- **Fix:** No receiver code change. Record the emulator's virtual network state and test one connected run with the setting restored afterward before diagnosing mDNS or RTSP behavior.
- **Prevention Rule:** Confirm that the emulator guest has a virtual address/route before attributing bind or discovery failures to receiver code. Keep emulator networking separate from the physical console network.

## Apply RAOP volume instead of acknowledging it as a no-op

- **Problem:** `SET_PARAMETER` returned success for volume requests while the PCM stream always rendered at unity gain.
- **Root Cause:** Session handling treated GET_PARAMETER and SET_PARAMETER as generic keepalives; the audio output had no gain control.
- **Fix:** Parse bounded `text/parameters` volume values, clamp to attenuation only, map the AirPlay mute sentinel to zero gain, and ramp PCM gain across each output block. Invalid and duplicate volume fields receive a 400 response and do not mutate session state.
- **Prevention Rule:** A successful control response must correspond to an applied runtime change. Cover both protocol state and the actual audio-sample transform with host tests, then verify the output path on hardware.

## Export dependent Lima paths sequentially

- **Problem:** The first full standalone build attempt used `/ps3dk` and `/src` include paths and failed before compiling.
- **Root Cause:** Several shell variables were assigned in one `export` command; expansions used the old, empty parent variables.
- **Fix:** Re-exported `PS3DEV` and `DEPS_ROOT` first, then derived `PS3DK`, `MBEDTLS_INCLUDE`, and the library path in subsequent commands. The same source then linked and passed ELF validation.
- **Prevention Rule:** In shell setup, assign parent environment variables before expanding them into dependent paths; verify include/library paths when a tool reports a root-level path.

## Prepare image assets on the host when Lima lacks Pillow

- **Problem:** The canonical package build stopped before compilation because the Lima guest could not import `PIL`.
- **Root Cause:** The checked-in image preparation script depends on Pillow, available on the Mac host but not installed in the lean guest.
- **Fix:** Ran the unchanged repository image preparation script on the host, verified the expected 2,764,800-byte RGB asset, and copied only that generated asset into the ignored Lima build directory.
- **Prevention Rule:** Keep optional image tooling out of the guest; run the deterministic checked-in asset converter on the host and verify exact dimensions/byte count before packaging.

## Remove redundant diagnostic text after image visibility is confirmed

- **Problem:** The user requested removal of blue framing and the `DISPLAY ACTIVE` label after seeing the application open; the current renderer contained neither item, but still drew a `PRESS START TO EXIT` overlay.
- **Root Cause:** The display implementation retained a diagnostic text overlay even after the supplied image itself was sufficient confirmation.
- **Fix:** Removed the glyph table, text renderer, and remaining prompt draw call so scanout shows the supplied background image without text.
- **Prevention Rule:** Keep the foreground display visually minimal once image visibility has been demonstrated; inspect the actual renderer before removing requested UI elements.

## Distinguish PCM queue flow from audible output

- **Problem:** A PCM ring can accept and consume frames while the GameOS output path still underflows or remains inaudible.
- **Root Cause:** Queue counters, audio-block submission, and physical sound describe different stages of the pipeline.
- **Fix:** Add separately named numeric counters for buffer occupancy, frames queued/consumed, and frames submitted to GameOS blocks; preserve underflow/drop events and explicitly avoid treating submission as playback evidence.
- **Prevention Rule:** Report each audio boundary independently and require a real listening observation for audible playback claims.

## Declare telemetry baselines before wiring session resets

- **Problem:** The first PPU compile failed because new per-session counter baselines were reset at the session boundary but had not been declared in `main`.
- **Root Cause:** The implementation pass added lifecycle reset sites before adding the corresponding local state declarations.
- **Fix:** Declared all three baselines with the other main-loop counters; the repeat PPU application link and ELF validation passed.
- **Prevention Rule:** When adding lifecycle-managed metrics, update declarations, session initialization/reset sites, and emission sites together, then run the strict PPU compile before recording the pass as validated.

## Bound the ALAC frame count before decoder array writes

- **Problem:** AddressSanitizer found a heap-buffer-overflow while decoding a reference frame that reported 353 samples against a negotiated 352-sample maximum. Further review found the 32-bit parser accumulated the declared count in a signed integer, risking undefined behavior for high-bit values before the bound check.
- **Root Cause:** The decoder trusted the frame's in-band sample count while predictor and output arrays were allocated to the SDP maximum. The output-byte capacity check happened after the count was parsed but did not protect those internal arrays.
- **Fix:** Accumulate 32-bit fields as unsigned, then reject zero and over-negotiated sample counts in both mono and stereo branches before narrowing, output-size arithmetic, or sample-array writes. The regression checks the 353-vs-352 boundary and `UINT32_MAX` under sanitizers while retaining the valid encrypted RTP-to-PCM golden path.
- **Prevention Rule:** Treat compressed frame headers as untrusted. Validate in-band sample counts against negotiated bounds before any count-based allocation, loop, arithmetic, or output write; run codec fixtures with sanitizers at the exact boundary.

## Fail malformed ALAC decode on undefined math

- **Problem:** A 256-input malformed-packet smoke run with UBSan halt-on-error found an oversized interlace shift and signed predictor accumulation overflow.
- **Root Cause:** The ALAC bitstream exposed fields used as C shift counts and arithmetic operands without range checks; unsupported predictor modes were logged but still continued into reconstruction.
- **Fix:** Reject unsupported predictor modes, zero quantization where the algorithm shifts by quantization minus one, invalid sample widths, and interlace shifts above 31. Use 64-bit predictor and weighted-interlace intermediates, explicit bounded sign extension, and constrain the RAOP Rice k-modifier to 30 or less.
- **Prevention Rule:** Keep a deterministic malformed-frame sanitizer corpus in the regular host path and set sanitizers to halt, so undefined behavior cannot be reported as a passing test.

## Keep package evidence separate from emulator evidence

- **Problem:** A validated build can be mistaken for a working runtime, especially when GUI access is temporarily blocked.
- **Root Cause:** Build and package checks complete in Lima independently of whether RPCS3 can be launched and inspected.
- **Fix:** Record source revision, ELF/PKG hashes, package structure checks, and runtime results as separate facts. Preserve the candidate locally while desktop access is unavailable.
- **Prevention Rule:** Never infer app launch, visible display, network readiness, or playback from ELF/PKG validation. Require a fresh runtime log tied to the tested package and user-visible evidence for audible output.

## Mutate valid compressed frames to reach deep decoder paths

- **Problem:** Uniform random packet bytes can be rejected at the fixed header and provide little coverage of entropy and predictor code.
- **Root Cause:** The initial sanitizer smoke generated only unconstrained random payloads, most of which did not resemble a valid ALAC frame.
- **Fix:** Keep a valid reference payload, preserve its fixed 54-bit header, and apply deterministic mutations to compressed bits; also test truncated variants and retain the random-byte corpus.
- **Prevention Rule:** For binary codec hardening, combine malformed random input with structure-aware mutations of a valid fixture under sanitizers.

## Require a fresh app log for headless emulator evidence

- **Problem:** A long-lived emulator process can be mistaken as evidence that a directly invoked title ran.
- **Root Cause:** The RPCS3 `--no-gui` invocation produced no stdout and did not update the capped application log.
- **Fix:** Preserve the previous log, compare its timestamp and package version after an attempted launch, and classify the attempt as inconclusive when no fresh startup event appears.
- **Prevention Rule:** A running RPCS3 process or elapsed timeout is not a title launch. Require a fresh version marker and startup events before reporting runtime evidence.

## Match the observed display to its package before repeating a UI fix

- **Problem:** The user reports blue framing and a `DISPLAY ACTIVE` label even though the current screen renderer contains neither.
- **Root Cause:** Unknown; the observed package and platform are not identified, and RPCS3 desktop access is currently unavailable.
- **Fix:** Verify source ancestry and keep the image-only renderer; record the visual mismatch as unresolved until a fresh, version-matched run can be inspected.
- **Prevention Rule:** Tie visual observations to the installed APP_VER and build before editing already-removed UI elements or creating another package.

## Keep the RAOP TXT server version aligned with RTSP

- **Problem:** Discovery advertised `vs=0.1.2` while RTSP advertised `AirTunes/105.1`.
- **Root Cause:** The TXT version was hard-coded from the early app/package version instead of the RAOP server protocol identity.
- **Fix:** Advertise `vs=105.1` and assert the exact TXT capability list in the discovery serializer test.
- **Prevention Rule:** Treat service TXT fields as protocol metadata; keep the RAOP server version consistent with the RTSP `Server` response and cover it in wire-format tests.

## Log successful decryption as a separate receive boundary

- **Problem:** RTP receipt and decrypt failures were visible, but a successful payload decryption had no explicit event, leaving logs unable to distinguish that stage from ALAC decode.
- **Root Cause:** Success was represented only by `raop_receiver_pop_audio()` returning a frame to its caller.
- **Fix:** Count successful decryptions and log the first packet plus each 128-packet checkpoint, with only the bounded count and payload length. Assert the counter in the encrypted RTP-to-PCM integration test.
- **Prevention Rule:** Instrument receiver pipelines at both success and failure boundaries, aggregate high-frequency success logs, and keep packet contents and peer identity out of diagnostics.

## Bound RTSP reply writes as well as request reads

- **Problem:** The RTSP parser bounded incomplete requests, but replies used a blocking `send()` loop with no timeout; a peer that stopped reading could hold the receiver loop.
- **Root Cause:** Request deadlines were enforced on receive while response writes had no socket-level or whole-response deadline.
- **Fix:** Configure a two-second `SO_SNDTIMEO` on accepted clients and check a two-second overall deadline across partial writes. Log write timeout separately from other send errors.
- **Prevention Rule:** Network deadlines must cover both directions. Keep a per-system-call timeout and a total transaction deadline, and test socket configuration plus the slow-reader path.

## Expose RTP health counters at bounded checkpoints

- **Problem:** The receiver counted missing, reordered, duplicate, late, and dropped packets internally, but logs mainly showed receive batches and resend events. A hardware log could therefore show RTP arrival without revealing queue health.
- **Root Cause:** The receiver diagnostics were added incrementally at the receive/decrypt boundaries; queue health statistics were not carried through to the capped log.
- **Fix:** Emit privacy-safe counter deltas and queue depth every 128 parsed packets, omit zero-valued error counters, and reset the log baseline on FLUSH while retaining internal cumulative stats.
- **Prevention Rule:** For real-time media, log bounded aggregate health counters at a fixed interval and test a discontinuity across stream flush; do not rely on raw per-packet logs.

## Separate RPCS3 host networking failures from receiver defects

- **Problem:** v0.1.24 stopped before opening the receiver when RPCS3 had no route; after enabling the emulator network, binding the production RTSP port still failed.
- **Root Cause:** The disconnected emulator could not provide a NetCtl address. Once connected, host TCP 5000 was already owned, so RPCS3's translated socket could not bind it. A port conflict is an emulator-host constraint, not evidence that the PS3 listener is faulty.
- **Fix:** Preserve production TCP 5000 and build an emulator-only v0.1.25 with the existing `RTSP_PORT=5001` override. It opened RTSP/RTP sockets and answered a local OPTIONS probe. mDNS separately failed at multicast-interface setup.
- **Prevention Rule:** Verify RPCS3 NetCtl state and host port ownership before changing receiver code. Keep emulator overrides distinct from production defaults, and classify mDNS interface failures separately from TCP/RTSP results.

## Add phase events when a protocol method stalls

- **Problem:** The v0.1.25 app log showed RTSP `ANNOUNCE` but not whether SDP parsing, RSA key unwrap, or reply generation completed.
- **Root Cause:** Method/status logging exposed the request boundary but not the synchronous substeps inside session handling.
- **Fix:** Add privacy-safe SDP parse and RSA key-unwrapping start/completion events and a repeatable host-to-RPCS3 synthetic sender. Keep the session log sink optional for host-only tests.
- **Prevention Rule:** For blocking protocol work, log entry and completion around each expensive or stateful stage without including SDP, keys, peer addresses, or media.

## Read the complete capped diagnostic log in host tests

- **Problem:** The slow-reader test occasionally failed to find `RTSP_SEND_TIMEOUT` even though the event was present in the receiver log.
- **Root Cause:** The test buffer read 4 KiB while the logger permits up to 16 KiB; the timeout event could be beyond the truncated prefix.
- **Fix:** Size the test read buffer from `RECEIVER_LOG_MAX_BYTES` plus one terminator byte.
- **Prevention Rule:** Test log assertions must read the full declared cap or parse incrementally; a truncated read can report a false product regression.


## Treat parser completion and response completion as separate boundaries

- **Problem:** The v0.1.26 RPCS3 log showed SDP parsing completed successfully, but the remote synthetic sender still received no ANNOUNCE response.
- **Root Cause:** The first phase instrumentation stopped at SDP parsing and RSA key unwrap; it did not expose validation branches or response construction. The RSA unwrap event was absent, but later inspection found its 25-character name exceeded the logger’s 24-character limit; absence did not identify the stall.
- **Fix:** Add bounded allowlisted events for ANNOUNCE field classification, validation completion, and response construction, then replay the same sender fixture.
- **Prevention Rule:** Instrument entry and completion around synchronous protocol boundaries before changing protocol or crypto behavior based on an emulator timeout.


## Bound scans of parsed fixed-size header fields

- **Problem:** v0.1.27 logging stopped after ANNOUNCE offer classification and before validation completion.
- **Root Cause:** The validation path used `strlen` on a fixed-size parsed Content-Type buffer. Its terminator should be written by the parser, but the RPCS3 evidence did not show whether this expression was the cause.
- **Fix:** Replace the unbounded scan with a capacity-bounded length helper and log the check boundary/result.
- **Prevention Rule:** Use bounded string lengths for fixed-capacity protocol fields, even when the parser is expected to NUL-terminate them; retain phase events to validate the runtime hypothesis.


## Keep diagnostic event limits in sync with event names

- **Problem:** RPCS3 v0.1.28 logs stopped after offer fields, while newly added Content-Type, RSA-start, crypto-input, and response event names were missing.
- **Root Cause:** `receiver_log` silently rejected event names longer than 24 characters, although several new names were 25–33 characters. The session callback ignored the logger's failure result.
- **Fix:** Raise the explicit event-name bound to 48 characters, enlarge the formatted-line buffer, and assert that a 33-character event is actually written and retrievable.
- **Prevention Rule:** Add a logger persistence test whenever diagnostic event vocabulary changes; verify the maximum event length is compatible with the storage filter and line buffer. Never use absence of an event as evidence until logger acceptance is verified.

## Release labels and PS3 SFO versions are different formats

- **Problem:** Packages installed but failed to launch with 80028F21. SFO fields contained 0.1.29 instead of XX.YY.
- **Root Cause:** Packaging substituted the human release label into both platform version fields, and checks only compared that same invalid string. This is a concrete defect; its relationship to the launch error requires hardware confirmation.
- **Fix:** Release 0.30 maps to SFO 00.30. Parse extracted SFO fields, enforce XX.YY, and match the v01.01 debug container format. Reject mismatched revision manifests before compiling.
- **Prevention Rule:** Validate platform formats independently of application labels. RPCS3 startup and successful installation do not prove the console accepts launch metadata. Derive source revision from the exact exported commit.

## SFO version defect blocked hardware launch

- **Problem:** 0.1.29 installed and appeared in Game but failed to launch with 80028F21.
- **Root Cause:** The build pipeline wrote a three-component application release label into PS3 SFO version fields, violating their XX.YY syntax. This was a launch-blocking metadata defect missed by earlier self-comparison checks and RPCS3 testing.
- **Fix:** SFO 00.30 restored hardware startup, diagnostic tone, and idle START-to-XMB in the compatibility build. The package type was also restored to the v01.01 type-1 format in that same build; strict causal isolation between those simultaneous changes was not performed.
- **Prevention Rule:** Keep release 0.1.30 separate from platform version 00.30. Validate actual extracted SFO syntax independently; installation success and emulator startup do not establish console launch compatibility.

## An iPhone output selection is not a completed AirPlay session

- **Problem:** iPhone selected the PS3 and Spotify showed playing, but no music sounded and controller exit later appeared frozen.
- **Root Cause:** Not yet established. Saved log stops inside OPTIONS, before any ANNOUNCE/SETUP/RECORD or RTP events; there is no START event or cleanup evidence.
- **Fix:** Preserve the original log and investigate synchronous OPTIONS challenge handling before touching ALAC/output timing or blaming cleanup. Correct the working build label without changing runtime code.
- **Prevention Rule:** Confirm each RTSP/RTP/PCM layer in logs. Preserve logs before relaunch (opening the app truncates the log). Avoid repeated streaming attempts that require forced shutdown.

## PPC64 registers do not imply 64-bit pointers on PS3

- **Problem:** OPTIONS challenge signing and synthetic ANNOUNCE key recovery stopped, followed by loss of controller responsiveness. Saved RPCS3 output records an invalid-memory fault in bignum multiplication.
- **Root Cause:** Mbed TLS selected PPC64 assembly that loads/stores pointer locals as eight bytes; the PS3 compiler stores those pointers as four bytes. This corrupts pointer slots during RSA arithmetic.
- **Fix:** Disable that assembly path through a consistent PS3 user configuration; retain portable C arithmetic, RSA blinding, and RNG. Add phase/timing diagnostics.
- **Prevention Rule:** Run private-key operations on the target ABI. Host crypto tests cannot catch a target-specific assembly/pointer-width incompatibility. Always inspect emulator fatal logs alongside app logs before diagnosing a missing response as a timeout.

The v0.1.31 fresh RPCS3 replay verifies the portable-arithmetic correction: the same challenge/private-key path completes (54 ms challenge, 26 ms encrypted ANNOUNCE), followed by successful SETUP/RECORD/TEARDOWN. Actual PS3 streamed music and controller behavior still require observation.

## Big-endian decode and live control diagnostics

- **Problem:** First real sender attempt produced crackling; START and PS controls appeared unavailable after disconnect.
- **Root Cause:** Decoder incorrectly hard-coded little-endian host order, producing incorrectly ordered 16-bit PCM on PS3. Separately, the app only flipped the display at startup and serviced controls after network work. Idle underrun spam overwrote the session evidence; the exact control/overlay cause is not yet confirmed.
- **Fix:** Compiler-based byte order, nonblocking regular presentation, input/callback polling first, bounded graphics cleanup, small decode batches, and rate-limited heartbeat/pad/menu logging. Remote normal quit completed all cleanup stages without reboot.
- **Prevention Rule:** Compare PCM bytes on the target architecture; a little-endian host test cannot validate the big-endian branch. Preserve logs before recovery and distinguish live audio polling from a frozen process. Do not promise PS-button overlay support until verified on hardware.

## Classic RAOP encryption and compositor resume need native coverage

- **Problem:** Real sender audio still crackled; XMB opened but app resume stuck and image flickered.
- **Root Cause:** Receiver incorrectly made RSA/AES setup depend on the optional et SDP attribute. Existing fixtures always supplied it and masked the gap. Separately, periodic presentation omitted the reference GPU wait-flip command and reused buffers after compositor activity without restoring their image. Logs show menu-close callbacks but do not isolate the exact resume cause.
- **Fix:** Infer encryption from complete key/IV pair when et is absent; reject partial pairs; test the missing-et path. Queue standard GPU wait ordering and refresh background buffers on menu close. Limit repeated decode failures and periodically repeat negotiated format/encryption status.
- **Prevention Rule:** Cover sender variants rather than a single synthetic SDP. A selected route and AES-call success do not prove decoded audio. Emulator presentation does not establish real XMB resume behavior. Preserve diagnostic context by aggregating repeated errors.


## Retain packet gaps during resend throttling

- **Problem:** The first real AirPlay music was audible but crackled. The retained v0.1.33 log reports repeated RTP sequence gaps alongside successful decrypt, ALAC decode and PCM generation.
- **Root Cause:** The resend handler acted on only the most recent gap, ignored it when the 50 ms pacing interval had not elapsed, and replaced the sole accepted resend range with later requests. This can lose recovery opportunities; the wrapped hardware log does not prove it caused every audible artifact.
- **Fix:** v0.1.34 queues a bounded number of missing ranges, retains up to 16 outstanding ranges for one second, and accepts delayed responses against any matching range. The host loopback sends two gaps close enough to hit the throttle and returns both retransmissions later.
- **Prevention Rule:** Keep rate-limited work in an explicit bounded pending queue, and track asynchronous replies by request/range rather than one mutable current slot. Verify real-time recovery counts and user-audible improvement on hardware.

## A hardware audio milestone can coexist with a quality failure

- **Problem:** The maintainer heard multiple real Spotify songs through AirPlay, yet audio was still crackly and low fidelity, while the screen flickered after XMB resume.
- **Root Cause:** Not fully established. The log proves encrypted RTP, successful ALAC frames and PCM submissions, and also shows sequence gaps; it does not include enough history for exact loss/recovery accounting. The display callbacks and no-error log do not prove stable scanout.
- **Fix:** Record this as first audible playback, not stable playback. Target the measured resend weakness independently and retain frequent aggregate counters; keep XMB resume as its own hardware gate.
- **Prevention Rule:** Report success by layer and quality separately. A decoded PCM path and a human-heard song do not imply clean audio, accurate timing, stable display, or a passing soak test.

## Verify package identity before attributing live audio diagnostics

- **Problem:** A newly staged receiver build was easy to confuse with the currently running app when evaluating crackling.
- **Root Cause:** The v0.1.34 install request timed out, while the PS3 continued running v0.1.33. The live log cadence (128-packet checkpoints) also differed from v0.1.34's 512-packet cadence.
- **Fix:** Retrieved installed SFO and EBOOT over FTP and compared both byte-for-byte with local candidates; v0.1.33 matched, v0.1.34 did not. The staged v0.1.34 PKG independently matched its local file.
- **Prevention Rule:** Identify the installed binary before interpreting its telemetry. Confirm an app has exited before replacing the same title.

## Select the intended microphone before recording diagnostic audio

- **Problem:** An attempted short capture used the iPhone microphone rather than the MacBook's built-in microphone.
- **Root Cause:** The AVFoundation input was selected by device index instead of verified by name; device index zero was not the intended local microphone.
- **Fix:** Discarded that capture as evidence and analyzed only the user's supplied MacBook recording. The supplied file's gross level analysis found no hard silence or large sample discontinuity, but does not exclude subtle crackle.
- **Prevention Rule:** Enumerate and explicitly select the MacBook microphone, confirm its identity before recording, and keep the sample private. Never assume the default input is local.

## Exclude intentional startup silence from stream underrun telemetry

- **Problem:** The PS3 log reported 51,200 PCM underflow frames, which looked like a large playback dropout.
- **Root Cause:** The audio callback rendered the empty PCM ring after the diagnostic tone and during RAOP's 128-packet prebuffer. Those frames are intentional silence, but the ring counted them as underruns. Counter baselines could also carry stale values across recording sessions.
- **Fix:** v0.1.35 renders intentional silence without touching ring statistics until the decoder and prebuffer are ready, and resets underflow/drop report baselines at session boundaries. A host regression asserts silence rendering does not increment underflows while a true empty active render does.
- **Prevention Rule:** Count an audio underrun only after the stream is in a state where PCM is expected. Separate intentional lead-in/prebuffer silence from starvation during active playback.


## RTP retransmission responses without a live tracking slot

- **Problem:** The live v0.1.35 log showed active PCM underflows alongside sequence gaps, resend requests, and no recovered packets. Rejection logs did not say why valid resend packets were rejected.
- **Root Cause:** Confirmed code behavior: a parsed resend was discarded whenever its one-second outstanding tracking entry was absent, even when its sequence could still be ahead of playout. The hardware log does not establish how many responses failed for this reason versus arriving late.
- **Fix:** v0.1.36 accepts an unmatched resend only from the negotiated peer and only when it is within the bounded 256-packet window behind the highest original RTP sequence. Queue insertion continues to reject already-played packets, duplicates, and capacity overflow. Add aggregated reason counters and an integration regression for a still-playable resend after tracking loss.
- **Prevention Rule:** Log resend acceptance/rejection categories and correlate active PCM underflows with received, missing, late, recovered, and buffer-occupancy counters before changing the resampler or claiming a single cause.

## Correlate active PCM starvation with RTP loss before changing audio DSP

- **Problem:** The user still hears AirPlay dropouts; the latest capture records 86,106 underflow frames during active playback.
- **Root Cause:** The same v0.1.35 capture has 195 missing RTP sequence numbers and no recovered resend packets in retained totals. Their expected audio duration is close to the logged underflow duration. This is strong correlation, not exclusive proof; sender pacing and scheduling may also contribute.
- **Fix:** Evaluate the bounded resend recovery already added in v0.1.36 on a new v0.1.37 hardware run, and compare missing, recovered, late, and underflow counters before changing buffering or resampling.
- **Prevention Rule:** Normalize packet gaps to audio-frame duration and compare them with active underflow frames. Keep recovery counters and avoid a decoder rewrite while transport loss is measurable.

## Static output does not need a continuous flip loop

- **Problem:** The user reports background flicker, especially after PS menu resume. Logs show no flip failure but record about 56.5 flips per second for a static image.
- **Root Cause:** The renderer continuously flips two identical framebuffers to support the system overlay. The log does not establish that this alone causes visible flicker.
- **Fix:** v0.1.37 submits a flip only for a menu-resume redraw, then refreshes the inactive framebuffer without repeated scanout changes.
- **Prevention Rule:** Keep a static standalone screen static. Test menu resume separately from steady-state scanout and require repeated physical display confirmation.


## Separate bounded emulator evidence from PS3 acceptance

- **Problem:** RPCS3 can launch and process a synthetic encrypted stream, but user-visible flicker and real audio dropout occur on physical hardware.
- **Root Cause:** Emulator validation establishes executable protocol stages only. It lacks the PS3 display compositor, HDMI chain, physical network path, and acoustic output conditions. mDNS also failed in this RPCS3 run.
- **Fix:** Record emulator milestones narrowly (RTSP status, key unwrap, decrypted RTP, ALAC frame, fixture result) and leave hardware display, sustained PCM, and audible quality explicitly pending until a version-matched console run.
- **Prevention Rule:** Do not promote emulator decode or a single screenshot into hardware playback/display acceptance. Use direct FTP for console files and webMAN for console controls; preserve the installed app when the endpoint cannot be reached.

## A valid package is not an emulator run

- **Problem:** The v0.1.39 package passed ELF, SFO, content-ID, extraction, asset, and NPDRM validation, but the attempted RPCS3 install did not complete.
- **Root Cause:** The RPCS3 Computer Use accessibility call timed out while opening the package installer, then remained unable to observe the app. The previous log still belongs to v0.1.38.
- **Fix:** Preserve the v0.1.39 candidate packages and exact hashes, record runtime as pending, and keep the existing emulator installation intact until UI control is available.
- **Prevention Rule:** Treat compile, package validation, emulator install, app launch, fixture replay, and audio observation as separate gates; never attribute an older log to a newly built PKG.


## Target RTSP status and client EOF are conflicting evidence

- **Problem:** A bounded TCP OPTIONS probe connected to the physical receiver, but the client read EOF instead of a response.
- **Root Cause:** Not established. The receiver log records status 200 only after its `send_all` routine returns success; this shows the server-side send path completed, but does not establish bytes reached or were parsed by the client.
- **Fix:** Preserve both observations, inspect response framing/socket lifecycle and repeat with a client that records received bytes and close timing. Do not infer full RAOP connection from discovery, TCP accept, or the server status log.
- **Prevention Rule:** Correlate client-side wire evidence with receiver-side logs at every protocol boundary.


## A remote game-exit request can recover the foreground app cleanly

- **Problem:** User reported controller input and the PS button stopped working during an active AirPlay session.
- **Root Cause:** The app log only samples controller polling result and pad-port presence; it does not record button masks or transitions. The app's retained heartbeat showed its system-menu-open flag set, but that is not proof the menu was visible or responsive.
- **Fix:** The webMAN XMB exit command produced `SYSUTIL_EXIT_GAME`, followed by screen/audio/receiver cleanup and `APP_STOP`; read-only status then showed XMB. Add privacy-safe button transition diagnostics to determine whether control samples arrive.
- **Prevention Rule:** Separate controller API health, controller presence, button state, system-menu callbacks, and application cleanup into distinct logged facts.

## Packet recovery helps but does not equal jitter measurement

- **Problem:** The v0.1.39 stream log showed substantial RTP gaps while also showing retransmission recovery and PCM output.
- **Root Cause:** Current counters report sequence gaps and recovery results, not packet arrival-time variation or continuous buffer occupancy. Sampling windows differ, and the capped log discarded older data.
- **Fix:** Preserve the observed gap/recovery/concealment counts as separate counters. Add monotonic RTP inter-arrival jitter, PCM occupancy extrema/starvation duration, and audio scheduling lateness before tuning buffer sizes.
- **Prevention Rule:** Do not infer jitter or exact unrecovered loss by subtracting counters sampled at different boundaries.

## Startup test tone is separate from receiver readiness

- **Problem:** A diagnostic tone played automatically at launch even though the user no longer needed it; it could be mistaken for an AirPlay audio result and delayed the useful silent waiting state.
- **Root Cause:** The tone generator was directly wired into the production audio-block renderer before receiver PCM.
- **Fix:** Remove tone state, generation, startup/completion events, and the tone object from the receiver link. Submit silence until PCM is ready. Retain the standalone tone component and host test for intentional diagnostics.
- **Prevention Rule:** Do not make a test signal part of normal playback startup; keep diagnostic stimuli explicitly invoked and report them separately from stream evidence.

## PCM resampling needs an objective high-frequency baseline

- **Problem:** Host PCM tests proved frame flow and lack of underflow but did not quantify frequency-dependent fidelity for 44.1-to-48 kHz conversion.
- **Root Cause:** Existing tests checked structural ring behavior, not output error against a reference waveform.
- **Fix:** Add a production-ring measurement over six frequencies and report SNR, amplitude ratio, and errors. The linear interpolation baseline degrades substantially at high frequencies (8.47 dB SNR at 15 kHz and 4.17 dB at 20 kHz in the current test); investigate a bounded higher-quality resampler as a separate measured change.
- **Prevention Rule:** Compare resampler candidates with aligned reference signals and report CPU cost and frame continuity before changing the production algorithm.

## Pad presence is not proof of controller input

- **Problem:** During v0.2.0 streaming, the user reported that START and PS did not work, while the log reported `PAD_HEALTH` success and port 0 present.
- **Root Cause:** The diagnostic only recorded `ioPadGetInfo` result and a connection bit. It did not record button states or named system-menu callbacks, so it could not show whether START reached the app or the PS event reached sysutil.
- **Fix:** Add privacy-safe button-mask transitions, pad connect/disconnect and data errors, and named menu-open/menu-close/exit events. Keep the existing webMAN exit as the tested recovery path.
- **Prevention Rule:** Report controller API health, pad presence, button transitions, system callback delivery, and app exit as separate signals. Never infer working controls from a present pad.

## Installed SELF may have a zero-padded tail

- **Problem:** The installed EBOOT hash differed from the package extraction during v0.2.0 and v0.2.1 checks.
- **Root Cause:** The installed EBOOT was 48 bytes longer; the extracted package SELF matched the installed file byte-for-byte as a prefix and the added tail was zero-filled.
- **Fix:** Verify SFO identity, compare the exact package SELF-length prefix, and inspect the extra tail before concluding the installed payload changed.
- **Prevention Rule:** Record exact lengths and compare the package payload prefix before treating a whole-file hash mismatch as a build/install failure.

## PS menu event delivery does not prove visible controller support

- **Problem:** The v0.2.1 retest was inconsistent: the user reports START exited once, then START and PS stopped responding after relaunch.
- **Root Cause:** The latest log records a PS menu-open callback, but no close callback, START transition, or button transition after the initial zero sample. `PAD_HEALTH` only proves the pad info call succeeded and a port is present; it does not prove fresh button reports arrived. The log cannot prove the menu was visible.
- **Fix:** Preserve the separate signals. The next diagnostic pass should log raw digital pad masks and positive report counts, and recognize START from both the PSL1GHT field and its documented raw mask.
- **Prevention Rule:** Do not infer working controls from pad presence or callback delivery. Correlate raw pad reports, normalized state transitions, sysutil open/close, user-visible menu state, and cleanup.

## A positive pad report does not imply a digital button transition

- **Problem:** v0.2.2 reported two positive `ioPadGetData` reports after the PS menu-open callback, but the user saw no PS response and the log contained no raw digital mask change or START event.
- **Root Cause:** Report count alone does not identify whether a report contained analog movement, a button edge, or a system-menu side effect. The callback confirms sysutil received a PS-menu-open event, while no close event or visible-menu confirmation exists.
- **Fix:** Keep the manual-reboot comparison pending and correlate raw masks, app START events, sysutil open/close, and user-visible behavior. Do not treat report count or callback delivery as proof of usable input.
- **Prevention Rule:** Log and interpret pad report arrival, digital mask changes, button actions, OS menu callbacks, and visible UI as separate evidence.


## Manual launch did not change controller failure

- **Problem:** v0.2.2 still had no usable controller buttons after a user-performed reboot and direct XMB launch.
- **Root Cause:** Not established. The run showed no digital mask transition or START event; a `SYSUTIL_MENU_OPEN` callback arrived without a close. This is insufficient to decide whether button data is missing or whether the system menu is open and intercepting app input.
- **Fix:** No speculative code change. Record the fresh run and next isolate START-first from PS-button behavior.
- **Prevention Rule:** Compare launch paths only with matching, controlled button order and visible-result observations; do not treat a system callback as evidence that the UI appeared.

## START raw-mask fallback must reach the exit action

- **Problem:** Controller code decoded START from both PSL1GHT's `BTN_START` field and the raw digital mask, but app exit still tested only `data.BTN_START`.
- **Root Cause:** The fallback was wired into diagnostic/normalized button state but not into the action condition.
- **Fix:** Use the normalized START bit for the exit action so either supported representation triggers the same behavior.
- **Prevention Rule:** Keep input decoding and action dispatch on the same normalized state; add a focused regression check for fallback-driven START dispatch.

## A webMAN HTTP success is not proof an app launched

- **Problem:** The v0.2.3 webMAN play request returned HTTP 200, but the app log contained no new `APP_START` and webMAN still reported XMB.
- **Root Cause:** The HTTP response confirms command handling, not successful title start.
- **Fix:** Require a fresh app-version/startup log and matching runtime state before recording launch as PASS; hand off to a direct XMB launch if the remote command does not start it.
- **Prevention Rule:** Track upload, install, launch, runtime input, and user-visible behavior as separate gates.


## System-menu callback is not proof of a rendered menu

- **Problem:** v0.2.3 logged `SYSUTIL_DRAW_BEGIN` and `SYSUTIL_MENU_OPEN`, but the user saw no XMB; START stopped working afterward.
- **Root Cause:** Suspected interaction between VSH's RSX overlay and the static-screen optimization that stopped flips when no redraw was pending. This is not yet confirmed on hardware.
- **Fix:** v0.2.4 resumes paced framebuffer flips only during the menu-open interval; the installed build still needs a hardware test.
- **Prevention Rule:** Validate visible system-menu output separately from callback delivery; retain scanout refresh while VSH draws overlays.

## Capped logs must retain prior launches

- **Problem:** Opening the diagnostic path with `wb` and wrapping at 16 KiB erased evidence from previous app sessions.
- **Root Cause:** The cap applied to the whole file and open truncated it, rather than bounding each launch separately.
- **Fix:** v0.2.4 appends sessions, timestamps `APP_START` in UTC, caps each session at 16 KiB, writes `LOG_SESSION_CAPPED`, then stops logging until next launch. Host regression passes; hardware append behavior is not yet confirmed.
- **Prevention Rule:** Bound log volume per session without truncating prior sessions; test append, timestamp and cap semantics across opens.

## A small append-only cap can expire before a stream test

- **Problem:** v0.2.4's log reached its 16 KiB per-run cap roughly 37 seconds after APP_START during active AirPlay, too early for a 10-15 minute stutter investigation.
- **Root Cause:** Verbose network/session diagnostics filled the per-launch budget quickly; the file then correctly stopped accepting events.
- **Fix:** Candidate v0.2.5 raises the per-launch budget to 5 MiB and tests the full cap while preserving older sessions.
- **Prevention Rule:** Size diagnostic budgets against observed event rates and requested test duration; include a release regression that fills and checks the configured cap.

## A combined release wrapper must pass toolchain variables downstream

- **Problem:** The new release wrapper passed host tests and built the PPU ELF, but package creation stopped because `PS3DEV` was unset in the packaging subprocess.
- **Root Cause:** The wrapper exported `PS3DK_SOURCE` and version values but not the derived asset/tool path expected by the package script.
- **Fix:** Derive and export `PS3DEV` from `PS3DK_SOURCE` before running the component stages; the corrected full release gate passed.
- **Prevention Rule:** Run every new build orchestrator end-to-end through package validation; passing unit/build stages alone does not validate its environment handoff.

## v0.2.6: packet validity and VSH transition lifecycle

- Problem: valid uncompressed ALAC exceeded all three former 1,400-byte caps; silence concealment hid this behind healthy PCM counts.
- Root Cause: separate encoded-size constants were smaller than the 1,416-byte stereo fixture, and pre-decode malformed counters were absent.
- Fix: shared 1,536-byte bound, exact synthetic PCM regression including AES clear tail, malformed/retry interval counters, bounded large-gap splitting/retries.
- Prevention Rule: include incompressible encrypted audio and maximum-size boundaries in each release gate; zero decode errors does not establish zero transport drops.
- Problem: VSH resume could remain dim with a frozen indicator after static presentation stopped.
- Root Cause: unproven; immediate return to static scanout may precede compositor completion, and DRAW event state was not tracked.
- Fix: candidate tracks DRAW state, paces a bounded two-second resume interval, then restores both images. Mocked tests verify presenter state transitions and timeout; visual hardware acceptance remains pending.
- Prevention Rule: preserve input responsiveness and require physical PS-menu/resume/START confirmation before labeling display restoration fixed.


## Fidelity and clock attribution

- Problem: Healthy PCM and zero ALAC errors coexist with imperfect sound.
- Root Cause: Linear interpolation is measurably inaccurate at high frequencies; output block pacing and oscillator drift are independent unknowns.
- Fix: A bounded FIR candidate is reference-tested; low-rate service/render/log timing aggregates are added before tuning clocks.
- Prevention Rule: Preserve exact nominal ratio and disclose filter transition limits; evaluate connection epochs separately and never equate healthy buffering with an on-time HDMI block.

## XMB guard test horizon

- Problem: The expanded 500/750ms guard tests failed the rapid-cycle static assertion.
- Root Cause: The test allowed only30ticks regardless of the selected guard.
- Fix: Derive the test horizon from the guard plus bounded flip completion.
- Prevention Rule: Test time bounds relative to the configured deadline and verify the default separately against the product target.
