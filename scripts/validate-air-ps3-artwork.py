#!/usr/bin/env python3
"""Validate the approved AIR PS3 source pixels and package artwork derivatives."""
from __future__ import annotations

import sys
from pathlib import Path
from PIL import Image


def fail(message: str) -> None:
    raise SystemExit(f"AIR PS3 artwork validation failed: {message}")


def load_rgb(path: Path) -> Image.Image:
    try:
        with Image.open(path) as image:
            image.load()
            return image.convert("RGB")
    except Exception as exc:
        fail(f"cannot decode {path.name}: {type(exc).__name__}")


def main() -> None:
    root = Path(sys.argv[1]) if len(sys.argv) == 2 else Path("assets/air-ps3")
    source_path = root / "artwork-source.png"
    source = load_rgb(source_path)
    if source.size != (1182, 665):
        fail(f"unexpected source dimensions {source.size}")

    for filename, size in (("ICON0.PNG", (320, 176)), ("PIC1.PNG", (1920, 1080))):
        candidate = root / filename
        image = load_rgb(candidate)
        if image.size != size:
            fail(f"{filename} dimensions are {image.size}, expected {size}")
        expected = source.resize(size, Image.Resampling.LANCZOS)
        if image.tobytes() != expected.tobytes():
            fail(f"{filename} does not match a direct resize of the approved artwork")
    print("AIR PS3 artwork validation passed: source pixels, ICON0 320x176, PIC1 1920x1080")


if __name__ == "__main__":
    main()
