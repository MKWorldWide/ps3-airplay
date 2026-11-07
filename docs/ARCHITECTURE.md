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
- `src/protocol`: RTSP state machine, SDP parsing, RTP sequencing, timing, and retransmission handling.
- `src/crypto`: RAOP challenge response and session-key/audio packet crypto.
- `src/codec`: bounded ALAC decoder wrapper.
- `src/audio`: bounded PCM ring, conversion, and resampling.
- `src/observability`: capped diagnostic logging that avoids stream content and peer identifiers.
- `prototype/standalone-receiver`: standalone app wiring for discovery, receive, decode, audio output, RSX diagnostic display, and controller-requested exit.
- `experimental/resident`: diagnostic PRX candidate; decoded PCM is discarded.

## Constraints

- Bound sessions, packet sizes, queue depths, timeouts, decoding work, and log storage.
- Advertised RAOP capabilities must match implemented behavior. Current host checks do not prove sender interoperability.
- Keep private keys, credentials, track metadata, peer addresses, and raw requests out of logs.
- Keep the standalone audio path distinct from the resident module path.
- Preserve the boundary between structural build checks and PS3 runtime evidence.

## Validation status

Portable host components have tests; cross-build and package structure checks have been completed. The user reports audible diagnostic tone and iPhone discovery, but also a black screen and failed connection. The new RSX screen and controller exit path are build-checked only. Real RAOP playback and game audio coexistence remain unverified. The repository's [test matrix](TEST_MATRIX.md) is the source of validation status.

## References

- [PSL1GHT](https://github.com/ps3dev/PSL1GHT) for standalone GameOS interfaces.
- [PS3DK](https://github.com/PS3-Homebrew/PS3DK) for the experimental PRX build path.
- [sys-airplay](https://github.com/azarucha/sys-airplay) for protocol study only.
