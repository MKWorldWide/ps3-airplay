#!/usr/bin/env python3
"""Set the CellOS LV2 ELF OS/ABI byte on a linked PS3 PPU executable."""

from pathlib import Path
import sys


ELFOSABI_CELLLV2 = 0x66
EM_PPC64 = 21


def main() -> int:
    if len(sys.argv) != 2:
        print(f"usage: {sys.argv[0]} PPU_ELF", file=sys.stderr)
        return 2

    path = Path(sys.argv[1])
    data = bytearray(path.read_bytes())
    if len(data) < 64 or data[:4] != b"\x7fELF":
        raise SystemExit(f"{path}: not an ELF64 file")
    if data[4] != 2 or data[5] != 2:
        raise SystemExit(f"{path}: expected ELF64 big-endian PPU output")
    if int.from_bytes(data[18:20], "big") != EM_PPC64:
        raise SystemExit(f"{path}: expected PowerPC64 machine type")
    if data[7] not in (0, ELFOSABI_CELLLV2):
        raise SystemExit(f"{path}: unexpected OS/ABI byte 0x{data[7]:02x}")

    old = data[7]
    data[7] = ELFOSABI_CELLLV2
    path.write_bytes(data)
    print(f"patched {path}: EI_OSABI 0x{old:02x} -> 0x{ELFOSABI_CELLLV2:02x}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
