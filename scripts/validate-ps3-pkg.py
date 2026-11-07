#!/usr/bin/env python3
"""Check the finalized NPDRM package envelope without unpacking it."""

from __future__ import annotations

import sys
from pathlib import Path


def fail(message: str) -> None:
    raise SystemExit(f"PKG validation failed: {message}")


def main() -> None:
    if len(sys.argv) != 3:
        fail("usage: validate-ps3-pkg.py PATH EXPECTED_CONTENT_ID")

    path = Path(sys.argv[1])
    expected_content_id = sys.argv[2].encode("ascii")
    data = path.read_bytes()
    if len(data) < 0xC0 or data[:4] != b"\x7fPKG":
        fail("missing or truncated PKG header")
    if int.from_bytes(data[4:8], "big") != 0x80000001:
        fail("expected finalized NPDRM package type 0x80000001")
    declared_size = int.from_bytes(data[24:32], "big")
    data_offset = int.from_bytes(data[32:40], "big")
    data_size = int.from_bytes(data[40:48], "big")
    item_count = int.from_bytes(data[20:24], "big")
    if declared_size != len(data):
        fail("header size does not match package size")
    if data_offset > declared_size or data_size > declared_size - data_offset:
        fail("package data region exceeds file size")
    if item_count < 1 or expected_content_id not in data[:0xC0]:
        fail("missing package items or expected content ID")

    print(
        f"PKG validation passed: {path.name}, {len(data)} bytes, "
        f"finalized NPDRM, {item_count} items"
    )


if __name__ == "__main__":
    main()
