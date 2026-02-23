#!/usr/bin/env python3
# -*- coding: utf-8 -*-

"""
Universal build script for passport-cli.
Guides the user through compiler selection and build process using CMake or Make.
"""

import os
import sys
import subprocess
import shutil
from pathlib import Path

# Attempt to import colorama for colored terminal output.
# If missing, provide a fallback that disables colors.
try:
    from colorama import init, Fore, Style
    init(autoreset=True)
    HAVE_COLORAMA = True
except ImportError:
    print("""ERROR: Colorama library is not installed.
Please install it using: pip install colorama
Or run the script without colorama support (colors will be disabled).""")
    HAVE_COLORAMA = False
    # Dummy color classes when colorama is absent.
    class Fore:
        RED = GREEN = YELLOW = BLUE = RESET = ''
    Style = Fore

def print_color(level, msg, end='\n'):
    """Print a message with a colored level prefix.
    level: one of 'INFO', 'WARN', 'ERROR', 'PROMPT'.
    msg: the message text.
    end: line ending (default newline)."""
    if level == 'INFO':
        print(f"{Fore.GREEN}[INFO]{Style.RESET_ALL} {msg}", end=end)
    elif level == 'WARN':
        print(f"{Fore.YELLOW}[WARN]{Style.RESET_ALL} {msg}", end=end)
    elif level == 'ERROR':
        print(f"{Fore.RED}[ERROR]{Style.RESET_ALL} {msg}", end=end)
    elif level == 'PROMPT':
        print(f"{Fore.BLUE}[PROMPT]{Style.RESET_ALL} {msg}", end=end)
    else:
        print(msg, end=end)

def run_cmd(cmd, check=True, capture=False, shell=False):
    """Execute a shell command.
    If capture=True, return (stdout, stderr, returncode).
    Otherwise, run and optionally check for errors."""
    if capture:
        result = subprocess.run(cmd, shell=shell, capture_output=True, text=True)
        return result.stdout.strip(), result.stderr.strip(), result.returncode
    else:
        subprocess.run(cmd, shell=shell, check=check)

def check_command(cmd):
    """Return True if the command is available in the system PATH."""
    return shutil.which(cmd) is not None

def get_choice(prompt_text, allowed, default):
    """Prompt the user for a single‑letter choice.
    - prompt_text: the prompt message (printed without newline).
    - allowed: set of allowed characters (lowercase).
    - default: default letter if user presses Enter.
    Returns the selected letter (lowercase). Exits on invalid input."""
    print_color('PROMPT', prompt_text, end='')
    ans = input().strip().lower()
    if ans == '':
        return default
    if ans in allowed:
        return ans
    print_color('ERROR', f"Invalid choice. Expected one of: {', '.join(sorted(allowed)).upper()}")
    sys.exit(1)

def main():
    """Main build routine: verify tools, select options, and run build."""
    # ==== 1. Verify essential build tools ====
    print_color('INFO', "Checking required tools...")
    tools_to_check = ['cmake', 'make', 'pkg-config']
    missing_tools = [tool for tool in tools_to_check if not check_command(tool)]

    if missing_tools:
        print_color('ERROR', f"Missing required tools: {', '.join(missing_tools)}")
        print_color('ERROR', "Please install them and try again.")
        sys.exit(1)
    else:
        print_color('INFO', "All required tools are present.")

    # ==== 2. Choose build type (Release/Debug) ====
    choice = get_choice(
        "Build type: [R]elease or [D]ebug? (default Release): ",
        allowed={'r', 'd'},
        default='r'
    )
    build_type = 'Release' if choice == 'r' else 'Debug'
    print_color('INFO', f"Build type: {build_type}")

    # ==== 3. Detect and select C++ compiler ====
    compiler_map = {}
    if check_command('g++'):
        compiler_map['g'] = 'g++'
    if check_command('clang'):
        compiler_map['c'] = 'clang'
    if check_command('clang++'):
        compiler_map['a'] = 'clang++'

    if not compiler_map:
        print_color('ERROR', "No C++ compiler found. Please install g++ or clang++.")
        sys.exit(1)

    allowed_compilers = set(compiler_map.keys())
    # Default to g++ if available, otherwise the first available.
    default_compiler = 'g' if 'g' in allowed_compilers else next(iter(allowed_compilers))

    choice = get_choice(
        "Compiler type: [G]++, [C]lang or Cl[A]ng++ (default G++): ",
        allowed=allowed_compilers,
        default=default_compiler
    )
    selected_compiler = compiler_map[choice]
    print_color('INFO', f"Selected compiler: {selected_compiler}")

    # ==== 4. Choose build system (CMake or Make) ====
    script_dir = Path(__file__).parent.resolve()
    builder_map = {}
    if (script_dir / 'CMakeLists.txt').exists():
        builder_map['c'] = 'cmake'
    if (script_dir / 'Makefile').exists():
        builder_map['m'] = 'make'

    if not builder_map:
        print_color('ERROR', "Neither CMakeLists.txt nor Makefile found in build directory.")
        sys.exit(1)

    allowed_builders = set(builder_map.keys())
    default_builder = 'c' if 'c' in allowed_builders else 'm'

    choice = get_choice(
        "Select builder: [C]Make or [M]ake (default CMake): ",
        allowed=allowed_builders,
        default=default_builder
    )
    builder = builder_map[choice]
    print_color('INFO', f"Selected builder: {builder.capitalize()}")

    # ==== 5. Prepare environment and run the build ====
    os.chdir(script_dir)
    env = os.environ.copy()
    env['CXX'] = selected_compiler

    # Set additional compiler flags based on the chosen compiler.
    if selected_compiler == 'g++':
        # g++ works out‑of‑the‑box with default settings.
        pass
    elif selected_compiler == 'clang':
        # When using clang as a C compiler for C++, we must explicitly link libstdc++.
        env['CXXFLAGS'] = env.get('CXXFLAGS', '') + ' -stdlib=libstdc++'
        env['LDFLAGS'] = env.get('LDFLAGS', '') + ' -lstdc++'
    elif selected_compiler == 'clang++':
        # clang++ automatically links the C++ standard library; we just set the runtime.
        env['CXXFLAGS'] = env.get('CXXFLAGS', '') + ' -stdlib=libstdc++'

    if builder == 'cmake':
        print_color('INFO', "Configuring with CMake...")
        cmake_cmd = ['cmake', f'-DCMAKE_BUILD_TYPE={build_type}', '.']
        if selected_compiler:
            cmake_cmd.insert(1, f'-DCMAKE_CXX_COMPILER={selected_compiler}')
        try:
            subprocess.run(cmake_cmd, env=env, check=True)
        except subprocess.CalledProcessError:
            print_color('ERROR', "CMake configuration failed.")
            sys.exit(1)
        print_color('INFO', "Building...")
        try:
            subprocess.run(['cmake', '--build', '.', '--config', build_type, '--parallel'], env=env, check=True)
        except subprocess.CalledProcessError:
            print_color('ERROR', "Build failed.")
            sys.exit(1)
    else:  # make
        print_color('INFO', "Building with Make...")
        build_lower = build_type.lower()
        make_cmd = ['make', f'BUILD={build_lower}', '-j', str(os.cpu_count() or 2)]
        try:
            subprocess.run(make_cmd, env=env, check=True)
        except subprocess.CalledProcessError:
            print_color('ERROR', "Make build failed.")
            sys.exit(1)

    # ==== 6. Verify that the executable was created ====
    bin_name = 'passport-cli'
    bin_path = script_dir / bin_name
    if bin_path.exists():
        print_color('INFO', f"Build successful! Executable: {bin_path}")
    else:
        print_color('ERROR', f"Executable {bin_name} not found after build.")
        sys.exit(1)

    print_color('INFO', "Build script finished successfully.")

if __name__ == '__main__':
    main()