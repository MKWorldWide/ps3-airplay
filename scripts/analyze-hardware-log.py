#!/usr/bin/env python3
"""Aggregate numeric diagnostics without publishing raw logs or device data.

TOTAL suffixes are not a counter contract. See SEMANTICS and source references
in the forensic report. Sampled sequence-bearing events are never summed.
"""
import argparse
from collections import Counter, defaultdict
from datetime import datetime
import hashlib
import json
from pathlib import Path
import re

LINE = re.compile(r'^(\S+) event=([A-Za-z0-9_.-]+) value=(-?\d+) bytes=(\d+)$')
DELTAS = {'RTP_RX_BATCH', 'RTP_MISSING_BATCH', 'RTP_REORDER_BATCH',
          'RTP_DUPLICATE_BATCH', 'RTP_LATE_BATCH', 'RTP_QUEUE_DROP_BATCH',
          'RTP_MISSING_TOTAL', 'RTP_GAP_WAIT_TOTAL', 'RTP_GAP_CONCEALED_TOTAL',
          'RTP_GAP_SKIPPED_TOTAL', 'RTP_RECOVERY_TOTAL', 'RTP_PENDING_DROPS',
          'RTP_TRACK_FULL', 'PCM_QUEUED_FRAMES', 'PCM_CONSUMED_FRAMES',
          'PCM_PORT_SUBMITTED', 'PCM_UNDERFLOW', 'PCM_DROP'}
SEMANTICS = {'delta_events': sorted(DELTAS),
 'cumulative': ['ALAC_FAIL_TOTAL', 'RTP_DECRYPT_OK', 'SCREEN_FLIPS'],
 'gauges': ['RTP_QUEUE_DEPTH', 'PCM_BUFFER_FRAMES'],
 'sampled_sequence_events': ['RTP_RESEND_REQUEST', 'RTP_RESEND_RECOVERED',
                             'RTP_GAP_CONCEALED', 'RTP_RESEND_REJECTED'],
 'warning': 'MISSING_BATCH and MISSING_TOTAL overlap; never add them. Recovery is insertion, not playback. Stream windows may be incomplete at boundaries.'}

def analyze(raw):
    sessions, current, bad = [], None, 0
    for line in raw.decode('utf-8', errors='replace').splitlines():
        m = LINE.fullmatch(line)
        if not m:
            bad += 1
            continue
        stamp, event, value, size = m.groups()
        try:
            datetime.fromisoformat(stamp.replace('Z', '+00:00'))
        except ValueError:
            bad += 1
            continue
        if current is None or event == 'APP_START':
            current = []
            sessions.append(current)
        current.append((stamp, event, int(value), int(size)))
    result = []
    for records in sessions:
        events = defaultdict(list)
        transitions, stream, playback = [], 0, 0
        stream_records = defaultdict(list)
        playback_records = defaultdict(list)
        for row in records:
            stamp, event, value, size = row
            events[event].append(row)
            if event == 'RTSP_CONNECTED':
                stream += 1
            stream_records[stream].append(row)
            if event in {'RTSP_RECORD', 'PCM_FLUSH'}:
                playback += 1
            playback_records[(stream, playback)].append(row)
            if event.startswith(('RTSP_', 'SYSUTIL_', 'SCREEN_')) or event in {'APP_START', 'APP_STOP', 'PCM_FLUSH', 'ALAC_READY', 'AUDIO_PREBUFFER_READY'}:
                if event not in {'RTSP_HANDLE_MS', 'SCREEN_FLIPS', 'RTSP_OPTIONS', 'RTSP_STATUS'}:
                    transitions.append({'timestamp': stamp, 'event': event, 'value': value, 'bytes': size})
        summary = {}
        for event, rows in sorted(events.items()):
            values = [r[2] for r in rows]
            summary[event] = {'records': len(rows), 'first': rows[0][0], 'last': rows[-1][0],
                              'min': min(values), 'max': max(values), 'mean': sum(values)/len(values)}
            if event in DELTAS or event.startswith('RTP_RESEND_REJECT_') or event == 'RTP_RESEND_UNTRACKED_RECOVERED':
                summary[event]['delta_sum'] = sum(values)
            if event == 'RTP_RECOVERY_TOTAL':
                summary[event]['request_delta_sum'] = sum(r[3] for r in rows)
        duration = (datetime.fromisoformat(records[-1][0].replace('Z', '+00:00'))-datetime.fromisoformat(records[0][0].replace('Z', '+00:00'))).total_seconds()
        versions = [e.removeprefix('PKG_VERSION_') for e in events if e.startswith('PKG_VERSION_')]
        groups = []
        for epoch, rows in stream_records.items():
            groups.append({'connection_epoch': epoch, 'first': rows[0][0], 'last': rows[-1][0],
                          'event_counts': dict(Counter(r[1] for r in rows))})
        result.append({'versions': versions, 'first': records[0][0], 'last': records[-1][0],
                       'duration_seconds': duration, 'records': len(records),
                       'has_app_start': 'APP_START' in events, 'has_app_stop': 'APP_STOP' in events,
                       'cap_markers': len(events['LOG_SESSION_CAPPED']),
                       'wrap_markers': len(events['LOG_WRAPPED']),
                       'events': summary, 'timeline': transitions, 'connection_epochs': groups,
                       'playback_epochs': [{'connection_epoch': k[0], 'playback_epoch': k[1],
                        'first': v[0][0], 'last': v[-1][0], 'records': len(v)}
                        for k,v in playback_records.items()]})
    return {'sha256': hashlib.sha256(raw).hexdigest(), 'bytes': len(raw), 'unparsed_lines': bad,
            'semantics': SEMANTICS, 'sessions': result}

def plot(raw, target):
    # Dependency-free standalone SVG: timestamped interval concealment and gauges.
    rows = [LINE.fullmatch(s) for s in raw.decode(errors='replace').splitlines()]
    rows = [m.groups() for m in rows if m]
    start = max((i for i,r in enumerate(rows) if r[1] == 'APP_START'), default=0)
    rows = rows[start:]
    series = [('RTP_GAP_CONCEALED_TOTAL', '#d34b3c'), ('RTP_QUEUE_DEPTH', '#2777b9'), ('PCM_BUFFER_FRAMES', '#188457')]
    svg = ['<svg xmlns="http://www.w3.org/2000/svg" width="1000" height="650" viewBox="0 0 1000 650"><rect width="1000" height="650" fill="white"/>']
    if rows:
        origin = datetime.fromisoformat(rows[0][0].replace('Z', '+00:00')); span = max(1,(datetime.fromisoformat(rows[-1][0].replace('Z', '+00:00'))-origin).total_seconds())
        for n,(event,color) in enumerate(series):
            pts = [((datetime.fromisoformat(t.replace('Z', '+00:00'))-origin).total_seconds(),int(v)) for t,e,v,b in rows if e == event]
            maximum = max((v for _,v in pts),default=1) or 1
            y = 45+n*195
            svg.append(f'<text x="55" y="{y}" font-family="sans-serif">{event} (max {maximum}; x = elapsed seconds, 0–{span:g})</text>')
            xy = ' '.join(f'{55+t/span*900:.2f},{y+155-v/maximum*140:.2f}' for t,v in pts)
            svg.append(f'<path d="M55 {y+10} V{y+155} H955" fill="none" stroke="#888"/><polyline points="{xy}" fill="none" stroke="{color}" stroke-width="1"/>')
    svg.append('</svg>'); target.write_text('\n'.join(svg))

if __name__ == '__main__':
    p=argparse.ArgumentParser(description=__doc__); p.add_argument('log',type=Path); p.add_argument('--output',type=Path); p.add_argument('--plot',type=Path)
    a=p.parse_args(); raw=a.log.read_bytes(); out=json.dumps(analyze(raw),indent=2)+'\n'
    if a.output: a.output.write_text(out)
    else: print(out,end='')
    if a.plot: plot(raw,a.plot)
