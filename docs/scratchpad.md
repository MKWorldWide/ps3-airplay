# Active work

- **Current task:** Add the streamlined privacy-review procedure to global and project contribution instructions.
- **Done:** The private global instructions and project `CONTRIBUTING.md` require one concise, mostly automated pass over the full staged diff and exact changed files, followed by manual review of matches. Routine findings should be sanitized automatically and rescanned; only unsafe-to-resolve content stays out of the commit. The global instruction file remains outside the repository.
- **Commit gate:** Run `git diff --cached --check`, available secret scanners, targeted private-data searches, and manual review. Note unavailable scanners. Remediate routine findings without asking the user; report only narrow blockers that require judgment.
- **Next project work:** Resume AIRPLAY02 loader diagnosis from v0.1.6. RPCS3 still rejects the ELF type before application startup; compare SELF inner ELF headers with a known booting artifact before building v0.1.7.
