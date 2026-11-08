#!/usr/bin/env python3
"""Check the checked-in release identity and the next patch mapping."""

import importlib.util
from pathlib import Path


script = Path(__file__).with_name("read-release-config.py")
spec = importlib.util.spec_from_file_location("release_config", script)
module = importlib.util.module_from_spec(spec)
spec.loader.exec_module(module)
current = module.load()
assert current["app_version"] == "1.0.0"
assert current["sfo_version"] == "01.00"
assert current["title_id"] == "AIRPLAY02"
assert current["xmb_title"] == "AIR PS3"
assert current["raop_name"] == "AirPS3"
next_release = dict(current, app_version="1.0.1", sfo_version="01.01")
assert module.validate(next_release)["app_version"] == "1.0.1"
for invalid in ("1.0.01", "01.01", "1.0", "1.0.1.0"):
    try:
        module.validate(dict(current, app_version=invalid))
    except ValueError:
        pass
    else:
        raise AssertionError(f"invalid semantic version accepted: {invalid}")
print("release version mapping passed: 1.0.0/01.00 -> 1.0.1/01.01")
