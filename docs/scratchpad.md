# Active work

- Installed 0.1.32 / SFO 00.32, source e34cbce; PKG SHA-256 fca5547fda865f22faff7099834d0c3a8e77e6658f955b5c3a4a941cfd7ac7b3. Upload/download and installed SFO/EBOOT match.
- PPU golden ALAC PCM comparison PASS; paced RPCS3 app received/decrypted 256 packets and queued/consumed 90,368 PCM frames. Regular presentation and pad/heartbeat healthy in emulator. No audible-target claim.
- Maintainer tests physical PS/held-PS menu and START first, then short music/disconnect/START. Fetch log before relaunch (HTTP fallback works when FTP stalls in-game).
- Historical 0.1.31 live log wrapped under idle underflow spam. Normal webMAN quit completed all cleanup without reboot. New log throttles that noise and records pad/menu/heartbeat state.
- Next unused build 0.1.33 / SFO 00.33. No boot-plugin, firmware, or resident-module changes.
