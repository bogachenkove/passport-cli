#!/usr/bin/env python
"""
Sign the generated SHA256 and BLAKE2b checksum files using GPG,
creating detached ASCII-armored signatures (.asc files).
"""

import subprocess
from pathlib import Path
import sys

# Directory where this script resides.
SCRIPT_DIR = Path(__file__).resolve().parent

# Paths for SHA256 checksum file and its expected signature.
OUTPUT_DIR_SHA256 = SCRIPT_DIR / "SHA256"
OUTPUT_FILE_SHA256 = OUTPUT_DIR_SHA256 / "SHA256SUMS"
OUTPUT_FILE_SHA256_ASC = OUTPUT_FILE_SHA256.with_suffix(".asc")

# Paths for BLAKE2b checksum file and its expected signature.
OUTPUT_DIR_BLAKE2B = SCRIPT_DIR / "BLAKE2b"
OUTPUT_FILE_BLAKE2B = OUTPUT_DIR_BLAKE2B / "BLAKE2BSUMS"
OUTPUT_FILE_BLAKE2B_ASC = OUTPUT_FILE_BLAKE2B.with_suffix(".asc")


def sign_file(file_path: Path, key_id: str = None):
    """Sign a file with GPG (detached ASCII signature).
    Creates a .asc file alongside the original.
    If key_id is provided, it selects a specific GPG key."""
    # Abort if the file to be signed does not exist.
    if not file_path.exists():
        print(f"[ERROR] File not found: {file_path}")
        return False

    # Build the GPG command: armor, detached signature, optional key selection.
    cmd = ["gpg", "--armor", "--detach-sign"]
    if key_id:
        cmd.extend(["--local-user", key_id])
    cmd.append(str(file_path))

    try:
        # Execute the GPG command and check for success.
        subprocess.run(cmd, check=True)
        print(f"[OK] Signed: {file_path} -> {file_path}.asc")
        return True
    except subprocess.CalledProcessError as e:
        print(f"[ERROR] Failed to sign {file_path}: {e}")
        return False


def main():
    # Sign the SHA256 checksum file.
    sign_file(OUTPUT_FILE_SHA256)

    # Sign the BLAKE2b checksum file.
    sign_file(OUTPUT_FILE_BLAKE2B)


if __name__ == "__main__":
    main()