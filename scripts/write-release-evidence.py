#!/usr/bin/env python3
"""Write path-free evidence after the complete Lima release gate succeeds."""

import argparse
import hashlib
import json
import re
from pathlib import Path


CONTENT_ID_PREFIX = "UP0001-AIRPLAY02_00-"


def sha256(path):
    digest = hashlib.sha256()
    with path.open("rb") as source:
        for block in iter(lambda: source.read(1024 * 1024), b""):
            digest.update(block)
    return digest.hexdigest()


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("--project-root", type=Path, required=True)
    parser.add_argument("--package", type=Path, required=True)
    parser.add_argument("--source-revision", required=True)
    parser.add_argument("--app-version", required=True)
    parser.add_argument("--sfo-version", required=True)
    parser.add_argument("--package-format", choices=("debug", "finalized"), required=True)
    parser.add_argument("--output", type=Path, required=True)
    args = parser.parse_args()
    if not re.fullmatch(r"[0-9a-f]{40}", args.source_revision):
        raise SystemExit("source revision must be a full Git SHA")
    config = json.loads((args.project_root / "release.json").read_text())
    if (args.app_version, args.sfo_version, args.package_format) != (
        config["app_version"], config["sfo_version"], config["package_format"]
    ):
        raise SystemExit("build identity differs from release.json")

    build_root = args.project_root / "build/lima-standalone-receiver"
    extract_root = args.project_root / "build/lima-standalone-package/extracted" / (
        CONTENT_ID_PREFIX + "0000000000000000"
    )
    artifact_paths = {
        "package": args.package,
        "elf": build_root / "ps3-airplay-receiver.elf",
        "build_info": build_root / "build-info.txt",
        "eboot": extract_root / "USRDIR/EBOOT.BIN",
        "sfo": extract_root / "PARAM.SFO",
        "icon0": extract_root / "ICON0.PNG",
        "pic1": extract_root / "PIC1.PNG",
        "background_rgb": extract_root / "USRDIR/background.rgb",
    }
    for label, path in artifact_paths.items():
        if not path.is_file():
            raise SystemExit(f"missing validated artifact: {label}")
    build_info = {}
    for line in artifact_paths["build_info"].read_text().splitlines():
        if "=" in line:
            key, value = line.split("=", 1)
            build_info[key] = value
    if build_info.get("project_revision") != args.source_revision:
        raise SystemExit("ELF build manifest source revision mismatch")
    if build_info.get("app_version") != args.app_version:
        raise SystemExit("ELF build manifest app version mismatch")
    if args.package.name != f"ps3-airplay-receiver-{args.app_version}.pkg":
        raise SystemExit("package filename does not match release version")

    artifacts = {}
    for label, path in artifact_paths.items():
        artifacts[label] = {"name": path.name, "size_bytes": path.stat().st_size, "sha256": sha256(path)}
    evidence = {
        "schema": 1,
        "source_revision": args.source_revision,
        "app_version": args.app_version,
        "sfo_app_ver": args.sfo_version,
        "sfo_version": args.sfo_version,
        "title_id": config["title_id"],
        "xmb_title": config["xmb_title"],
        "raop_name": config["raop_name"],
        "content_id": CONTENT_ID_PREFIX + "0000000000000000",
        "package_format": args.package_format,
        "validation": {
            "host_tests_asan_ubsan": "passed",
            "ppu_crypto_and_receiver_build": "passed",
            "cellos_elf_validation": "passed",
            "sfo_version_title_id_and_boot_metadata": "passed",
            "pkg_extraction_structure_and_relative_paths": "passed",
            "embedded_artwork_byte_comparison": "passed",
            "pkg_envelope": "passed",
        },
        "toolchain": {key: build_info[key] for key in (
            "pipeline", "ps3dk_version", "mbedtls_version", "ppu_compiler",
            "ppu_linker", "psl1ght_revision"
        ) if key in build_info},
        "artifacts": artifacts,
    }
    args.output.parent.mkdir(parents=True, exist_ok=True)
    args.output.write_text(json.dumps(evidence, indent=2, sort_keys=True) + "\n")
    sums = args.output.parent / "SHA256SUMS.txt"
    package_record = artifacts["package"]
    sums.write_text(
        f"{package_record['sha256']}  {package_record['name']}\n"
        f"{sha256(args.output)}  {args.output.name}\n"
    )
    print(f"release evidence written: {args.output.name}")
    print(f"source={args.source_revision} app={args.app_version} sfo={args.sfo_version}")
    print(f"pkg_sha256={artifacts['package']['sha256']} size={artifacts['package']['size_bytes']}")


if __name__ == "__main__":
    main()
