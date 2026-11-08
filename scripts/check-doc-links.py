#!/usr/bin/env python3
"""Check local Markdown links across AIR PS3's public documentation."""
from __future__ import annotations

import re
import sys
from pathlib import Path
from urllib.parse import unquote, urlsplit

ROOT = Path(__file__).resolve().parents[1]
LINK = re.compile(r"!?\[[^\]]*\]\(([^)]+)\)")
REFERENCE = re.compile(r"^\[[^\]]+\]:\s*(\S+)", re.MULTILINE)
FENCE = re.compile(r"^\s*(```|~~~)")
SCAN = (ROOT / "README.md", ROOT / "CONTRIBUTING.md", ROOT / "THIRD_PARTY_NOTICES.md")
scan_paths = list(SCAN)
for directory in ("docs", "prototype", "experimental"):
    scan_paths.extend((ROOT / directory).rglob("*.md"))

missing = []
count = 0
for file in scan_paths:
    if not file.exists():
        continue
    text = file.read_text(encoding="utf-8")
    visible_lines = []
    fenced = False
    for line in text.splitlines():
        if FENCE.match(line):
            fenced = not fenced
            continue
        if not fenced:
            visible_lines.append(line)
    outside_code = "\n".join(visible_lines)
    matches = [m.group(1) for m in LINK.finditer(outside_code)]
    matches.extend(m.group(1) for m in REFERENCE.finditer(outside_code))
    for raw in matches:
        count += 1
        link = unquote(raw.strip("<>").split(' "', 1)[0].strip())
        parts = urlsplit(link)
        if parts.scheme or parts.netloc or not parts.path:
            continue
        if parts.path.startswith("/"):
            missing.append((file, raw, "absolute path; use repository-relative link"))
            continue
        target = (file.parent / parts.path).resolve()
        if not target.is_relative_to(ROOT.resolve()) or not target.exists():
            missing.append((file, raw, "not found"))

for file, link, reason in missing:
    print(f"{file.relative_to(ROOT)}: {link}: {reason}", file=sys.stderr)

print(f"Checked {count} Markdown links in {len(scan_paths)} files; broken local links: {len(missing)}")
sys.exit(bool(missing))
