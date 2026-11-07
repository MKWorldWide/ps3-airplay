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
| RSX label or flip polling never completes | Foreground app can appear frozen and prevent controller exit; this resembles the reported black-screen/forced-shutdown failure | v0.1.4 still inherits unbounded waits from the screen implementation, and controller setup occurs after graphics setup. Bound those waits and preserve an early exit path before the next launch test. |
| Receiver loopback timing test is scheduling-sensitive | A host run can stall at its first timing-probe receive, reducing confidence in protocol changes | Latest direct guest run timed out; trace-assisted run passed. Diagnose the discrepancy and require repeated uninstrumented passes before marking green |
| App fails before fresh diagnostics appear | XMB launch error prevents display, controller, and AirPlay testing; a stale log can hide whether `main()` or the logger opened | AIRPLAY02 v0.1.2/v0.1.3 and AIRTEST01 v0.1.0 each returned `80028F21`; receiver log remained unchanged. The reviewed PS3 developer error table has no entry for this exact code. v0.1.4 is only locally built. Compare against the earlier tone-producing AIRPLAY02 build and do not infer a root cause from the code alone. |
| Network startup depends on immediate NetCtl state | The receiver could exit before mDNS/listener startup if NetCtl has not reported an IP yet, even when routing is available | v0.1.4 restores the earlier UDP-route address lookup first and adds a five-second bounded NetCtl wait/fallback. Cross-build passes; hardware startup behavior remains unverified. |
| Native launch control diverts effort from working receiver | The silent test app does not preserve diagnostic tone or AirPlay behavior and cannot establish receiver playback | AIRTEST01 v0.1.0 is a separate installed app, not an AIRPLAY02 upgrade. It returned `80028F21` and is parked; do not use it as the receiver test path. |
| Receiver version string drifts from package metadata | Hardware logs could be misattributed to a different package | v0.1.4 makes `APP_VERSION` a build input for both startup log and SFO packaging; the extracted SFO reports 0.1.4. Verify exact installed package before interpreting future logs. |

## Required gates before hardware testing

1. Identify the exact target software/hardware compatibility requirements.
2. Establish and document a tested recovery route.
3. Begin with a reversible standalone test.
4. Keep boot-time plugin settings unchanged unless a separately reviewed recovery plan explicitly requires otherwise.
5. Test resident lifecycle without audio before evaluating game coexistence.
