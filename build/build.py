#!/usr/bin/env python3

import os
import platform
import shutil
import subprocess
import sys
import hashlib
import socket
import re
from pathlib import Path
from dataclasses import dataclass
from typing import Optional, Dict, List, Tuple, Union

# Optional color output
try:
    from colorama import init, Fore, Style
    init(autoreset=True)
    COLORAMA_AVAILABLE = True
except ImportError:
    COLORAMA_AVAILABLE = False
    Fore = type('Fore', (), {'RED': '', 'GREEN': '', 'YELLOW': '', 'CYAN': '', 'RESET': ''})()
    Style = type('Style', (), {'BRIGHT': '', 'RESET_ALL': ''})()


def print_color(message: str, color=Fore.RESET, brightness=Style.RESET_ALL) -> None:
    if COLORAMA_AVAILABLE:
        print(f"{brightness}{color}{message}{Style.RESET_ALL}")
    else:
        print(message)


def error_exit(message: str) -> None:
    print_color(f"Error: {message}", Fore.RED, Style.BRIGHT)
    sys.exit(1)


def check_network(timeout: int = 5) -> bool:
    """Check if network is available by connecting to Cloudflare DNS."""
    try:
        socket.create_connection(("1.1.1.1", 53), timeout=timeout)
        return True
    except OSError:
        return False


def add_to_path(directory: Union[str, Path]) -> bool:
    """Add a directory to the PATH environment variable for the current process."""
    dir_path = Path(directory).expanduser().resolve()
    if dir_path.is_dir():
        os.environ['PATH'] = str(dir_path) + os.pathsep + os.environ.get('PATH', '')
        return True
    return False


def sanitize_env_var_name(name: str) -> str:
    """Keep only alphanumeric and underscore characters."""
    name = name.strip()
    return re.sub(r'[^a-zA-Z0-9_]', '', name)


def get_choice(prompt: str, options: str, default: Optional[str] = None,
               option_help: Optional[str] = None) -> str:
    """
    Universal choice selector.
    prompt: text prompt (e.g., "Build type")
    options: string of allowed characters (e.g., "DR")
    default: default value if user presses Enter
    option_help: optional string describing each option (e.g., "D=Debug, R=Release")
    Returns selected character in uppercase.
    Pressing Q at any time aborts the script.
    """
    options_upper = options.upper()
    if default:
        default = default.upper()
        if default not in options_upper:
            raise ValueError(f"Default '{default}' not in allowed options '{options_upper}'")
        default_text = f" (default: {default})"
    else:
        default_text = ""

    if option_help:
        full_prompt = f"{prompt} [{options_upper}] {option_help}{default_text}: "
    else:
        full_prompt = f"{prompt} [{options_upper}]{default_text}: "

    while True:
        user_input = input(full_prompt).strip().upper()
        if user_input == 'Q':
            print_color("Aborted by user.", Fore.RED)
            sys.exit(1)
        if not user_input and default:
            return default
        if len(user_input) == 1 and user_input in options_upper:
            return user_input
        print_color(f"Invalid input. Expected one of: {options_upper} (or Q to quit)", Fore.YELLOW)


def install_prompt(specific_cmd: Optional[str] = None) -> bool:
    """
    Universal prompt for entering installation commands.
    If specific_cmd is provided, it will check for its presence after a successful command.
    Returns True if a command succeeded, False if user aborted.
    """
    if specific_cmd:
        print_color(f"Enter installation command for '{specific_cmd}' (or press Enter to abort, or type quit/exit):", Fore.CYAN)
    else:
        print_color("Enter installation commands (one per line).", Fore.CYAN)
        print_color("After a successful command, the build will be retried automatically.", Fore.CYAN)
        print_color("Press Enter on an empty line to abort this installation attempt.", Fore.CYAN)
        print_color("Type 'quit' or 'exit' to abort the script.", Fore.CYAN)

    while True:
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

        if specific_cmd and shutil.which(specific_cmd) is None:
            print_color(f"Command succeeded but '{specific_cmd}' still not found in PATH. You may need to restart your shell or try another command.", Fore.YELLOW)
            continue
        return True


def require_command(cmd_name: str) -> bool:
    """
    Ensure a command is available. If not, offer interactive options.
    Returns True when command becomes available.
    """
    while True:
        if shutil.which(cmd_name) is not None:
            return True

        print_color(f"Required command '{cmd_name}' not found in PATH.", Fore.YELLOW)
        action = get_choice(
            f"Choose action for '{cmd_name}'",
            "IM",
            default=None,
            option_help="(I=Install via command, M=Manually specify path)"
        )

        if action == 'I':  # Install via command
            install_prompt(specific_cmd=cmd_name)
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
            if add_to_path(exe_path.parent):
                print_color(f"Added {exe_path.parent} to PATH. Verifying...", Fore.GREEN)
            else:
                print_color(f"Failed to add {exe_path.parent} to PATH.", Fore.RED)
            continue


@dataclass
class BuildConfig:
    build_type: str  # 'D' or 'R'
    compiler: str    # 'G' or 'C'
    builder: str     # 'C' or 'M'

    @property
    def build_type_name(self) -> str:
        return 'Debug' if self.build_type == 'D' else 'Release'

    @property
    def compiler_name(self) -> str:
        return 'G++' if self.compiler == 'G' else 'Clang++'

    @property
    def builder_name(self) -> str:
        return 'CMake' if self.builder == 'C' else 'Make'

    @property
    def make_build_mode(self) -> str:
        return 'debug' if self.build_type == 'D' else 'release'


class EnvironmentManager:
    """Manages environment variables for the build."""
    def __init__(self):
        self.user_env: Dict[str, str] = {}

    def set_var(self, name: str, value: str) -> None:
        self.user_env[name] = value

    def get_env_copy(self) -> Dict[str, str]:
        env = os.environ.copy()
        env.update(self.user_env)
        return env


class BuildDirectory:
    """Handles locating and cleaning build directory."""
    def __init__(self, builder: str):
        self.builder = builder
        self.build_file = "CMakeLists.txt" if builder == 'C' else "Makefile"
        self.leftover_dir = "CMakeFiles" if builder == 'C' else "MakeFiles"
        self.path: Optional[Path] = None

    def locate(self) -> Path:
        """Find directory containing build file, prompt if not found."""
        script_dir = Path.cwd()
        candidate = script_dir / self.build_file
        if candidate.exists():
            print_color(f"Found {self.build_file} in current directory.", Fore.GREEN)
            self.path = script_dir
        else:
            print_color(f"{self.build_file} not found in current directory.", Fore.YELLOW)
            while True:
                user_path = input(f"Enter path to directory containing {self.build_file}: ").strip()
                if not user_path:
                    continue
                path = Path(user_path).expanduser().resolve()
                if (path / self.build_file).exists():
                    self.path = path
                    print_color(f"Using {self.build_file} from {self.path}", Fore.GREEN)
                    break
                else:
                    print_color(f"{self.build_file} not found in {path}. Try again.", Fore.YELLOW)
        return self.path

    def clean(self) -> None:
        """Remove leftover build directories and cache files."""
        if not self.path:
            return
        leftover = self.path / self.leftover_dir
        if leftover.exists() and leftover.is_dir():
            print_color(f"Removing existing {self.leftover_dir} from source directory.", Fore.YELLOW)
            shutil.rmtree(leftover)

        if self.builder == 'C':
            cache = self.path / "CMakeCache.txt"
            if cache.exists():
                cache.unlink()
            cmake_install = self.path / "cmake_install.cmake"
            if cmake_install.exists():
                cmake_install.unlink()


class BuildExecutor:
    """Executes the build using CMake or Make with retry logic."""
    def __init__(self, config: BuildConfig, env_mgr: EnvironmentManager, build_dir: Path):
        self.config = config
        self.env_mgr = env_mgr
        self.build_dir = build_dir

    def run_with_retry(self) -> bool:
        attempt = 0
        while True:
            attempt += 1
            print_color(f"\n--- Build attempt {attempt} ---", Fore.CYAN, Style.BRIGHT)
            success = self._run_build()
            if success:
                return True
            if not self._handle_failure():
                error_exit("Build aborted by user.")
            print_color("Retrying build...", Fore.CYAN)

    def _run_build(self) -> bool:
        if self.config.builder == 'C':
            return self._build_with_cmake()
        else:
            return self._build_with_make()

    def _build_with_cmake(self) -> bool:
        require_command('cmake')
        require_command('g++' if self.config.compiler == 'G' else 'clang++')

        cmake_files = self.build_dir / "CMakeFiles"
        if cmake_files.exists():
            shutil.rmtree(cmake_files)
        cache_file = self.build_dir / "CMakeCache.txt"
        if cache_file.exists():
            cache_file.unlink()

        print_color("\n--- Running CMake ---", Fore.CYAN, Style.BRIGHT)
        os.chdir(self.build_dir)

        env = self.env_mgr.get_env_copy()
        cmake_cmd = ['cmake', '.']
        cmake_cmd.append(f'-DCMAKE_BUILD_TYPE={self.config.build_type_name}')
        if self.config.compiler == 'G':
            cmake_cmd.append('-DCMAKE_CXX_COMPILER=g++')
        else:
            cmake_cmd.extend(['-DCMAKE_CXX_COMPILER=clang++', '-DCMAKE_CXX_FLAGS=-stdlib=libstdc++'])

        print_color(f"Executing: {' '.join(cmake_cmd)}", Fore.YELLOW)
        if subprocess.run(cmake_cmd, env=env).returncode != 0:
            print_color("CMake generation failed.", Fore.RED)
            return False

        print_color("\n--- Compiling (cmake --build) ---", Fore.CYAN, Style.BRIGHT)
        build_cmd = ['cmake', '--build', '.', '--parallel']
        if subprocess.run(build_cmd, env=env).returncode != 0:
            print_color("Compilation failed.", Fore.RED)
            return False

        print_color("CMake build completed successfully.", Fore.GREEN)
        return True

    def _build_with_make(self) -> bool:
        require_command('make')
        require_command('g++' if self.config.compiler == 'G' else 'clang++')

        make_files = self.build_dir / "MakeFiles"
        if make_files.exists():
            print_color(f"Removing existing {make_files} for clean build.", Fore.YELLOW)
            shutil.rmtree(make_files)

        print_color("\n--- Running Make ---", Fore.CYAN, Style.BRIGHT)
        os.chdir(self.build_dir)

        env = self.env_mgr.get_env_copy()
        if self.config.compiler == 'G':
            env['CXX'] = 'g++'
        else:
            env['CXX'] = 'clang++ -stdlib=libstdc++'
        env['BUILD'] = self.config.make_build_mode

        for var in ('SODIUMROOT', 'ICUROOT'):
            if var in env:
                print_color(f"  {var}={env[var]}", Fore.CYAN)

        make_cmd = ['make', '--jobs']
        print_color(f"Executing: {' '.join(make_cmd)} with BUILD={env['BUILD']}, CXX={env.get('CXX','')}", Fore.YELLOW)
        if subprocess.run(make_cmd, env=env).returncode != 0:
            print_color("Make build failed.", Fore.RED)
            return False

        print_color("Make build completed successfully.", Fore.GREEN)
        return True

    def _handle_failure(self) -> bool:
        """Interactive handler for build failures."""
        print_color("Build failed. You can try to resolve missing dependencies.", Fore.YELLOW)
        while True:
            action = get_choice(
                "Choose action",
                "SEQ",
                default=None,
                option_help="(S=Set environment variable, E=Enter install command, Q=Quit)"
            )
            if action == 'S':  # Set environment variable
                raw_name = input("Enter environment variable name (e.g., SODIUMROOT, ICUROOT): ").strip()
                if not raw_name:
                    print_color("No variable name entered.", Fore.YELLOW)
                    continue
                var_name = sanitize_env_var_name(raw_name)
                if not var_name:
                    print_color(f"Invalid variable name after sanitisation: '{raw_name}'", Fore.YELLOW)
                    continue
                var_value = input(f"Enter value for {var_name} (path to library root): ").strip()
                if not var_value:
                    print_color("No value entered.", Fore.YELLOW)
                    continue
                self.env_mgr.set_var(var_name, var_value)
                print_color(f"Environment variable {var_name}={var_value} will be used in next build attempt.", Fore.GREEN)
                return True

            elif action == 'E':  # Enter installation command
                if install_prompt():
                    return True
                continue

            elif action == 'Q':  # Quit
                return False


class SignatureVerifier:
    """Verifies built executable against signed checksums."""
    def __init__(self, build_dir: Path):
        self.build_dir = build_dir
        self.sig_dir = build_dir / "signature"
        self.required_files = {
            'public_key': ('publickey.asc', None),
            'sha256_sums': ('SHA256/SHA256SUMS', None),
            'sha256_sums_asc': ('SHA256/SHA256SUMS.asc', None),
            'blake2b_sums': ('BLAKE2b/BLAKE2BSUMS', None),
            'blake2b_sums_asc': ('BLAKE2b/BLAKE2BSUMS.asc', None)
        }

    def verify(self) -> bool:
        print_color("\n--- Signature Verification ---", Fore.CYAN, Style.BRIGHT)
        if get_choice("Verify executable signature?", "YN", default="Y", option_help="(Y=Yes, N=No)") == 'N':
            return False

        require_command('gpg')
        self._locate_signature_files()

        # Import public key if needed
        pubkey_path = self.required_files['public_key'][1]
        key_id = self._extract_key_id(pubkey_path)
        if key_id and not self._key_in_keyring(key_id):
            print_color("Importing public key...", Fore.CYAN)
            result = subprocess.run(['gpg', '--import', str(pubkey_path)], capture_output=True, text=True)
            if result.returncode != 0:
                print_color(f"Failed to import public key: {result.stderr}", Fore.RED)
                return False
            print_color("Public key imported.", Fore.GREEN)
        else:
            print_color("Public key already present in keyring.", Fore.GREEN)

        # Verify signatures
        if not self._verify_signature('sha256_sums'):
            return False
        if not self._verify_signature('blake2b_sums'):
            return False

        # Compute hashes of executable
        exe_path = self.build_dir / "passport-cli"
        if not exe_path.exists():
            print_color(f"Executable not found at {exe_path}", Fore.RED)
            return False

        blake2_hash, sha256_hash = self._compute_hashes(exe_path)
        if not blake2_hash or not sha256_hash:
            print_color("Failed to compute hashes.", Fore.RED)
            return False

        # Check against checksum files
        if not self._check_hash_in_file('sha256_sums', sha256_hash, exe_path.name):
            return False
        if not self._check_hash_in_file('blake2b_sums', blake2_hash, exe_path.name):
            return False

        print_color("\nAll signature and hash verifications passed.", Fore.GREEN, Style.BRIGHT)
        return True

    def _locate_signature_files(self) -> None:
        """Find signature files, prompting for missing ones."""
        for key, (rel_path, _) in self.required_files.items():
            full_path = self.sig_dir / rel_path
            if full_path.exists():
                self.required_files[key] = (rel_path, full_path)
            else:
                print_color(f"Signature file {rel_path} not found in default location.", Fore.YELLOW)
                while True:
                    user_input = input(f"Enter path to {rel_path}: ").strip()
                    if not user_input:
                        continue
                    user_path = Path(user_input).expanduser().resolve()
                    if user_path.exists():
                        self.required_files[key] = (rel_path, user_path)
                        break
                    else:
                        print_color(f"File not found: {user_path}", Fore.YELLOW)

    def _extract_key_id(self, pubkey_path: Path) -> Optional[str]:
        try:
            result = subprocess.run(['gpg', '--with-colons', '--import-options', 'show-only', '--import', str(pubkey_path)],
                                    capture_output=True, text=True, check=False)
            for line in result.stdout.splitlines():
                if line.startswith('fpr:'):
                    parts = line.split(':')
                    if len(parts) > 9:
                        return parts[9]
        except Exception as e:
            print_color(f"Error reading public key: {e}", Fore.YELLOW)
        return None

    def _key_in_keyring(self, key_id: str) -> bool:
        result = subprocess.run(['gpg', '--list-keys', '--with-colons', key_id],
                                capture_output=True, text=True, check=False)
        return result.returncode == 0

    def _verify_signature(self, key: str) -> bool:
        sums_path = self.required_files[key][1]
        asc_path = self.required_files[f'{key}_asc'][1]
        print_color(f"Verifying {key} signature...", Fore.CYAN)
        result = subprocess.run(['gpg', '--verify', str(asc_path), str(sums_path)],
                                capture_output=True, text=True)
        if result.returncode != 0:
            print_color(f"{key} signature verification failed: {result.stderr}", Fore.RED)
            return False
        print_color(f"{key} signature is valid.", Fore.GREEN)
        return True

    @staticmethod
    def _compute_hashes(file_path: Path) -> Tuple[Optional[str], Optional[str]]:
        try:
            with open(file_path, 'rb') as f:
                data = f.read()
            blake2 = hashlib.blake2b(data).hexdigest()
            sha256 = hashlib.sha256(data).hexdigest()
            return blake2, sha256
        except Exception:
            return None, None

    def _check_hash_in_file(self, key: str, file_hash: str, exe_name: str) -> bool:
        sums_path = self.required_files[key][1]
        with open(sums_path, 'r') as f:
            content = f.read()
        for line in content.splitlines():
            parts = line.strip().split()
            if len(parts) >= 2 and exe_name in line and file_hash == parts[0]:
                return True
        print_color(f"{key} hash does NOT match any entry in checksums file.", Fore.RED)
        return False


def get_build_config() -> BuildConfig:
    """Interactive build configuration selection."""
    while True:
        print_color("\n--- Build Configuration ---", Fore.CYAN, Style.BRIGHT)
        build_type = get_choice("Build type", "DR", default="R", option_help="(D=Debug, R=Release)")
        compiler = get_choice("Compiler", "GC", default="G", option_help="(G=G++, C=Clang++)")
        builder = get_choice("Build system", "CM", default="C", option_help="(C=CMake, M=Make)")

        config = BuildConfig(build_type, compiler, builder)

        print_color("\n--- Selected Configuration ---", Fore.CYAN)
        print(f"  Build type:   {config.build_type_name}")
        print(f"  Compiler:     {config.compiler_name}")
        print(f"  Build system: {config.builder_name}")

        if get_choice("Proceed with this configuration?", "YN", default="Y", option_help="(Y=Yes, N=No)") == 'Y':
            return config
        print_color("Re-enter configuration...", Fore.YELLOW)


def main() -> None:
    system = platform.system()
    release = platform.release()
    print_color(f"System: {system} {release}", Fore.CYAN)
    print_color("Passport-CLI build script", Fore.GREEN, Style.BRIGHT)

    config = get_build_config()
    env_mgr = EnvironmentManager()

    build_dir_handler = BuildDirectory(config.builder)
    build_dir = build_dir_handler.locate()
    build_dir_handler.clean()

    executor = BuildExecutor(config, env_mgr, build_dir)
    executor.run_with_retry()

    exe_path = build_dir / "passport-cli"
    if not exe_path.exists():
        error_exit(f"Executable not found in {build_dir}.")

    blake2_hash, sha256_hash = SignatureVerifier._compute_hashes(exe_path)
    if blake2_hash and sha256_hash:
        print_color("\n--- Hashes of built executable ---", Fore.CYAN, Style.BRIGHT)
        print(f"BLAKE2b: {blake2_hash}")
        print(f"SHA256:  {sha256_hash}")
    else:
        print_color("Unable to compute hashes.", Fore.YELLOW)

    verifier = SignatureVerifier(build_dir)
    verifier.verify()

    print_color("\nBuild completed successfully!", Fore.GREEN, Style.BRIGHT)


if __name__ == "__main__":
    main()