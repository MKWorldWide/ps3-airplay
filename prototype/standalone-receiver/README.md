# Standalone RAOP receiver

This PSL1GHT app connects the shared RAOP/session crypto/RTP/ALAC path to the
bounded PCM ring and GameOS audio port. It starts one RTSP session and advertises
the diagnostic-only `_raop._tcp` service. The source is not yet proven against
an iPhone or on a PS3.

## Build

On a configured Linux x86_64 PS3DK host, set the SDK source variable as required by the build script, then run:

```sh
export PS3DK_SOURCE="/path/to/ps3-sdk-source"
./scripts/build-lima-standalone-receiver.sh
```

The output is `build/lima-standalone-receiver/ps3-airplay-receiver.elf`.
No build step edits `boot_plugins.txt` or installs a plugin.

To make a fake-SELF homebrew package after building the ELF:

```sh
./scripts/package-lima-standalone-receiver.sh
```

The result is `build/lima-standalone-package/ps3-airplay-receiver.pkg`. Lima
validates its listing, `AIRPLAY02` PARAM.SFO, extraction, and EBOOT payload. The
PKG has not been installed or run on the PS3.

## Runtime and cleanup

The app uses RTSP TCP 5000, UDP 6000–6002, and mDNS UDP 5353. It writes a capped
16 KiB diagnostic log to `/dev_hdd0/tmp/ps3-airplay.log`; it does not log audio
payloads, SDP bodies, keys, peer IPs, or track names. XMB exit triggers ordered
audio/network/crypto cleanup. To remove the log after an app run, delete that
single file through FTP. If the package is installed, remove its app entry with
the XMB Delete action. If it hangs, close it from the existing homebrew manager
or restart the console; do not add it to boot plugins.

At startup the app emits a five-second, 440 Hz stereo diagnostic through the
same GameOS audio block queue used for receiver playback. Its level is capped
at five percent full scale with 50 ms fades at both ends. The capped log records
`AUDIO_TEST_TONE_START` and `AUDIO_TEST_TONE_COMPLETE`; neither event means the
sound was actually audible. A PS3 and connected display/audio system are still
required to verify that.

This runtime behavior is not yet hardware-tested. The mDNS TXT values now
match the implemented stereo ALAC, no/RSA encryption, 44.1 kHz, 16-bit UDP
path, but remain diagnostic-only until LAN discovery and iPhone session
interoperability are observed.

During RECORD, the app sends bounded classic RAOP timing probes to the sender's
SETUP timing port, then records matching replies and RTT in the capped log.
This is a diagnostic timing exchange; sender-clock alignment and audio
scheduling are still absent. Forward audio sequence gaps trigger bounded 0xD5
resend requests through the negotiated peer control port (maximum 16 packets,
50 ms minimum interval). Returned 0xD6 packets are accepted only from that
peer port and for sequences in the latest requested range, then inserted by
sequence into the bounded 256-packet queue. The standalone decoder waits for
128 queued packets before starting, then drains in batches that preserve room
for a maximum ALAC frame in the PCM ring. This start buffer is not clock-synced
playback scheduling.
