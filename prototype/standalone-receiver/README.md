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

To make the standalone NPDRM homebrew package after building the ELF:

```sh
./scripts/package-lima-standalone-receiver.sh
```

Prepare the image asset with Pillow and pass it into the packager (the default
path is `build/diagnostic-background.rgb`):

```sh
python3 -m pip install Pillow
python3 scripts/prepare-diagnostic-background.py \
	assets/diagnostic-background.jpg build/diagnostic-background.rgb
./scripts/package-lima-standalone-receiver.sh
```

The result is `build/lima-standalone-package/ps3-airplay-receiver.pkg`. The
packager creates an NPDRM EBOOT bound to the package content ID, includes the
PSL1GHT default `ICON0.PNG` and a size-checked 1280x720 RGB background, and
validates package listing, SFO metadata, extraction, and payload equality. The
maintainer-provided source image is public in `assets/`; generated RGB data
stays under ignored `build/`.

## Runtime and cleanup

The app uses RTSP TCP 5000, UDP 6000–6002, and mDNS UDP 5353. It writes a capped
16 KiB diagnostic log to `/dev_hdd0/tmp/ps3-airplay.log`; it does not log audio
payloads, SDP bodies, keys, peer IPs, or track names. XMB exit triggers ordered
audio/network/crypto cleanup. The screen draws only the supplied background;
there is no blue diagnostic frame, `DISPLAY ACTIVE` label, or on-screen exit
prompt. Controller START requests orderly cleanup and return to XMB. PS-button
XMB exit remains handled through the system utility callback. Clean exit is
build-checked but not yet confirmed on hardware. To remove the log after an app
run, delete that single file through FTP. If the package is installed, remove
its app entry with the XMB Delete action. Do not add it to boot plugins.

`SCREEN_READY` confirms RSX setup and the first screen flip. `SCREEN_INIT_FAILED`
stores a negative stage code: `-1` argument, `-2` memory, `-3` background file,
`-4` RSX init, `-5` video state, `-6` video configuration, `-7` framebuffer
allocation, `-8` framebuffer mapping, `-9` display-buffer registration, and
`-10` screen flip. `CONTROLLER_READY` and `USER_EXIT_START` record input setup
and the explicit exit action.

The app no longer emits a diagnostic tone at startup. Before streamed audio is
ready, it submits silence to the GameOS audio blocks. The standalone tone
generator and host test remain available for deliberate diagnostics, but they
are not linked into the receiver package.

This runtime behavior is not yet hardware-tested. The mDNS TXT values now
match the implemented stereo ALAC, no/RSA encryption, 44.1 kHz, 16-bit UDP
path, but remain diagnostic-only until LAN discovery and iPhone session
interoperability are observed.

During RECORD, the app sends bounded classic RAOP timing probes to the sender's
SETUP timing port, then records matching replies and RTT in the capped log.
This is a diagnostic timing exchange; sender-clock alignment and audio
scheduling are still absent. Forward audio sequence gaps trigger bounded 0xD5
resend requests through the negotiated peer control port (maximum 16 packets,
50 ms minimum interval). Structurally valid 0xD6 packets from that peer are
inserted by sequence while they remain within the 256-packet receive window,
including valid replies that arrive after request bookkeeping expires. The
playout queue waits up to 80 ms at a sequence hole for a resend; if it remains
missing, the app inserts one negotiated ALAC-frame of PCM silence. It conceals
at most 128 missing frames per discontinuity, then skips to the next real
packet to prevent unbounded latency. Numeric diagnostics distinguish gap waits,
concealed frames, and skipped frames. The standalone decoder waits for 128
queued packets before starting, then drains in batches that preserve room for
a maximum ALAC frame in the PCM ring. This start buffer is not clock-synced
playback scheduling.
