# Lessons learned

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
