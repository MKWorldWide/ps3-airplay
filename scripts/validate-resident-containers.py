#!/usr/bin/env python3
"""Distinguish production SPRX and RPCS3 fake-signed PS3DK containers."""

from __future__ import annotations

import argparse
import struct
from pathlib import Path


SCE_MAGIC = b"SCE\0"
PS3_SPRX_TYPE = 0x00010001
PS3_FAKE_SPRX_TYPE = 0x80000001


def self_type(path: Path) -> int:
    data = path.read_bytes()
    if len(data) < 12 or data[:4] != SCE_MAGIC:
        raise ValueError(f"{path.name}: missing/truncated SCE SELF header")
    return struct.unpack_from(">I", data, 8)[0]


def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument("--signed-sprx", type=Path, required=True)
    parser.add_argument("--fake-sprx", type=Path, required=True)
    args = parser.parse_args()

    signed_type = self_type(args.signed_sprx)
    fake_type = self_type(args.fake_sprx)
    if args.signed_sprx.read_bytes() == args.fake_sprx.read_bytes():
        raise SystemExit("production and emulator-only SPRX containers are identical")
    if signed_type != PS3_SPRX_TYPE:
        raise SystemExit(
            f"production SPRX has unexpected SELF type 0x{signed_type:08x}"
        )
    if fake_type != PS3_FAKE_SPRX_TYPE:
        raise SystemExit(
            f"RPCS3 test SPRX has unexpected SELF type 0x{fake_type:08x}"
        )
    print(f"production SPRX SELF type: 0x{signed_type:08x} PASS")
    print(f"RPCS3 fake SPRX SELF type: 0x{fake_type:08x} PASS")
    print("container identities are distinct: PASS")
    print("target-loader compatibility: NOT ESTABLISHED")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
