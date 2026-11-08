#!/usr/bin/env python3
"""Validate extracted PS3 launch metadata, including fixed-width versions."""
import re
import struct
import sys
from pathlib import Path


def validate(path, expected_version, expected_title=None):
    data = Path(path).read_bytes()
    if len(data) < 20 or data[:4] != b'\x00PSF':
        raise ValueError('invalid SFO header')
    _, keys, values, count = struct.unpack_from('<4I', data, 4)
    fields = {}
    for i in range(count):
        key_off, fmt, size, capacity, value_off = struct.unpack_from('<HHIII', data, 20 + i * 16)
        key_start = keys + key_off
        key_end = data.index(0, key_start)
        name = data[key_start:key_end].decode('ascii')
        start = values + value_off
        if size > capacity or start + capacity > len(data):
            raise ValueError('SFO value exceeds its allocation')
        raw = data[start:start + size]
        fields[name] = raw.rstrip(b'\0').decode('utf-8') if fmt == 0x204 else int.from_bytes(raw, 'little')
    for key in ('APP_VER', 'VERSION'):
        if not re.fullmatch(r'[0-9]{2}\.[0-9]{2}', str(fields.get(key, ''))):
            raise ValueError(f'{key} must use XX.YY')
        if fields[key] != expected_version:
            raise ValueError(f'{key} differs from requested SFO version')
    for key, expected in {'TITLE_ID': 'AIRPLAY02', 'CATEGORY': 'HG', 'BOOTABLE': 1}.items():
        if fields.get(key) != expected:
            raise ValueError(f'unexpected {key}')
    if expected_title is not None and fields.get('TITLE') != expected_title:
        raise ValueError('unexpected TITLE')
    print(f'SFO launch metadata passed: AIRPLAY02, {expected_version}, {fields.get("TITLE")}')


if __name__ == '__main__':
    try:
        validate(sys.argv[1], sys.argv[2], sys.argv[3] if len(sys.argv) > 3 else None)
    except (ValueError, IndexError, struct.error, UnicodeError) as exc:
        raise SystemExit(f'SFO validation failed: {exc}')
