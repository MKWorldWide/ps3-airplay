#!/usr/bin/env python3
"""Convert the public diagnostic JPEG into a fixed-size RGB runtime asset."""

from pathlib import Path
import sys

from PIL import Image

WIDTH = 1280
HEIGHT = 720


def main() -> int:
	if len(sys.argv) != 3:
		print(f"usage: {sys.argv[0]} INPUT.jpg OUTPUT.rgb", file=sys.stderr)
		return 2

	image_path = Path(sys.argv[1])
	output_path = Path(sys.argv[2])
	with Image.open(image_path) as image:
		image = image.convert("RGB")
		image = image.resize((WIDTH, HEIGHT), Image.Resampling.LANCZOS)
		output_path.parent.mkdir(parents=True, exist_ok=True)
		output_path.write_bytes(image.tobytes())
	if output_path.stat().st_size != WIDTH * HEIGHT * 3:
		print("unexpected RGB output size", file=sys.stderr)
		return 1
	return 0


if __name__ == "__main__":
	raise SystemExit(main())
