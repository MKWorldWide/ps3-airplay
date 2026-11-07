# Active work

- Build 0.1.31 / SFO 00.31: disable incompatible Mbed TLS PPC64 assembly on PS3's 32-bit-pointer ABI.
- Saved exact RPCS3 replay crash maps to bignum multiplication; ld/std pointer accesses disagree with compiler's lwz/stw pointer locals. Portable C fix pending fresh replay.
- Added challenge/RNG, handler-duration and cleanup/sysutil checkpoints; immediate loop exit after START detection.
- Next: clean PPU and host crypto checks, RPCS3 challenge/ANNOUNCE replay on port 5001, production build on port 5000, package validation, upload/install and exact payload verification under standing authority.
- Previous 0.1.30 logs are preserved locally. No in-game module or firmware changes.
