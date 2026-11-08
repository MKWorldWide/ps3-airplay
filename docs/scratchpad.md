# Active work

- Current task: diagnose v0.2.2 controller input failure.
- Manual reboot and direct XMB launch reproduced the same no-buttons behavior as webMAN launch; launch method is not the differentiator.
- Latest private log: `work/hardware/2026-11-07-pre-v020-install/v022-live-log-after-manual-reboot.log`, 3,672 bytes, SHA-256 `d51d7fb882e0ddb5dc60cae08449bb1aab4292c59c700ecb0fde5a1b051c0c9f`. It records `SYSUTIL_MENU_OPEN` but no close, no raw digital transition, no START, and no app stop. Callback delivery does not prove visible menu behavior.
- The console is under the user's manual control. Do not issue remote exit, launch, install, or other state-changing commands.
- Next discriminating observation: on a fresh launch, press START first; report whether it exits. If it does not, press PS once and confirm whether the system menu visibly appears/responds. Keep these as separate button tests.
