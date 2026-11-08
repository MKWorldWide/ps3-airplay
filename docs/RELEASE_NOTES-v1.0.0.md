# AIR PS3 v1.0.0

AIR PS3 is a standalone AirPlay 1 (RAOP) audio receiver for compatible PS3 homebrew/CFW environments. An iPhone remains the audio source; this is not Spotify Connect and does not run Spotify on the PS3.

This candidate carries forward the v0.2.8 receiver with app version `1.0.0` and SFO `APP_VER`/`VERSION` `01.00`. It preserves title ID `AIRPLAY02`, XMB title **AIR PS3**, service name **AirPS3**, and the maintainer-approved artwork. The release package is a type-1 debug PKG; installation on retail firmware is not promised.

The user accepted the v0.2.8 run after about 45 minutes and four XMB open/close cycles. The user-supplied summary reported no PCM underflows, ALAC failures, RTP queue drops, concealed/skipped frames, or screen presentation failures. A maximum audio-service interval near 45 ms occurred on XMB resume and remains a known brief hitch. This is summarized user evidence, not a fresh independent raw-log audit.

The maintainer subsequently reported that v1.0.0 is running beautifully on the development PS3. No duration, measurements, or specific playback/control checks were supplied with that report. See the [hardware observation](reports/air_ps3_v1_0_0_user_report.md); it records the report without extending it into unreported test claims.

## Included behavior

- AirPlay 1 discovery and RAOP session handling.
- RTP audio receive, decrypt/ALAC decode, bounded PCM buffering, and GameOS audio output.
- Bounded diagnostic logging at `/dev_hdd0/tmp/ps3-airplay.log`.
- Foreground PS3 application; START exits to XMB.

## Known limitation

- Returning to the app after the XMB can produce a brief resume hitch. The accepted v0.2.8 run recorded a maximum service interval near 45 ms during resume.
- Game audio mixing, resident background playback, AirPlay video/mirroring, Spotify Connect, and retail-firmware installation are out of scope.

See [installation and recovery](INSTALLATION.md) and [third-party notices](../THIRD_PARTY_NOTICES.md).
