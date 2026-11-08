# Active work

- Current task: diagnose v0.2.2 controller input failure.
- Manual reboot and direct XMB launch reproduced the same no-buttons behavior as webMAN launch; launch method is not the differentiator.
- Latest private log: `work/hardware/2026-11-07-pre-v020-install/v022-live-log-after-manual-reboot.log`, 3,672 bytes, SHA-256 `d51d7fb882e0ddb5dc60cae08449bb1aab4292c59c700ecb0fde5a1b051c0c9f`. It records `SYSUTIL_MENU_OPEN` but no close, no raw digital transition, no START, and no app stop. Callback delivery does not prove visible menu behavior.
- The console is under the user's manual control. Do not issue remote exit, launch, install, or other state-changing commands.
- User confirms PS was pressed first and no system menu appeared visibly. Source review found the app normalized raw START into `buttons` but the exit action still checked only `data.BTN_START`.
- v0.2.3 source fix changes START exit to use the normalized mask; app label 0.2.3 / SFO 00.43. PS-menu visibility remains a separate unresolved issue.
- Build and validate v0.2.3 in the existing Lima VM. Then stage/install under standing authorization and ask the user to test START first; do not claim PS menu fixed.
