#!/usr/bin/env python3
"""Validate a standalone PS3 PPU ELF against the known-good 01.01 layout."""

from __future__ import annotations

import struct
import sys
from pathlib import Path


EXPECTED_LOAD_FLAGS = (0x400005, 0x600006, 0x400004, 0x600006)


def fail(message: str) -> None:
    raise SystemExit(f"ELF validation failed: {message}")


def main() -> None:
    if len(sys.argv) != 2:
        fail("usage: validate-ps3-elf.py PATH")

    path = Path(sys.argv[1])
    data = path.read_bytes()
    if len(data) < 64 or data[:4] != b"\x7fELF":
        fail("missing or truncated ELF header")

    ident = data[:16]
    if ident[4] != 2 or ident[5] != 2 or ident[6] != 1:
        fail("expected ELF64, big-endian, current ELF version")
    if ident[7] != 0x66:
        fail(f"expected CellOS OSABI 0x66, got 0x{ident[7]:02x}")

    header = struct.unpack_from(">HHIQQQIHHHHHH", data, 16)
    elf_type, machine = header[0], header[1]
    entry, phoff, flags = header[3], header[4], header[6]
    ehsize, phentsize, phnum = header[7], header[8], header[9]
    if (elf_type, machine, entry, flags) != (2, 21, 0x10000000, 0):
        fail("expected ET_EXEC PPC64, entry 0x10000000, and zero e_flags")
    if ehsize != 64 or phentsize != 56 or phnum < 5:
        fail("unexpected ELF/program-header layout")
    if phoff + phentsize * phnum > len(data):
        fail("program-header table exceeds file size")

    load_flags = []
    for index in range(phnum):
        p_type, p_flags = struct.unpack_from(">II", data, phoff + index * phentsize)
        if p_type == 1:
            load_flags.append(p_flags)
    if tuple(load_flags[:4]) != EXPECTED_LOAD_FLAGS:
        rendered = ", ".join(f"0x{x:x}" for x in load_flags[:4])
        fail(f"PT_LOAD flags differ from 01.01 reference: {rendered}")
    if 0x6600006 not in load_flags:
        fail("missing zero-sized PS3 special PT_LOAD segment")

    print(
        f"ELF validation passed: {path.name}, {len(data)} bytes, "
        f"CellOS PPC64 executable, {len(load_flags)} PT_LOAD segments"
    )


if __name__ == "__main__":
    main()
