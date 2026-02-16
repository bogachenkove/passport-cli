#!/usr/bin/env python
"""
Verify GPG detached signatures for SHA256 and BLAKE2b checksum files.
This script imports a public key, then checks the authenticity of
the previously generated .asc signature files.
"""

import subprocess
from pathlib import Path
import sys

# Directory where this script resides.
SCRIPT_DIR = Path(__file__).resolve().parent

# Expected location of the public key file.
DEFAULT_PUBKEY = SCRIPT_DIR / "publickey.asc"

# Paths for SHA256 checksum and its signature.
OUTPUT_DIR_SHA256 = SCRIPT_DIR / "SHA256"
SHA256_SUMS = OUTPUT_DIR_SHA256 / "SHA256SUMS"
SHA256_ASC = OUTPUT_DIR_SHA256 / "SHA256SUMS.asc"

# Paths for BLAKE2b checksum and its signature.
OUTPUT_DIR_BLAKE2B = SCRIPT_DIR / "BLAKE2b"
BLAKE2B_SUMS = OUTPUT_DIR_BLAKE2B / "BLAKE2BSUMS"
BLAKE2B_ASC = OUTPUT_DIR_BLAKE2B / "BLAKE2BSUMS.asc"


def get_public_key_path() -> Path:
    """Obtain the path to the public key file.
    If the default 'publickey.asc' exists, use it.
    Otherwise, repeatedly prompt the user for a valid file path."""
    # Use the default key if it is already present in the script directory.
    if DEFAULT_PUBKEY.exists():
        print(f"[INFO] Using default public key: {DEFAULT_PUBKEY}")
        return DEFAULT_PUBKEY

    # Warn the user and ask for a custom location.
    print(f"[WARNING] Default public key not found: {DEFAULT_PUBKEY}")
    while True:
        user_input = input("Please enter the full path to your public key file (publickey.asc): ").strip()
        if not user_input:
            continue  # Empty input – ask again.
        path = Path(user_input).expanduser().resolve()
        if path.exists():
            return path
        print(f"[ERROR] File does not exist: {path}")


def import_key(key_path: Path) -> bool:
    """Import the public key into the local GPG keyring.
    Returns True on success, False on failure."""
    try:
        # Run gpg --import, capturing output to avoid cluttering the console.
        subprocess.run(["gpg", "--import", str(key_path)], check=True, capture_output=True, text=True)
        print(f"[OK] Public key imported from: {key_path}")
        return True
    except subprocess.CalledProcessError as e:
        # Show the error message from GPG to help the user diagnose the problem.
        print(f"[ERROR] Failed to import public key:\n{e.stderr}")
        return False


def verify_signature(asc_file: Path, data_file: Path) -> bool:
    """Verify a detached GPG signature.
    Returns True if the signature is valid, False otherwise."""
    # Ensure both required files exist before invoking GPG.
    if not asc_file.exists():
        print(f"[ERROR] Signature file not found: {asc_file}")
        return False
    if not data_file.exists():
        print(f"[ERROR] Data file not found: {data_file}")
        return False

    try:
        # --verify expects the signature file first, then the signed data.
        subprocess.run(
            ["gpg", "--verify", str(asc_file), str(data_file)],
            check=True,
            capture_output=True,
            text=True
        )
        print(f"[OK] Valid signature for {data_file.name}")
        return True
    except subprocess.CalledProcessError as e:
        # Print GPG's stderr to give detailed failure reason.
        print(f"[ERROR] Signature verification failed for {data_file.name}:\n{e.stderr}")
        return False


def main():
    # 1. Obtain and import the public key.
    pubkey_path = get_public_key_path()
    if not import_key(pubkey_path):
        sys.exit(1)  # Cannot proceed without a trusted public key.

    # 2. Verify both signatures.
    results = []
    results.append(verify_signature(SHA256_ASC, SHA256_SUMS))
    results.append(verify_signature(BLAKE2B_ASC, BLAKE2B_SUMS))

    # 3. Report overall status.
    if all(results):
        print("\n[SUCCESS] All signatures are valid.")
    else:
        print("\n[FAILURE] One or more signatures are invalid.")
        sys.exit(1)  # Exit with error code to indicate failure in automated environments.


if __name__ == "__main__":
    main()