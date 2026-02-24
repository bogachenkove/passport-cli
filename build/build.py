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

def check_dependencies():
    """Verify required tools are installed."""
    required = ['cmake', 'make', 'pkg-config']
    missing = [cmd for cmd in required if shutil.which(cmd) is None]
    if missing:
        error_exit(f"Missing required packages: {', '.join(missing)}. Install them and retry.")

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

def main():
    # System information
    system = platform.system()
    release = platform.release()
    print_color(f"System: {system} {release}", Fore.CYAN)

    print_color("Passport-CLI build script", Fore.GREEN, Style.BRIGHT)

    check_dependencies()

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

    print_color("\nBuild completed successfully!", Fore.GREEN, Style.BRIGHT)

if __name__ == "__main__":
    main()