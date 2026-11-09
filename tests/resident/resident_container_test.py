#!/usr/bin/env python3
"""Exercise fail-closed resident SELF-container classification."""

import subprocess
import sys
import tempfile
import unittest
from pathlib import Path


PROJECT_ROOT = Path(__file__).resolve().parents[2]
VALIDATOR = PROJECT_ROOT / "scripts" / "validate-resident-containers.py"


def self_header(container_type: int) -> bytes:
    return b"SCE\0" + (2).to_bytes(4, "big") + container_type.to_bytes(4, "big")


class ResidentContainerTest(unittest.TestCase):
    def run_validator(self, signed: bytes, fake: bytes) -> subprocess.CompletedProcess:
        with tempfile.TemporaryDirectory() as directory:
            root = Path(directory)
            signed_path = root / "resident.sprx"
            fake_path = root / "resident.fake.sprx"
            signed_path.write_bytes(signed)
            fake_path.write_bytes(fake)
            return subprocess.run(
                [
                    sys.executable,
                    str(VALIDATOR),
                    "--signed-sprx",
                    str(signed_path),
                    "--fake-sprx",
                    str(fake_path),
                ],
                text=True,
                capture_output=True,
                check=False,
            )

    def test_accepts_distinct_production_and_rpc3_types(self) -> None:
        result = self.run_validator(
            self_header(0x00010001) + b"production payload",
            self_header(0x80000001) + b"emulator payload",
        )
        self.assertEqual(result.returncode, 0, result.stderr)
        self.assertIn("target-loader compatibility: NOT ESTABLISHED", result.stdout)

    def test_rejects_fake_container_as_production(self) -> None:
        result = self.run_validator(
            self_header(0x80000001) + b"fake",
            self_header(0x80000001) + b"other fake",
        )
        self.assertNotEqual(result.returncode, 0)
        self.assertIn("production SPRX has unexpected SELF type", result.stderr)

    def test_rejects_non_self_header(self) -> None:
        result = self.run_validator(b"\x7fELF" + bytes(32), self_header(0x80000001))
        self.assertNotEqual(result.returncode, 0)
        self.assertIn("missing/truncated SCE SELF header", result.stderr)

    def test_rejects_identical_container_artifacts(self) -> None:
        artifact = self_header(0x00010001) + b"same"
        result = self.run_validator(artifact, artifact)
        self.assertNotEqual(result.returncode, 0)
        self.assertIn("containers are identical", result.stderr)


if __name__ == "__main__":
    unittest.main()
