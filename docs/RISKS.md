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
| v0.1.2 XMB launch returns `80028F21` | The installed app cannot yet reach display, controller, or AirPlay validation; cause is not established and the diagnostic log has no fresh startup record | Installed EBOOT and SFO match the package, and the PKG installation completed. Verify the NPDRM/application startup contract and use a reversible minimal launch control before producing another console build. Do not alter CFW or system settings based only on anecdotal code reports |

## Required gates before hardware testing

1. Identify the exact target software/hardware compatibility requirements.
2. Establish and document a tested recovery route.
3. Begin with a reversible standalone test.
4. Keep boot-time plugin settings unchanged unless a separately reviewed recovery plan explicitly requires otherwise.
5. Test resident lifecycle without audio before evaluating game coexistence.
