#!/usr/bin/env python3
"""
Build script for Passport-CLI project.
Allows selection of build type, compiler, and build system.
Builds directly in the directory containing the build files.
"""

import os
import platform
import shutil
import subprocess
import sys
import hashlib
from pathlib import Path

# Optional color output
try:
    from colorama import init, Fore, Style
    init(autoreset=True)
    COLORAMA_AVAILABLE = True
except ImportError:
    COLORAMA_AVAILABLE = False
    class Fore:
        RED = GREEN = YELLOW = CYAN = RESET = ''
    class Style:
        BRIGHT = RESET_ALL = ''

def print_color(message, color=Fore.RESET, brightness=Style.RESET_ALL):
    if COLORAMA_AVAILABLE:
        print(f"{brightness}{color}{message}{Style.RESET_ALL}")
    else:
        print(message)

def error_exit(message):
    print_color(f"Error: {message}", Fore.RED, Style.BRIGHT)
    sys.exit(1)

def require_command(cmd_name):
    """
    Check if a command is available. If not, offer to update package lists and
    prompt user to enter an installation command, execute it, and re-check.
    Loop until command is found or user aborts with Q.
    """
    while True:
        if shutil.which(cmd_name) is not None:
            return True
        print_color(f"Required command '{cmd_name}' not found in PATH.", Fore.YELLOW)

        # Ask to update package lists
        update_choice = get_choice("Update package lists before installing?", "YN", default="N")
        if update_choice == 'Y':
            print_color("Enter update command (e.g., 'sudo apt update'):", Fore.CYAN)
            update_cmd = input("> ").strip()
            if update_cmd:
                result = subprocess.run(update_cmd, shell=True)
                if result.returncode != 0:
                    print_color(f"Update command failed with exit code {result.returncode}.", Fore.RED)

        # Ask for installation command
        print_color(f"Enter installation command for '{cmd_name}' (or press Enter to abort):", Fore.CYAN)
        install_cmd = input("> ").strip()
        if not install_cmd:
            error_exit(f"Aborted due to missing '{cmd_name}'.")
        result = subprocess.run(install_cmd, shell=True)
        if result.returncode != 0:
            print_color(f"Installation command failed with exit code {result.returncode}.", Fore.RED)
            # Loop again

def get_choice(prompt, options, default=None):
    """
    Universal choice selector.
    prompt: text prompt (e.g., "Build type (Debug/Release)")
    options: string of allowed characters (e.g., "DR")
    default: default value if user presses Enter
    Returns selected character in uppercase.
    Pressing Q at any time aborts the script.
    """
    options_upper = options.upper()
    if default:
        default = default.upper()
        if default not in options_upper:
            raise ValueError(f"Default '{default}' not in allowed options '{options_upper}'")
        prompt = f"{prompt} [{options_upper}] (default: {default}): "
    else:
        prompt = f"{prompt} [{options_upper}]: "

    while True:
        user_input = input(prompt).strip().upper()
        if user_input == 'Q':
            print_color("Aborted by user.", Fore.RED)
            sys.exit(1)
        if not user_input and default:
            return default
        if len(user_input) == 1 and user_input in options_upper:
            return user_input
        print_color(f"Invalid input. Expected one of: {options_upper} (or Q to quit)", Fore.YELLOW)

def get_build_config():
    """Loop to select build configuration with confirmation."""
    while True:
        print_color("\n--- Build Configuration ---", Fore.CYAN, Style.BRIGHT)
        build_type = get_choice("Build type (Debug/Release)", "DR", default="R")
        compiler = get_choice("Compiler (G++/Clang++)", "GC", default="G")
        builder = get_choice("Build system (CMake/Make)", "CM", default="C")

        type_map = {'D': 'Debug', 'R': 'Release'}
        compiler_map = {'G': 'G++', 'C': 'Clang++'}
        builder_map = {'C': 'CMake', 'M': 'Make'}

        print_color("\n--- Selected Configuration ---", Fore.CYAN)
        print(f"  Build type:   {type_map[build_type]}")
        print(f"  Compiler:     {compiler_map[compiler]}")
        print(f"  Build system: {builder_map[builder]}")

        confirm = get_choice("Proceed with this configuration?", "YN", default="Y")
        if confirm == 'Y':
            return build_type, compiler, builder
        else:
            print_color("Re-enter configuration...", Fore.YELLOW)

def locate_build_file(builder):
    """
    Locate the build system file (CMakeLists.txt or Makefile).
    If not found in current directory, prompt user for path.
    Also check for leftover build directories (CMakeFiles or MakeFiles) and remove them.
    Returns the absolute path to the directory containing the file.
    """
    script_dir = Path.cwd()
    if builder == 'C':
        filename = "CMakeLists.txt"
        leftover_dir = "CMakeFiles"
    else:
        filename = "Makefile"
        leftover_dir = "MakeFiles"

    # Check current directory
    candidate = script_dir / filename
    if candidate.exists():
        print_color(f"Found {filename} in current directory.", Fore.GREEN)
        src_dir = script_dir
    else:
        print_color(f"{filename} not found in current directory.", Fore.YELLOW)
        while True:
            user_path = input(f"Enter path to directory containing {filename}: ").strip()
            if not user_path:
                continue
            path = Path(user_path).expanduser().resolve()
            if (path / filename).exists():
                src_dir = path
                print_color(f"Using {filename} from {src_dir}", Fore.GREEN)
                break
            else:
                print_color(f"{filename} not found in {path}. Try again.", Fore.YELLOW)

    # Remove leftover build directories if present
    leftover_path = src_dir / leftover_dir
    if leftover_path.exists() and leftover_path.is_dir():
        print_color(f"Removing existing {leftover_dir} from source directory.", Fore.YELLOW)
        shutil.rmtree(leftover_path)

    # Also remove CMake cache and other artifacts for a clean build
    if builder == 'C':
        cache = src_dir / "CMakeCache.txt"
        if cache.exists():
            cache.unlink()
        cmake_install = src_dir / "cmake_install.cmake"
        if cmake_install.exists():
            cmake_install.unlink()

    return src_dir

def build_with_cmake(build_dir, build_type, compiler):
    """Build using CMake. Returns True on success, False on failure."""
    require_command('cmake')
    if compiler == 'G':
        require_command('g++')
    else:
        require_command('clang++')

    print_color("\n--- Running CMake ---", Fore.CYAN, Style.BRIGHT)
    os.chdir(build_dir)

    cmake_cmd = ['cmake', '.']
    cmake_cmd.append(f'-DCMAKE_BUILD_TYPE={"Debug" if build_type=="D" else "Release"}')
    if compiler == 'G':
        cmake_cmd.append('-DCMAKE_CXX_COMPILER=g++')
    else:  # Clang++
        cmake_cmd.extend([
            '-DCMAKE_CXX_COMPILER=clang++',
            '-DCMAKE_CXX_FLAGS=-stdlib=libstdc++'
        ])

    print_color(f"Executing: {' '.join(cmake_cmd)}", Fore.YELLOW)
    result = subprocess.run(cmake_cmd)
    if result.returncode != 0:
        print_color("CMake generation failed.", Fore.RED)
        return False

    print_color("\n--- Compiling (cmake --build) ---", Fore.CYAN, Style.BRIGHT)
    build_cmd = ['cmake', '--build', '.', '--parallel']
    result = subprocess.run(build_cmd)
    if result.returncode != 0:
        print_color("Compilation failed.", Fore.RED)
        return False

    print_color("CMake build completed successfully.", Fore.GREEN)
    return True

def build_with_make(build_dir, build_type, compiler):
    """Build using Make. Returns True on success, False on failure."""
    require_command('make')
    if compiler == 'G':
        require_command('g++')
    else:
        require_command('clang++')

    print_color("\n--- Running Make ---", Fore.CYAN, Style.BRIGHT)
    os.chdir(build_dir)

    env = os.environ.copy()
    if compiler == 'G':
        env['CXX'] = 'g++'
    else:
        # For clang++, add -stdlib=libstdc++ to the compiler command itself
        env['CXX'] = 'clang++ -stdlib=libstdc++'
    env['BUILD'] = 'debug' if build_type == 'D' else 'release'

    make_cmd = ['make', '--jobs']
    print_color(f"Executing: {' '.join(make_cmd)} with BUILD={env['BUILD']}, CXX={env.get('CXX','')}", Fore.YELLOW)
    result = subprocess.run(make_cmd, env=env)
    if result.returncode != 0:
        print_color("Make build failed.", Fore.RED)
        return False

    print_color("Make build completed successfully.", Fore.GREEN)
    return True

def prompt_install_commands():
    """
    Let the user enter shell commands to install dependencies.
    Returns after the first successful command (returncode 0) or when user enters an empty line.
    Failed commands do not exit the prompt.
    """
    print_color("Enter installation commands (one per line).", Fore.CYAN)
    print_color("After a successful command, the build will be retried automatically.", Fore.CYAN)
    print_color("Press Enter on an empty line to abort this installation attempt.", Fore.CYAN)
    while True:
        cmd = input("> ").strip()
        if not cmd:
            # User aborts this installation round
            print_color("No command entered, continuing with build attempt.", Fore.YELLOW)
            return
        result = subprocess.run(cmd, shell=True)
        if result.returncode == 0:
            print_color("Command succeeded. Retrying build...", Fore.GREEN)
            return  # Success, exit prompt and retry build
        else:
            print_color(f"Command failed with exit code {result.returncode}. You may try another command.", Fore.RED)

def run_build_with_retry(builder, build_dir, build_type, compiler):
    """Attempt build up to 3 times, prompting for dependency installation on failure."""
    max_attempts = 3
    for attempt in range(1, max_attempts + 1):
        print_color(f"\n--- Build attempt {attempt}/{max_attempts} ---", Fore.CYAN, Style.BRIGHT)
        success = False
        if builder == 'C':
            success = build_with_cmake(build_dir, build_type, compiler)
        else:
            success = build_with_make(build_dir, build_type, compiler)

        if success:
            return True

        if attempt < max_attempts:
            print_color("Build failed. You may install missing dependencies.", Fore.YELLOW)
            prompt_install_commands()
            print_color("Retrying build...", Fore.CYAN)
        else:
            error_exit("Build failed after 3 attempts. Aborting.")

def compute_hashes(file_path):
    """Compute BLAKE2b and SHA256 hashes of a file."""
    if not file_path.exists():
        return None, None
    with open(file_path, 'rb') as f:
        data = f.read()
    blake2 = hashlib.blake2b(data).hexdigest()
    sha256 = hashlib.sha256(data).hexdigest()
    return blake2, sha256

def verify_signature(exe_path):
    """
    Verify the built executable against signed checksums.
    Returns True if verification succeeds, False if user skips.
    """
    print_color("\n--- Signature Verification ---", Fore.CYAN, Style.BRIGHT)
    verify_choice = get_choice("Verify executable signature?", "YN", default="Y")
    if verify_choice == 'N':
        return False

    require_command('gpg')

    # Default signature directory is "signature" next to the script
    script_dir = Path.cwd()
    sig_dir_default = script_dir / "signature"

    # Required files structure
    required_files = {
        'public_key': ('publickey.asc', None),
        'sha256_sums': ('SHA256/SHA256SUMS', None),
        'sha256_sums_asc': ('SHA256/SHA256SUMS.asc', None),
        'blake2b_sums': ('BLAKE2b/BLAKE2BSUMS', None),
        'blake2b_sums_asc': ('BLAKE2b/BLAKE2BSUMS.asc', None)
    }

    # First, check if default signature directory exists and contains files
    def check_file(rel_path):
        full = sig_dir_default / rel_path
        return full if full.exists() else None

    for key, (rel_path, _) in required_files.items():
        found = check_file(rel_path)
        required_files[key] = (rel_path, found)

    # For any missing file, prompt user for its path
    missing = [(key, rel_path) for key, (rel_path, path) in required_files.items() if path is None]
    if missing:
        print_color("Some signature files are missing. Please provide paths.", Fore.YELLOW)
        for key, rel_path in missing:
            while True:
                user_input = input(f"Enter path to {rel_path}: ").strip()
                if not user_input:
                    continue
                user_path = Path(user_input).expanduser().resolve()
                if user_path.exists():
                    required_files[key] = (rel_path, user_path)
                    break
                else:
                    print_color(f"File not found: {user_path}", Fore.YELLOW)

    # Extract actual paths
    pubkey_path = required_files['public_key'][1]
    sha256_sums_path = required_files['sha256_sums'][1]
    sha256_sums_asc_path = required_files['sha256_sums_asc'][1]
    blake2b_sums_path = required_files['blake2b_sums'][1]
    blake2b_sums_asc_path = required_files['blake2b_sums_asc'][1]

    # Import public key if not already in keyring
    print_color("Checking public key...", Fore.CYAN)
    # Extract key ID from public key file
    try:
        result = subprocess.run(['gpg', '--with-colons', '--import-options', 'show-only', '--import', str(pubkey_path)],
                                capture_output=True, text=True, check=False)
        # Parse fingerprint from output (simplified: look for fpr record)
        key_id = None
        for line in result.stdout.splitlines():
            if line.startswith('fpr:'):
                parts = line.split(':')
                if len(parts) > 9:
                    key_id = parts[9]  # fingerprint
                    break
        if not key_id:
            print_color("Could not extract key ID from public key file.", Fore.YELLOW)
            key_id = None
    except Exception as e:
        print_color(f"Error reading public key: {e}", Fore.YELLOW)
        key_id = None

    # Check if key is already in keyring
    key_present = False
    if key_id:
        result = subprocess.run(['gpg', '--list-keys', '--with-colons', key_id],
                                capture_output=True, text=True, check=False)
        key_present = result.returncode == 0

    if not key_present:
        print_color("Importing public key...", Fore.CYAN)
        result = subprocess.run(['gpg', '--import', str(pubkey_path)], capture_output=True, text=True)
        if result.returncode != 0:
            print_color(f"Failed to import public key: {result.stderr}", Fore.RED)
            return False
        else:
            print_color("Public key imported.", Fore.GREEN)
    else:
        print_color("Public key already present in keyring.", Fore.GREEN)

    # Verify signatures
    print_color("\nVerifying SHA256 checksums signature...", Fore.CYAN)
    result = subprocess.run(['gpg', '--verify', str(sha256_sums_asc_path), str(sha256_sums_path)],
                            capture_output=True, text=True)
    if result.returncode != 0:
        print_color(f"SHA256 signature verification failed: {result.stderr}", Fore.RED)
        return False
    else:
        print_color("SHA256 signature is valid.", Fore.GREEN)

    print_color("Verifying BLAKE2b checksums signature...", Fore.CYAN)
    result = subprocess.run(['gpg', '--verify', str(blake2b_sums_asc_path), str(blake2b_sums_path)],
                            capture_output=True, text=True)
    if result.returncode != 0:
        print_color(f"BLAKE2b signature verification failed: {result.stderr}", Fore.RED)
        return False
    else:
        print_color("BLAKE2b signature is valid.", Fore.GREEN)

    # Compute hashes of built executable
    exe_name = "passport-cli"
    exe_full = exe_path / exe_name if exe_path.is_dir() else exe_path
    if not exe_full.exists():
        print_color(f"Executable {exe_name} not found at {exe_full}", Fore.RED)
        return False

    blake2_hash, sha256_hash = compute_hashes(exe_full)
    if not blake2_hash or not sha256_hash:
        print_color("Failed to compute hashes.", Fore.RED)
        return False

    # Check SHA256
    print_color("\nChecking SHA256 hash against checksums file...", Fore.CYAN)
    with open(sha256_sums_path, 'r') as f:
        sha256_content = f.read()
    # Expect format: "hash  filename" or "hash *filename"
    found = False
    for line in sha256_content.splitlines():
        parts = line.strip().split()
        if len(parts) >= 2:
            file_hash = parts[0]
            # filename might have * or space
            if exe_name in line and sha256_hash == file_hash:
                found = True
                break
    if found:
        print_color("SHA256 hash matches.", Fore.GREEN)
    else:
        print_color("SHA256 hash does NOT match any entry in checksums file.", Fore.RED)
        return False

    # Check BLAKE2b
    print_color("Checking BLAKE2b hash against checksums file...", Fore.CYAN)
    with open(blake2b_sums_path, 'r') as f:
        blake2b_content = f.read()
    found = False
    for line in blake2b_content.splitlines():
        parts = line.strip().split()
        if len(parts) >= 2:
            file_hash = parts[0]
            if exe_name in line and blake2_hash == file_hash:
                found = True
                break
    if found:
        print_color("BLAKE2b hash matches.", Fore.GREEN)
    else:
        print_color("BLAKE2b hash does NOT match any entry in checksums file.", Fore.RED)
        return False

    print_color("\nAll signature and hash verifications passed.", Fore.GREEN, Style.BRIGHT)
    return True

def main():
    # Check if running as root
    if os.geteuid() == 0:
        error_exit("This script must not be run as root. Please run as a normal user.")

    # System information
    system = platform.system()
    release = platform.release()
    print_color(f"System: {system} {release}", Fore.CYAN)

    print_color("Passport-CLI build script", Fore.GREEN, Style.BRIGHT)

    build_type, compiler, builder = get_build_config()

    # Locate the build system file and clean leftovers
    build_dir = locate_build_file(builder)

    print_color(f"Build directory: {build_dir}", Fore.CYAN)

    # Run the build with retry logic
    run_build_with_retry(builder, build_dir, build_type, compiler)

    exe_name = "passport-cli"
    exe_path = build_dir / exe_name
    if not exe_path.exists():
        error_exit(f"Executable {exe_name} not found in {build_dir}.")

    blake2_hash, sha256_hash = compute_hashes(exe_path)
    if blake2_hash and sha256_hash:
        print_color("\n--- Hashes of built executable ---", Fore.CYAN, Style.BRIGHT)
        print(f"BLAKE2b: {blake2_hash}")
        print(f"SHA256:  {sha256_hash}")
    else:
        print_color("Unable to compute hashes.", Fore.YELLOW)

    # Optional signature verification
    verify_signature(build_dir)

    print_color("\nBuild completed successfully!", Fore.GREEN, Style.BRIGHT)

if __name__ == "__main__":
    main()