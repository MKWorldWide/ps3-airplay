# Active work

- Installed 0.1.31 / SFO 00.31; source 224f15d; PKG SHA-256 354dc5f33ef19c9342fee12efe21b70d2a7818d26660bc7ee439327675b4d60d. Uploaded/downloaded bytes and installed EBOOT/SFO match.
- Fixed incompatible PPC64 bignum assembly on PS3's 32-bit-pointer ABI. Fresh RPCS3 challenge completes in 54 ms; encrypted ANNOUNCE in 26 ms; SETUP/RECORD/TEARDOWN all 200. Target prebuffer/decode/audio remains unproven.
- Maintainer will test physical console. Collect log before relaunch; inspect RTSP progression, RTP/decrypt/ALAC/PCM, callback and cleanup events. Do not infer music from selected iPhone route.
- Next unused package version: 0.1.32 / SFO 00.32. Prior physical logs and rollback artifacts preserved locally. No firmware, boot-plugin, or resident module changes.
