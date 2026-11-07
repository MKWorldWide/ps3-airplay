# Standalone PCM tone prototype

This small PSL1GHT application exercises the PS3 user-mode audio port with a
five-second, 440 Hz stereo tone. It uses a low sample amplitude, a reduced port
level, an end fade, a bounded PCM ring, the audio event queue, and reverse-order
audio cleanup. The ring converts signed 16-bit little-endian samples into the
audio port's float format and can resample 44.1 kHz input to the 48 kHz output.
The XMB exit event also stops playback.

This remains an audio API/build proof only. It does not yet receive AirPlay,
RAOP, network discovery, background playback, or game-audio mixing. The tone
path pushes and consumes samples through the same ring planned for decoded PCM.

## Build

Set `PS3DEV` and `PSL1GHT` to an installed upstream ps3dev directory, then run:

```sh
make -C prototype/standalone
```

The default build produces `ps3-airplay-tone.elf`. Packaging that ELF as a
runnable `.self` and testing on a PS3 are separate validation steps.

## Current validation boundary

The application, PCM ring, and upstream PSL1GHT `audiosync` sample cross-compile
to PowerPC ELF files. The ring has a host ASan/UBSan test. On the current Apple
Silicon host, PSL1GHT's `fself` tool
crashes with `Bus error: 10` during the additional fake-SELF packaging step.
The ELF is not a console-tested deliverable, and no audio playback result is
claimed until packaging and a hardware run succeed.
