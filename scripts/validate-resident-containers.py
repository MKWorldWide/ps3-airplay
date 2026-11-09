#!/usr/bin/env python3
"""Distinguish production SPRX and RPCS3 fake-signed PS3DK containers."""

from __future__ import annotations

import argparse
import struct
from pathlib import Path


SCE_MAGIC = b"SCE\0"


def self_header(path: Path) -> tuple[int, int]:
    data = path.read_bytes()
    if len(data) < 12 or data[:4] != SCE_MAGIC:
        raise ValueError(f"{path.name}: missing/truncated SCE SELF header")
    return struct.unpack_from(">HH", data, 8)


def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument("--signed-sprx", type=Path, required=True)
    parser.add_argument("--fake-sprx", type=Path, required=True)
    args = parser.parse_args()

    signed_flags, signed_type = self_header(args.signed_sprx)
    fake_flags, fake_type = self_header(args.fake_sprx)
    if args.signed_sprx.read_bytes() == args.fake_sprx.read_bytes():
        raise SystemExit("production and emulator-only SPRX containers are identical")
    if signed_type != 1 or signed_flags not in (1, 0x000A):
        raise SystemExit(
            "production SPRX has unexpected SELF header "
            f"flags=0x{signed_flags:04x} type=0x{signed_type:04x}"
        )
    if fake_type != 1 or fake_flags != 0x8000:
        raise SystemExit(
            "RPCS3 fake SPRX has unexpected SELF header "
            f"flags=0x{fake_flags:04x} type=0x{fake_type:04x}"
        )
    print(f"production SPRX flags/type: 0x{signed_flags:04x}/0x{signed_type:04x} PASS")
    print(f"RPCS3 fake SPRX flags/type: 0x{fake_flags:04x}/0x{fake_type:04x} PASS")
    print("container identities are distinct: PASS")
    print("target-loader compatibility: NOT ESTABLISHED")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
