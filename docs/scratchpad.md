# Current scratchpad

- Task: implement verbose, privacy-conscious logging for the experimental resident install/runtime test; stay on `experimental/resident-airplay` and do not touch physical PS3s.
- Implemented: bounded 128-entry shared diagnostics journal with multi-producer publication sequence, monotonic microsecond stamps, event/result/value/byte fields, and dropped-record counter. Harness writes elapsed-time events, drains the resident journal during waits and teardown, and reports logger failure/overflow. Added an ASan/UBSan concurrency test and repeatable runner.
- Validation: diagnostics concurrency/overflow test passes; harness C syntax-only check passes against local API mocks; 4 container tests pass; build-script syntax and `git diff --check` pass.
- Blocker: no PS3DK PPU compiler or resident Lima VM is available in this environment, so the updated shared ABI has not been built, packaged, installed, or run in RPCS3. Do not reuse the older package to claim the new logging works.
- Next: restore the project PS3DK/Lima build endpoint, build a fresh `RPRX90001`, verify package embedding/hashes, install/run in RPCS3, then inspect resident sequence ordering and drop count in the harness log. Hardware readiness remains NOT READY.
