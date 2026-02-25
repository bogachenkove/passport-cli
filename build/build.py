#!/usr/bin/env python3
"""
Build script for Passport-CLI project.
Allows selection of build type, compiler, and build system.
Builds directly in the directory containing the build files.
Supports manual specification of library paths via environment variables.
"""

import os
import platform
import shutil
import subprocess
import sys
import hashlib
import socket
import re
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

# Global store for user‑defined environment variables (e.g., SODIUMROOT, ICUROOT)
USER_ENV = {}

def print_color(message, color=Fore.RESET, brightness=Style.RESET_ALL):
    if COLORAMA_AVAILABLE:
        print(f"{brightness}{color}{message}{Style.RESET_ALL}")
    else:
        print(message)

def error_exit(message):
    print_color(f"Error: {message}", Fore.RED, Style.BRIGHT)
    sys.exit(1)

def check_network():
    """Check if network is available by connecting to Google DNS."""
    try:
        socket.create_connection(("8.8.8.8", 53), timeout=5)
        return True
    except OSError:
        return False

def add_to_path(directory):
    """Add a directory to the PATH environment variable for the current process."""
    if directory and os.path.isdir(directory):
        os.environ['PATH'] = directory + os.pathsep + os.environ.get('PATH', '')
        return True
    return False

def sanitize_env_var_name(name):
    """Keep only alphanumeric and underscore characters."""
    # Remove leading/trailing whitespace and any non‑printable chars
    name = name.strip()
    # Keep only letters, digits, underscore
    return re.sub(r'[^a-zA-Z0-9_]', '', name)

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

def install_prompt(specific_cmd=None):
    """
    Universal prompt for entering installation commands.
    If specific_cmd is provided, it will check for its presence after a successful command.
    Returns True if a command succeeded (and specific_cmd became available if specified),
    False if user pressed Enter to abort this round.
    """
    if specific_cmd:
        print_color(f"Enter installation command for '{specific_cmd}' (or press Enter to abort, or type quit/exit):", Fore.CYAN)
    else:
        print_color("Enter installation commands (one per line).", Fore.CYAN)
        print_color("After a successful command, the build will be retried automatically.", Fore.CYAN)
        print_color("Press Enter on an empty line to abort this installation attempt.", Fore.CYAN)
        print_color("Type 'quit' or 'exit' to abort the script.", Fore.CYAN)

    while True:
        # Check network and warn if offline
        if not check_network():
            print_color("Warning: No network connectivity. Installation commands may fail.", Fore.YELLOW)

        cmd = input("> ").strip()
        if not cmd:
            return False
        if cmd.lower() in ('quit', 'exit'):
            error_exit("Aborted by user.")
        result = subprocess.run(cmd, shell=True)
        if result.returncode != 0:
            print_color(f"Command failed with exit code {result.returncode}. You may try another command.", Fore.RED)
            continue
        # Command succeeded
        if specific_cmd:
            # Verify that the required command is now available
            if shutil.which(specific_cmd) is not None:
                print_color(f"Command '{specific_cmd}' is now available.", Fore.GREEN)
                return True
            else:
                print_color(f"Command succeeded but '{specific_cmd}' still not found in PATH. You may need to restart your shell or try another command.", Fore.YELLOW)
                # Continue prompting
        else:
            # No specific command to verify, just assume success
            return True

def require_command(cmd_name):
    """
    Ensure a command is available. If not, offer interactive options:
    - Update package lists
    - Install via command
    - Manually specify path to executable
    - Quit
    Loops until command is found or user aborts.
    """
    while True:
        if shutil.which(cmd_name) is not None:
            return True

        print_color(f"Required command '{cmd_name}' not found in PATH.", Fore.YELLOW)

        action = get_choice(
            f"Choose action for '{cmd_name}'",
            "UIMP",  # U=Update, I=Install, M=Manual path, Q handled globally
            default=None
        )

        if action == 'U':  # Update package lists
            if not check_network():
                print_color("Warning: No network connectivity. Update may fail.", Fore.YELLOW)
            print_color("Enter update command (e.g., 'sudo apt update'):", Fore.CYAN)
            update_cmd = input("> ").strip()
            if update_cmd.lower() in ('quit', 'exit'):
                error_exit("Aborted by user.")
            if update_cmd:
                subprocess.run(update_cmd, shell=True)
            continue

        elif action == 'I':  # Install via command
            if not check_network():
                print_color("Warning: No network connectivity. Installation may fail.", Fore.YELLOW)
            if install_prompt(specific_cmd=cmd_name):
                continue
            else:
                continue

        elif action == 'M':  # Manually specify path
            print_color(f"Enter full path to the '{cmd_name}' executable:", Fore.CYAN)
            path_str = input("> ").strip()
            if not path_str:
                print_color("No path entered.", Fore.YELLOW)
                continue
            exe_path = Path(path_str).expanduser().resolve()
            if not exe_path.exists():
                print_color(f"File not found: {exe_path}", Fore.RED)
                continue
            if not os.access(exe_path, os.X_OK):
                print_color(f"File is not executable: {exe_path}", Fore.RED)
                continue
            parent_dir = str(exe_path.parent)
            if add_to_path(parent_dir):
                print_color(f"Added {parent_dir} to PATH. Verifying...", Fore.GREEN)
            else:
                print_color(f"Failed to add {parent_dir} to PATH.", Fore.RED)
            continue

def handle_build_failure():
    """
    Interactive handler for build failures (e.g., missing libraries).
    Offers options to set environment variables (SODIUMROOT, ICUROOT), run installation commands,
    update package lists, or quit.
    Returns True if the user performed an action that might resolve the issue,
    False if the user aborts.
    """
    print_color("Build failed. You can try to resolve missing dependencies.", Fore.YELLOW)
    while True:
        action = get_choice(
            "Choose action",
            "SEUQ",  # S=Set env, E=Enter install command, U=Update, Q=Quit
            default=None
        )
        if action == 'S':  # Set environment variable
            print_color("Enter environment variable name (e.g., SODIUMROOT, ICUROOT):", Fore.CYAN)
            raw_name = input("> ").strip()
            if not raw_name:
                print_color("No variable name entered.", Fore.YELLOW)
                continue
            var_name = sanitize_env_var_name(raw_name)
            if not var_name:
                print_color(f"Invalid variable name after sanitisation: '{raw_name}'", Fore.YELLOW)
                continue
            print_color(f"Enter value for {var_name} (path to library root):", Fore.CYAN)
            var_value = input("> ").strip()
            if not var_value:
                print_color("No value entered.", Fore.YELLOW)
                continue
            # Store for subsequent build attempts
            USER_ENV[var_name] = var_value
            print_color(f"Environment variable {var_name}={var_value} will be used in next build attempt.", Fore.GREEN)
            return True

        elif action == 'E':  # Enter installation command
            if install_prompt(specific_cmd=None):
                return True
            else:
                # User pressed Enter to abort this round – go back to menu
                continue

        elif action == 'U':  # Update package lists
            if not check_network():
                print_color("Warning: No network connectivity. Update may fail.", Fore.YELLOW)
            print_color("Enter update command (e.g., 'sudo apt update'):", Fore.CYAN)
            update_cmd = input("> ").strip()
            if update_cmd.lower() in ('quit', 'exit'):
                error_exit("Aborted by user.")
            if update_cmd:
                subprocess.run(update_cmd, shell=True)
            continue

        elif action == 'Q':  # Quit
            return False

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

    # Remove CMakeFiles to ensure clean state
    cmake_files = build_dir / "CMakeFiles"
    if cmake_files.exists():
        print_color(f"Removing existing {cmake_files} for clean build.", Fore.YELLOW)
        shutil.rmtree(cmake_files)
    cache_file = build_dir / "CMakeCache.txt"
    if cache_file.exists():
        cache_file.unlink()

    print_color("\n--- Running CMake ---", Fore.CYAN, Style.BRIGHT)
    os.chdir(build_dir)

    # Build environment with user-defined variables
    env = os.environ.copy()
    env.update(USER_ENV)

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
    result = subprocess.run(cmake_cmd, env=env)
    if result.returncode != 0:
        print_color("CMake generation failed.", Fore.RED)
        return False

    print_color("\n--- Compiling (cmake --build) ---", Fore.CYAN, Style.BRIGHT)
    build_cmd = ['cmake', '--build', '.', '--parallel']
    result = subprocess.run(build_cmd, env=env)
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

    # Remove MakeFiles to ensure clean state
    make_files = build_dir / "MakeFiles"
    if make_files.exists():
        print_color(f"Removing existing {make_files} for clean build.", Fore.YELLOW)
        shutil.rmtree(make_files)

    print_color("\n--- Running Make ---", Fore.CYAN, Style.BRIGHT)
    os.chdir(build_dir)

    # Build environment with user-defined variables
    env = os.environ.copy()
    env.update(USER_ENV)
    if compiler == 'G':
        env['CXX'] = 'g++'
    else:
        env['CXX'] = 'clang++ -stdlib=libstdc++'
    env['BUILD'] = 'debug' if build_type == 'D' else 'release'

    # Debug: show environment variables that might affect build
    if 'SODIUMROOT' in env:
        print_color(f"  SODIUMROOT={env['SODIUMROOT']}", Fore.CYAN)
    if 'ICUROOT' in env:
        print_color(f"  ICUROOT={env['ICUROOT']}", Fore.CYAN)

    make_cmd = ['make', '--jobs']
    print_color(f"Executing: {' '.join(make_cmd)} with BUILD={env['BUILD']}, CXX={env.get('CXX','')}", Fore.YELLOW)
    result = subprocess.run(make_cmd, env=env)
    if result.returncode != 0:
        print_color("Make build failed.", Fore.RED)
        return False

    print_color("Make build completed successfully.", Fore.GREEN)
    return True

def run_build_with_retry(builder, build_dir, build_type, compiler):
    """Attempt build indefinitely, prompting for dependency installation on failure."""
    attempt = 0
    while True:
        attempt += 1
        print_color(f"\n--- Build attempt {attempt} ---", Fore.CYAN, Style.BRIGHT)
        success = False
        if builder == 'C':
            success = build_with_cmake(build_dir, build_type, compiler)
        else:
            success = build_with_make(build_dir, build_type, compiler)

        if success:
            return True

        # Interactive failure handling
        if not handle_build_failure():
            error_exit("Build aborted by user.")
        print_color("Retrying build...", Fore.CYAN)

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