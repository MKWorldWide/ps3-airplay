# Active work

- **Current task:** Continue first standalone PS3 validation after launch failed with `80029530`.
- **Verified:** First installed EBOOT matched its package; no receiver startup log exists. Corrected PKG uses `make_self_npdrm` with the package content ID, includes the PSL1GHT default icon, and was uploaded after approval; downloaded-back size and SHA-256 match.
- **Next:** User reinstalls/updates `AIRPLAY02` from XMB using the corrected PKG, launches, checks responsiveness and tone audibility, and reports the TV result. Then retrieve and sanitize the new diagnostic log.
