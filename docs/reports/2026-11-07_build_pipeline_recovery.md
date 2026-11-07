# Standalone build pipeline recovery

**Date:** 2026-11-07
**Scope:** Recover the standalone PPU build route from the preserved AIRPLAY02 v01.01 artifact and prepare the next RPCS3 candidate.

## Evidence

The user reports that AIRPLAY02 v01.01 launched and played audio on the PS3. The package was preserved as a local reference; the user-observed runtime result has not been repeated in this pass.

RPCS3-decrypted v01.01 ELF and a fresh PS3DK PPU build share these executable properties: ELF64, big-endian, PPC64, CellOS OSABI 0x66, ET_EXEC, entry 0x10000000, and the same first four PS3-specific PT_LOAD flag values. The files are not byte-identical because the source differs. The v0.1.6 legacy-linked ELF has generic initial load flags 0x5 and 0x6, and a different memory layout. This confirms a legacy toolchain output regression; the exact reason RPCS3 rejected that package is not proven.

## Validation

- Lima/PS3DK receiver ELF build completed and passed the new known-reference layout validator.
- The same validator accepted the preserved v01.01 ELF and rejected the v0.1.6 ELF as expected.
- scripts/build-lima-crypto.sh passed RAOP crypto, session, SDP, mDNS, RTSP/RTP loopback, capped listener logging, ALAC reference decoding, PCM ring/resampling, five-second diagnostic tone, and PPU object/archive checks.
- The host loopback completed on this run; previous hangs remain recorded as intermittent test-harness behavior and were not erased by this pass.
- No v0.1.7 package was generated in this pass. The next step is to commit this pipeline, rebuild from the committed revision, finalize and extract the package, then install/launch only in RPCS3.

## Result boundary

This establishes a validated candidate build route and an ABI/layout gate. It does not establish that v0.1.7 installs or launches, that display output or controller exit works, that an iPhone can connect, or that AirPlay music is audible. No physical PS3 files or boot settings were changed.
