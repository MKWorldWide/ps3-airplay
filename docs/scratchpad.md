# Active work

- AIRPLAY02 v0.1.39 / SFO 00.39 is installed. It is now stopped and the console is confirmed at XMB.
- webMAN `/xmb.ps3$exit` returned HTTP 200. Fresh FTP log records SYSUTIL_EXIT_GAME, complete screen/audio/receiver cleanup, and APP_STOP; a second log download was byte-identical.
- During the preceding stream, log state reached RAOP_RECORDING (state 3), encrypted 44.1 kHz stereo RTP, successful decrypt/ALAC, and PCM submission. Two 512-packet batches reported 99 missing sequence numbers; recovery counters report 112 recovered packets and 16 concealed frames across separate intervals. No RTP skips, decode failures, or underflow event appear in the retained segment.
- PCM occupancy snapshots were 12,426 then 12,512 of 16,384 frames (~76%); received packets were queued 93-97 deep of 256. This segment suggests some packet loss but no queue saturation; log rollover means earlier behavior is missing.
- PAD_HEALTH repeatedly reports successful polling and port 0 present, but the log omits button states. User reports controller/PS input unresponsive; retained APP_HEARTBEAT has menu-open flag set, but that does not prove the menu was visible or usable.
- Next: add privacy-safe button transition and event-name logging; add direct RTP inter-arrival jitter and PCM occupancy/underflow duration metrics. Before another console launch, compare build/runtime changes and preserve this run's private logs.
