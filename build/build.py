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

def check_root():
    """Ensure script is not run as root."""
    if os.geteuid() == 0:
        error_exit("This script should not be run as root. Please run as a normal user.")

def check_network():
    """Check internet connectivity by pinging a reliable host."""
    try:
        subprocess.run(['ping', '-c', '1', '8.8.8.8'],
                       stdout=subprocess.DEVNULL,
                       stderr=subprocess.DEVNULL,
                       check=True)
        return True
    except subprocess.CalledProcessError:
        return False

def get_distro():
    """Detect Linux distribution ID and version."""
    if platform.system() != 'Linux':
        return None, None, None
    try:
        with open('/etc/os-release', 'r') as f:
            lines = f.readlines()
        os_release = {}
        for line in lines:
            if '=' in line:
                key, value = line.strip().split('=', 1)
                os_release[key] = value.strip('"')
        distro_id = os_release.get('ID', '').lower()
        distro_version = os_release.get('VERSION_ID', '')
        distro_name = os_release.get('NAME', '')
        return distro_id, distro_version, distro_name
    except Exception:
        return None, None, None

# Allowed distributions and their families – temporarily only Debian and Arch families are active
ALLOWED_DISTROS = {
    'debian': ['debian', 'ubuntu', 'kali', 'tails'],  # raspberrypi removed
    'arch': ['archlinux', 'manjaro'],
    # 'redhat': ['redhat', 'centos', 'fedora', 'rhel'],
    # 'suse': ['opensuse'],
    # 'void': ['void'],
    # 'slackware': ['slackware'],
    # 'gentoo': ['gentoo']
}
FLAT_ALLOWED = [d for family in ALLOWED_DISTROS.values() for d in family]

def check_allowed_distro(distro_id):
    """Warn if distribution is not in the allowed list."""
    if distro_id and distro_id not in FLAT_ALLOWED:
        print_color(f"Warning: Distribution '{distro_id}' is not in the allowed list. "
                    f"Package installation may not work correctly.", Fore.YELLOW)

def get_package_manager(distro_id):
    """Return package manager command and update/install templates based on distro."""
    if distro_id in ALLOWED_DISTROS['debian']:
        return {
            'update': ['sudo', 'apt', 'update'],
            'install': ['sudo', 'apt', 'install', '-y']
        }
    elif distro_id in ALLOWED_DISTROS['arch']:
        return {
            'update': ['sudo', 'pacman', '-Sy'],
            'install': ['sudo', 'pacman', '-S', '--noconfirm']
        }
    # elif distro_id in ALLOWED_DISTROS['redhat']:
    #     if shutil.which('dnf'):
    #         return {
    #             'update': ['sudo', 'dnf', 'check-update'],
    #             'install': ['sudo', 'dnf', 'install', '-y']
    #         }
    #     else:
    #         return {
    #             'update': ['sudo', 'yum', 'check-update'],
    #             'install': ['sudo', 'yum', 'install', '-y']
    #         }
    # elif distro_id in ALLOWED_DISTROS['suse']:
    #     return {
    #         'update': ['sudo', 'zypper', 'refresh'],
    #         'install': ['sudo', 'zypper', 'install', '-y']
    #     }
    # elif distro_id in ALLOWED_DISTROS['void']:
    #     return {
    #         'update': ['sudo', 'xbps-install', '-Su'],
    #         'install': ['sudo', 'xbps-install', '-y']
    #     }
    # elif distro_id in ALLOWED_DISTROS['slackware']:
    #     return {
    #         'update': [],
    #         'install': ['sudo', 'slackpkg', 'install']
    #     }
    # elif distro_id in ALLOWED_DISTROS['gentoo']:
    #     return {
    #         'update': ['sudo', 'emerge', '--sync'],
    #         'install': ['sudo', 'emerge', '-v']
    #     }
    else:
        return None

def install_missing_packages(missing):
    """Offer to install missing packages and do so if user agrees."""
    if not missing:
        return True
    print_color(f"Missing required packages: {', '.join(missing)}", Fore.YELLOW)
    choice = get_choice("Install missing packages?", "YN", default="Y")
    if choice == 'N':
        return False

    # Check network connectivity
    if not check_network():
        print_color("No internet connection. Cannot install packages.", Fore.RED)
        return False

    # Determine distribution
    distro_id, _, _ = get_distro()
    if not distro_id:
        print_color("Could not determine Linux distribution. Please install packages manually.", Fore.RED)
        return False

    pm = get_package_manager(distro_id)
    if not pm:
        print_color(f"Unsupported distribution '{distro_id}'. Please install packages manually.", Fore.RED)
        return False

    # Update package cache if supported
    if pm['update']:
        print_color("Updating package cache...", Fore.CYAN)
        try:
            subprocess.run(pm['update'], check=True)
        except subprocess.CalledProcessError:
            print_color("Failed to update package cache. Proceeding with installation anyway.", Fore.YELLOW)

    # Install missing packages
    print_color(f"Installing: {' '.join(missing)}", Fore.CYAN)
    cmd = pm['install'] + missing
    try:
        subprocess.run(cmd, check=True)
    except subprocess.CalledProcessError:
        print_color("Package installation failed.", Fore.RED)
        return False

    # Verify installation
    still_missing = [pkg for pkg in missing if shutil.which(pkg) is None]
    if still_missing:
        print_color(f"Still missing after installation: {', '.join(still_missing)}", Fore.RED)
        return False

    print_color("All required packages installed.", Fore.GREEN)
    return True

def check_dependencies():
    """Verify required tools are installed; return list of missing."""
    required = ['cmake', 'make', 'pkg-config', 'gpg']
    missing = [cmd for cmd in required if shutil.which(cmd) is None]
    return missing

def get_choice(prompt, options, default=None):
    """
    Universal choice selector.
    prompt: text prompt (e.g., "Build type (Debug/Release)")
    options: string of allowed characters (e.g., "DR")
    default: default value if user presses Enter
    Returns selected character in uppercase.
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
        if not user_input and default:
            return default
        if len(user_input) == 1 and user_input in options_upper:
            return user_input
        print_color(f"Invalid input. Expected one of: {options_upper}", Fore.YELLOW)

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
        print_color(f"Removing existing {leftleft_dir} from source directory.", Fore.YELLOW)
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
    """Build using CMake."""
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
        error_exit("CMake generation failed.")

    print_color("\n--- Compiling (cmake --build) ---", Fore.CYAN, Style.BRIGHT)
    build_cmd = ['cmake', '--build', '.', '--parallel']
    result = subprocess.run(build_cmd)
    if result.returncode != 0:
        error_exit("Compilation failed.")
    print_color("CMake build completed successfully.", Fore.GREEN)

def build_with_make(build_dir, build_type, compiler):
    """Build using Make."""
    print_color("\n--- Running Make ---", Fore.CYAN, Style.BRIGHT)
    os.chdir(build_dir)

    env = os.environ.copy()
    if compiler == 'G':
        env['CXX'] = 'g++'
    else:
        env['CXX'] = 'clang++'
    env['BUILD'] = 'debug' if build_type == 'D' else 'release'

    make_cmd = ['make', '--jobs']
    print_color(f"Executing: {' '.join(make_cmd)} with BUILD={env['BUILD']}, CXX={env.get('CXX','')}", Fore.YELLOW)
    result = subprocess.run(make_cmd, env=env)
    if result.returncode != 0:
        error_exit("Compilation failed.")
    print_color("Make build completed successfully.", Fore.GREEN)

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
    check_root()

    # System and distribution info
    system = platform.system()
    release = platform.release()
    distro_id, distro_version, distro_name = get_distro()
    if system == 'Linux' and distro_id:
        distro_str = f"{distro_name} {distro_version}"
        print_color(f"System: {distro_str} ({system} {release})", Fore.CYAN)
        check_allowed_distro(distro_id)
    else:
        print_color(f"System: {system} {release}", Fore.CYAN)

    print_color("Passport-CLI build script", Fore.GREEN, Style.BRIGHT)

    # Check dependencies and offer installation if missing
    missing = check_dependencies()
    if missing:
        if not install_missing_packages(missing):
            error_exit(f"Missing required packages: {', '.join(missing)}. Install them and retry.")

    build_type, compiler, builder = get_build_config()

    # Locate the build system file and clean leftovers
    build_dir = locate_build_file(builder)

    print_color(f"Build directory: {build_dir}", Fore.CYAN)

    # Run the build
    if builder == 'C':  # CMake
        build_with_cmake(build_dir, build_type, compiler)
    else:  # Make
        build_with_make(build_dir, build_type, compiler)

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