# Risks and safety gates

| Risk | Impact | Mitigation / evidence needed |
|---|---|---|
| Game audio cannot mix with external PCM | In-game background audio goal may not be achievable | Treat as a separate feasibility gate; test per title and make no universal claim |
| Resident module lifecycle is incompatible or unstable | Freeze, crash, or loss of console access | Do not load until exact compatibility and a tested recovery path are established; never use boot loading for initial tests |
| Standalone and resident SDK assumptions differ | Link/load failure or instability | Validate application and PRX paths independently |
| RAOP behavior varies across senders | Discovery or connection may fail | Keep protocol bounded, log sanitized state, and test actual sender versions |
| Port or multicast conflicts | Service may not start or could disrupt another service | Check port ownership and multicast behavior before a controlled runtime test |
| Audio buffering/clock behavior is incomplete | Dropouts, latency, or drift | Add timed jitter handling and sender-clock mapping; measure on hardware |
| Diagnostic service is mistaken for playback-ready software | Silent or failed connections | Label capability accurately and gate claims on iPhone and audible-output evidence |
| Sensitive data enters source, issues, or logs | Privacy exposure | Use the contributor redaction checklist; never publish identifiers, credentials, addresses, or raw logs without review |
| Third-party licensing is incomplete | Redistribution may violate license terms | Preserve notices and review dependencies before binary releases |
| RSX label or flip polling never completes | Foreground app can appear frozen and prevent controller exit; this resembles the reported black-screen/forced-shutdown failure | v0.1.2 source has no overall deadline on these waits; review and add bounded failure handling before another risky hardware run unless v0.1.2 is deliberately selected after recovery/preflight review |
| Receiver loopback timing test is scheduling-sensitive | A host run can stall at its first timing-probe receive, reducing confidence in protocol changes | Latest direct guest run timed out; trace-assisted run passed. Diagnose the discrepancy and require repeated uninstrumented passes before marking green |
| App fails before fresh diagnostics appear | XMB launch error prevents display, controller, and AirPlay testing; a stale log can hide whether `main()` or the logger opened | v0.1.2 and v0.1.3 (legacy PSL1GHT runtime) both return `80028F21`; installed v0.1.3 files match the package, but the old log is unchanged. Next compare a minimal native PS3DK build and, if licensed access exists, Sony's official SDK build path. Do not infer a specific root cause from the error code alone. |
| Hard-coded application version in startup log | A future log could misidentify the package version and confuse diagnosis | v0.1.3 still logs `PKG_VERSION_0.1.2`. Make the startup version a build input and verify it against `PARAM.SFO` before the next package is staged. |
| Native launch control behaves differently on the PS3 than on the host | A locally valid ELF/PKG does not establish XMB launch, visible HDMI pixels, input responsiveness, or clean exit | AIRTEST01 v0.1.0 builds and passes package checks only. Transfer/install requires approval; test it once with START and automatic timeout available, and stop if launch hangs. |

## Required gates before hardware testing

1. Identify the exact target software/hardware compatibility requirements.
2. Establish and document a tested recovery route.
3. Begin with a reversible standalone test.
4. Keep boot-time plugin settings unchanged unless a separately reviewed recovery plan explicitly requires otherwise.
5. Test resident lifecycle without audio before evaluating game coexistence.
