#!/usr/bin/env python3
"""Read the checked-in AIR PS3 release version/configuration."""

import json
import re
import sys
from pathlib import Path


ROOT = Path(__file__).resolve().parents[1]
CONFIG = ROOT / "release.json"


def validate(config):
    if not re.fullmatch(r"(?:0|[1-9][0-9]*)\.(?:0|[1-9][0-9]*)\.(?:0|[1-9][0-9]*)", config["app_version"]):
        raise ValueError("app_version must be semantic X.Y.Z")
    if not re.fullmatch(r"[0-9]{2}\.[0-9]{2}", config["sfo_version"]):
        raise ValueError("sfo_version must be fixed-width XX.YY")
    if config["title_id"] != "AIRPLAY02" or config["xmb_title"] != "AIR PS3":
        raise ValueError("release identity changed without an explicit project decision")
    if config["raop_name"] != "AirPS3":
        raise ValueError("release RAOP name changed without an explicit project decision")
    if config["package_format"] != "debug":
        raise ValueError("public release candidate must be a validated debug PKG")
    return config


def load():
    return validate(json.loads(CONFIG.read_text(encoding="utf-8")))


if __name__ == "__main__":
    try:
        values = load()
        if len(sys.argv) == 1:
            print(json.dumps(values, sort_keys=True))
        elif len(sys.argv) == 2 and sys.argv[1] in values:
            print(values[sys.argv[1]])
        else:
            raise ValueError("usage: read-release-config.py [app_version|sfo_version|title_id|xmb_title|raop_name|package_format]")
    except (OSError, ValueError, KeyError, json.JSONDecodeError) as exc:
        raise SystemExit(f"release config error: {exc}")
