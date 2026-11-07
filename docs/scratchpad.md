# Active work

- Current task: build/upload release 0.30 using SFO 00.30 and v01.01-style debug package.
- Proven defect: older release labels were written directly to fixed-format SFO versions. Hardware effect pending.
- Implementation: package/SFO validators, source-revision guard, production RTSP 5000. Runtime protocol source unchanged; unfinished RNG tracing saved locally.
- Next: commit sanitized source, archive exact commit, compile/package, verify extracted metadata/assets, upload and download-back comparison, install under standing authorization and verify installed files. Human observation is required for visible image/tone/START exit.
- Keep original v01.01 and v0.1.29 artifacts for rollback. Boot plugins and firmware untouched.
