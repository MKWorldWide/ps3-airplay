#!/usr/bin/env python3
"""Fail-closed validation for an AIR PS3 draft-release package and evidence."""

import argparse
import hashlib
import json
import re
import struct
from pathlib import Path


REQUIRED_CHECKS = (
    "host_tests_asan_ubsan", "ppu_crypto_and_receiver_build",
    "cellos_elf_validation", "sfo_version_title_id_and_boot_metadata",
    "pkg_extraction_structure_and_relative_paths",
    "embedded_artwork_byte_comparison", "pkg_envelope",
)


def digest(path):
    h = hashlib.sha256()
    with path.open("rb") as source:
        for block in iter(lambda: source.read(1024 * 1024), b""):
            h.update(block)
    return h.hexdigest()


def fail(message):
    raise SystemExit(f"release candidate rejected: {message}")


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("--package", type=Path, required=True)
    parser.add_argument("--evidence", type=Path, required=True)
    parser.add_argument("--expected-source", required=True)
    parser.add_argument("--expected-sha256", required=True)
    parser.add_argument("--release-config", type=Path, default=Path("release.json"))
    args = parser.parse_args()
    evidence = json.loads(args.evidence.read_text(encoding="utf-8"))
    config = json.loads(args.release_config.read_text(encoding="utf-8"))
    if evidence.get("schema") != 1:
        fail("unsupported evidence schema")
    if not re.fullmatch(r"[0-9a-f]{40}", args.expected_source):
        fail("expected source must be a full Git SHA")
    if evidence.get("source_revision") != args.expected_source:
        fail("evidence source revision mismatch")
    for key, ev_key in (("app_version", "app_version"), ("sfo_version", "sfo_app_ver"),
                        ("title_id", "title_id"), ("xmb_title", "xmb_title"),
                        ("raop_name", "raop_name"), ("package_format", "package_format")):
        if evidence.get(ev_key) != config.get(key):
            fail(f"{ev_key} differs from release.json")
    if evidence.get("sfo_version") != config.get("sfo_version"):
        fail("SFO VERSION and APP_VER differ")
    if evidence.get("content_id") != "UP0001-AIRPLAY02_00-0000000000000000":
        fail("content ID mismatch")
    if any(evidence.get("validation", {}).get(name) != "passed" for name in REQUIRED_CHECKS):
        fail("one or more source-build checks are not marked passed")
    expected_name = f"ps3-airplay-receiver-{config['app_version']}.pkg"
    if args.package.name != expected_name:
        fail("package filename does not match configured version")
    package_data = args.package.read_bytes()
    if len(package_data) < 0xC0 or package_data[:4] != b"\x7fPKG":
        fail("invalid or truncated package header")
    expected_type = 1 if config["package_format"] == "debug" else 0x80000001
    if struct.unpack_from(">I", package_data, 4)[0] != expected_type:
        fail("package type differs from release config")
    if struct.unpack_from(">Q", package_data, 24)[0] != len(package_data):
        fail("declared package size mismatch")
    count = struct.unpack_from(">I", package_data, 20)[0]
    if count < 1 or evidence["content_id"].encode("ascii") not in package_data[:0xC0]:
        fail("package item count or content ID is invalid")
    record = evidence.get("artifacts", {}).get("package", {})
    actual_hash = digest(args.package)
    if actual_hash != args.expected_sha256 or actual_hash != record.get("sha256"):
        fail("package SHA-256 mismatch")
    if len(package_data) != record.get("size_bytes"):
        fail("package size differs from evidence")
    for name in ("elf", "eboot", "sfo", "icon0", "pic1", "background_rgb"):
        item = evidence.get("artifacts", {}).get(name, {})
        if not re.fullmatch(r"[0-9a-f]{64}", item.get("sha256", "")) or item.get("size_bytes", 0) <= 0:
            fail(f"missing or invalid {name} artifact evidence")
    print(f"release candidate verified: {config['app_version']} / {config['sfo_version']}")
    print(f"source={args.expected_source} pkg_sha256={actual_hash} bytes={len(package_data)}")


if __name__ == "__main__":
    try:
        main()
    except (OSError, KeyError, ValueError, json.JSONDecodeError, struct.error) as exc:
        fail(str(exc))
