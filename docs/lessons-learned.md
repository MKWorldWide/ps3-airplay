# Lessons learned

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
