#!/usr/bin/env python3
"""Exercise evidence acceptance and fail-closed mismatches with a fake PKG header."""

import hashlib
import json
import subprocess
import sys
import tempfile
from pathlib import Path


ROOT = Path(__file__).resolve().parents[1]
VERIFY = ROOT / "scripts/verify-release-evidence.py"
CONFIG = json.loads((ROOT / "release.json").read_text())
CONTENT_ID = b"UP0001-AIRPLAY02_00-0000000000000000"
CHECKS = (
    "host_tests_asan_ubsan", "ppu_crypto_and_receiver_build",
    "cellos_elf_validation", "sfo_version_title_id_and_boot_metadata",
    "pkg_extraction_structure_and_relative_paths",
    "embedded_artwork_byte_comparison", "pkg_envelope",
)


def invoke(pkg, evidence, sha):
    return subprocess.run(
        [sys.executable, str(VERIFY), "--package", str(pkg), "--evidence", str(evidence),
         "--expected-source", "a" * 40, "--expected-sha256", sha,
         "--release-config", str(ROOT / "release.json")],
        text=True, capture_output=True,
    )


with tempfile.TemporaryDirectory() as temp:
    root = Path(temp)
    pkg = root / f"ps3-airplay-receiver-{CONFIG['app_version']}.pkg"
    payload = bytearray(0xC0)
    payload[:4] = b"\x7fPKG"
    payload[4:8] = (1).to_bytes(4, "big")
    payload[20:24] = (5).to_bytes(4, "big")
    payload[24:32] = len(payload).to_bytes(8, "big")
    payload[32:40] = (0xC0).to_bytes(8, "big")
    payload[40:48] = (0).to_bytes(8, "big")
    payload[0x50:0x50 + len(CONTENT_ID)] = CONTENT_ID
    pkg.write_bytes(payload)
    fake_hash = "b" * 64
    evidence_data = {
        "schema": 1, "source_revision": "a" * 40,
        "app_version": CONFIG["app_version"], "sfo_app_ver": CONFIG["sfo_version"],
        "sfo_version": CONFIG["sfo_version"], "title_id": CONFIG["title_id"],
        "xmb_title": CONFIG["xmb_title"], "raop_name": CONFIG["raop_name"],
        "content_id": CONTENT_ID.decode(), "package_format": "debug",
        "validation": {name: "passed" for name in CHECKS},
        "artifacts": {"package": {"sha256": hashlib.sha256(payload).hexdigest(), "size_bytes": len(payload)}},
    }
    for name in ("elf", "eboot", "sfo", "icon0", "pic1", "background_rgb"):
        evidence_data["artifacts"][name] = {"sha256": fake_hash, "size_bytes": 1}
    evidence = root / "release-evidence.json"
    evidence.write_text(json.dumps(evidence_data))
    digest = hashlib.sha256(payload).hexdigest()
    result = invoke(pkg, evidence, digest)
    assert result.returncode == 0, result.stderr
    result = invoke(pkg, evidence, "0" * 64)
    assert result.returncode != 0 and "SHA-256 mismatch" in result.stderr
    evidence_data["validation"]["pkg_envelope"] = "pending"
    evidence.write_text(json.dumps(evidence_data))
    result = invoke(pkg, evidence, digest)
    assert result.returncode != 0 and "not marked passed" in result.stderr

print("release evidence verifier passed valid identity and rejected hash/status mismatches")
