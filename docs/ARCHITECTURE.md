# Architecture

## Goal

Receive classic AirPlay 1 / RAOP audio from an iPhone and play it through a PS3 homebrew application. A resident plugin and game-audio coexistence are separate experiments.

## Data flow

```text
iPhone / RAOP sender
        |
        v
DNS-SD/mDNS discovery -> RTSP session and SDP -> UDP RTP receive
        -> decrypt -> bounded ALAC decode -> PCM ring -> standalone GameOS audio
                                                      -> RSX diagnostic screen
```

The standalone receiver and resident PRX share portable protocol/codec components where practical, but have different lifecycle and output constraints.

## Components

- `src/discovery`: bounded RAOP DNS-SD serialization and service behavior.
- `src/protocol`: RTSP state machine, SDP parsing, RTP sequencing, timing, and retransmission handling. Session diagnostics use an optional event callback to mark SDP parsing, ANNOUNCE validation, RSA key-unwrapping, and response-formatting boundaries without logging request contents.
- `src/crypto`: RAOP challenge response and session-key/audio packet crypto.
- `src/codec`: bounded ALAC decoder wrapper.
- `src/audio`: bounded PCM ring, conversion, and resampling.
- `src/observability`: 16 KiB capped diagnostic logging that avoids stream content, peer addresses, keys, and track identifiers; wrap events identify when the bounded file rolls over.
- `prototype/standalone-receiver`: standalone app wiring for discovery, receive, decode, audio output, RSX diagnostic display, and controller-requested exit.
- `experimental/resident`: diagnostic PRX candidate; decoded PCM is discarded.

## Constraints

- Bound sessions, packet sizes, queue depths, timeouts, decoding work, and log storage.
- Accepted RTSP client sockets have a two-second send timeout, and each complete response has a two-second overall deadline. Slow or stalled readers are disconnected rather than allowed to stall the receiver loop indefinitely.
- RTP diagnostics emit an aggregate checkpoint every 128 accepted audio packets: packet/payload deltas and queue depth, plus nonzero missing, reordered, duplicate, late, and queue-drop deltas. FLUSH starts a new logging baseline while cumulative receiver statistics remain available internally.
- Advertised RAOP capabilities must match implemented behavior. Current host checks do not prove sender interoperability.
- The standalone app advertises the IPv4 address returned by NetCtl after `IPObtained`; the former multicast-socket route selection could choose an address that did not match the PS3's active network configuration.
- Verbose package logging records allowlisted RTSP method names, response codes, state changes, selected numeric SDP fields, listener/UDP readiness and errors, and batched RTP counters. It never records request bodies or peer addresses.
- Keep private keys, credentials, track metadata, peer addresses, and raw requests out of logs.
- Keep the standalone audio path distinct from the resident module path.
- Preserve the boundary between structural build checks and PS3 runtime evidence.

## Validation status

Portable host/sanitizer tests and PPU component compilation pass. v0.1.26 runs in RPCS3, opens RTSP/RTP sockets, answers OPTIONS, and parses a synthetic ANNOUNCE SDP successfully, but sends no response; RSA key unwrap did not start. mDNS setup fails at the emulated multicast interface. RPCS3 display pixels/controller exit, physical HDMI output, iPhone interoperability, and audible AirPlay music remain unverified. The repository's [test matrix](TEST_MATRIX.md) and [ANNOUNCE reports](reports/) hold current evidence.

## References

- [PSL1GHT](https://github.com/ps3dev/PSL1GHT) for standalone GameOS interfaces.
- [PS3DK](https://github.com/PS3-Homebrew/PS3DK) for the experimental PRX build path.
- [sys-airplay](https://github.com/azarucha/sys-airplay) for protocol study only.
- [Unofficial AirPlay Protocol Specification](https://github.com/openairplay/airplay-spec) for RAOP DNS-SD TXT field meanings, including `vs` as the server version.
- [Shairport Sync](https://github.com/mikebrady/shairport-sync) as an established classic AirPlay receiver behavior reference; its protocol/server version is `105.1` in the RTSP Server header and RAOP advertisements.
