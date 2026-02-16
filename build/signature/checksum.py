#!/usr/bin/env python
"""
Generate SHA256 and BLAKE2b checksums for all .cpp and .hpp files
in the source directory and save them to separate checksum files.
"""

import hashlib
from pathlib import Path
import sys

# Absolute path to the directory containing this script.
SCRIPT_DIR = Path(__file__).resolve().parent

# Project root is two levels up from the script directory.
PROJECT_ROOT = SCRIPT_DIR.parent.parent

# Directory containing the source code files to checksum.
SOURCE_DIR = PROJECT_ROOT / "source"

# Output directory and file for SHA256 checksums.
OUTPUT_DIR_SHA256 = SCRIPT_DIR / "SHA256"
OUTPUT_FILE_SHA256 = OUTPUT_DIR_SHA256 / "SHA256SUMS"

# Output directory and file for BLAKE2b checksums.
OUTPUT_DIR_BLAKE2B = SCRIPT_DIR / "BLAKE2b"
OUTPUT_FILE_BLAKE2B = OUTPUT_DIR_BLAKE2B / "BLAKE2BSUMS"


def calculate_sha256(file_path: Path) -> str:
    """Calculate SHA‑256 hash of a file.
    Reads the entire file in binary mode and returns the
    hexadecimal digest."""
    data = file_path.read_bytes()
    return hashlib.sha256(data).hexdigest()


def calculate_blake2b(file_path: Path) -> str:
    """Calculate BLAKE2b hash of a file.
    Reads the entire file in binary mode and returns the
    hexadecimal digest."""
    data = file_path.read_bytes()
    return hashlib.blake2b(data).hexdigest()


def main():
    # Exit if the source directory does not exist.
    if not SOURCE_DIR.exists():
        print(f"[ERROR] Source directory not found: {SOURCE_DIR}")
        sys.exit(1)

    # Create output directories if they do not already exist.
    OUTPUT_DIR_SHA256.mkdir(parents=True, exist_ok=True)
    OUTPUT_DIR_BLAKE2B.mkdir(parents=True, exist_ok=True)

    # Collect all files with .cpp or .hpp extension recursively.
    files = []
    for file_path in SOURCE_DIR.rglob("*"):
        if file_path.suffix.lower() in {".cpp", ".hpp"}:
            files.append(file_path)

    # Sort files alphabetically by their relative path to ensure
    # a deterministic order in the output checksum files.
    files.sort(key=lambda p: p.relative_to(SOURCE_DIR).as_posix())

    # Open both checksum files simultaneously and write the results.
    with OUTPUT_FILE_SHA256.open("w", encoding="utf-8") as sha_file, \
         OUTPUT_FILE_BLAKE2B.open("w", encoding="utf-8") as blake_file:

        for file_path in files:
            # Compute relative path for consistent output format.
            rel_path = file_path.relative_to(SOURCE_DIR).as_posix()
            sha256_hash = calculate_sha256(file_path)
            blake2b_hash = calculate_blake2b(file_path)

            # Write hash and path in standard format: "<hash>  <path>"
            sha_file.write(f"{sha256_hash}  {rel_path}\n")
            blake_file.write(f"{blake2b_hash}  {rel_path}\n")

    print(f"[OK] SHA256 sums written to: {OUTPUT_FILE_SHA256}")
    print(f"[OK] BLAKE2b sums written to: {OUTPUT_FILE_BLAKE2B}")


if __name__ == "__main__":
    main()