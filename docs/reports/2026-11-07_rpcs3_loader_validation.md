# RPCS3 package and loader validation

Date: 2026-11-07 (America/New_York)
Base source revision: `54e6fba9bee57b9d02efcf021cea01b20e24ccd0`
Emulator: RPCS3 0.0.43-20568-b94078be

## Scope

Correct the v0.1.4 package-path issue, build the next receiver candidates, and test installation and launch through RPCS3. No PS3 files, settings, or boot plugins were changed.

## Results

| Version | Build and package result | RPCS3 result |
|---|---|---|
| 0.1.4 corrected package | 3,183,600 bytes; SHA-256 `7811cdb221765aaa9e6ac3e57bd875cb8b7409045e6f371ff7a2a7a2d0744a8f`; relative package paths and SFO verified | Install succeeded; boot failed because EBOOT was not accepted |
| 0.1.5 | 3,183,600 bytes; SHA-256 `dfe2411ce0079d09fd84e7169865fad299dcb5f2759c2b8e0876fb2b7e5a8449`; content ID and relative entries verified | Install succeeded; RPCS3 reported invalid ELF OS/ABI before app startup |
| 0.1.6 | 3,183,600 bytes; SHA-256 `c71cf7ee0ec314496971ea32d831c05238746eada6796b017092b3873a2ecc33`; PPU ELF `EI_OSABI=0x66`, SFO says 0.1.6 | Install succeeded; RPCS3 still reported invalid ELF type before app startup |

No `APP_START` event was written for v0.1.4–v0.1.6. Display output, controller exit, diagnostic tone, RAOP connection, RTP, ALAC, and streamed audio were not reached or tested. The earlier user-reported tone remains evidence for a different, unidentified run only.

## Build notes

The build script expected PSL1GHT commit `6e565a70e927f55813babe86dc6f64149535f908`, but that revision was absent in the Lima guest. The actual build used its available PSL1GHT checkout at `dd4e8c9be744e3273d665c393fa6d97a87402e2e`; this result must not be attributed to the pinned 2020 runtime. The PPU ELF was ELF64, big-endian, PowerPC64, and contained `.lib.stub`, `.sys_proc_prx_param`, and `.opd` sections. The first build advertised generic System V OS/ABI. v0.1.6 changes the OS/ABI byte to CellOS LV2 `0x66`, following the documented PS3 post-link requirement, but the remaining invalid ELF-type failure shows this patch alone is insufficient.

## Next step

Compare the inner ELF and SELF headers, `e_type`, flags, program headers, and post-link processing with a known artifact that demonstrably boots in RPCS3. Do not produce v0.1.7 until the required conversion is established. Keep the PS3 untouched until the candidate passes RPCS3 startup and the outstanding RSX/START-exit safety review.
