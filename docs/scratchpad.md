# Active work

- **Current task:** Fix the black display and provide a clean XMB exit after the user heard the diagnostic tone but had to force power off.
- **Verified:** The previous app advertised `ps3-airplay-diagnostic`; iPhone selection failed to connect. Current-run log is not retrieved. New RSX background/START-exit build compiles in Lima and packages with the 1280x720 RGB image.
- **Next:** Record hardware observations and build evidence; review public diff. Ask approval to replace staged package using the new digest. After user updates from XMB, test displayed image, controller START exit, and inspect the capped log before debugging RAOP.
