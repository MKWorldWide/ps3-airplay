# PS3 plugin and AirPlay research

This note records engineering boundaries from public project sources. It contains no live-console inventory.

## Findings

### A resident PS3 module differs from a standalone homebrew app

Public webMAN MOD source demonstrates start/stop callbacks, worker ownership, and cleanup patterns. This is a design reference, not a copy-ready skeleton or a compatibility guarantee. The project keeps resident PRX lifecycle work separate from the PSL1GHT standalone application.

PS3DK documents PRX generation and its TLS constraints. A structurally valid PRX only demonstrates that the selected generator accepts the container. It does not prove compatibility with a particular CFW/Cobra build or safe start/stop/unload on hardware.

### System background music is not proof of arbitrary PCM mixing

A system BGM toggle does not accept an arbitrary PCM stream. No universal game audio mixing capability is claimed. Establishing coexistence requires a separate, controlled hardware feasibility test.

### The Switch receiver is a protocol reference, not a PS3 port

[sys-airplay](https://github.com/azarucha/sys-airplay) is useful for classic RAOP protocol and buffering study. Its platform networking, audio, module lifecycle, and user-interface APIs are Nintendo Switch specific and cannot be carried over as a PS3 implementation.

### Standalone audio is its own validation gate

The upstream [PSL1GHT audio samples](https://github.com/ps3dev/PSL1GHT/tree/master/samples/audio) demonstrate the ordinary GameOS audio-port approach. A PPU build is not evidence of successful console launch or audible output. Validate app audio and resident-plugin audio separately.

## Recommended boundaries

1. Test portable protocol, crypto, codec, queue, and logging behavior on the host.
2. Validate package structure separately from console load and runtime.
3. Establish a tested recovery path before any resident module test.
4. Test AirPlay discovery/session and audible standalone output before claiming playback.
5. Evaluate in-game mixing independently and report results per tested title.

## Public references

- [sys-airplay](https://github.com/azarucha/sys-airplay)
- [PSL1GHT](https://github.com/ps3dev/PSL1GHT)
- [PS3DK PRX generation design](https://github.com/PS3-Homebrew/PS3DK/blob/v0.27.0/docs/design/sprx-generation.md)
- [webMAN MOD module lifecycle](https://github.com/aldostools/webMAN-MOD)
