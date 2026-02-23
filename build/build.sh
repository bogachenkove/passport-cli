#!/bin/bash
# Universal build script for passport-cli on Linux/macOS.
# Executes a step-by-step build process including dependency checks,
# compiler selection, and final installation.

set -e  # Exit immediately if any command fails.

# Color codes for console output formatting.
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

# :info - Print an informational message in green.
info()  { echo -e "${GREEN}[INFO]${NC} $1"; }
# :warn - Print a warning message in yellow.
warn()  { echo -e "${YELLOW}[WARN]${NC} $1"; }
# :error - Print an error message in red and exit.
error() { echo -e "${RED}[ERROR]${NC} $1"; exit 1; }
# :prompt - Print a prompt message in blue.
prompt() { echo -e "${BLUE}[PROMPT]${NC} $1"; }

# ==== 1. System detection ====
# Determine whether the OS is Linux or macOS.
OS="$(uname -s)"
case "$OS" in
    Linux*)  OS_TYPE=linux ;;
    Darwin*) OS_TYPE=macos ;;
    *)       error "Unsupported operating system: $OS" ;;
esac
info "Detected OS: $OS_TYPE"

# ==== 2. Network check ====
# Verify internet connectivity by pinging google.com.
info "Checking internet connection..."
if ping -c 1 -W 2 google.com &>/dev/null; then
    info "Network is available."
else
    error "No internet connection. Please check your network and try again."
fi

# ==== 3. Package database update ====
# Refresh the system package manager's cache (best effort).
info "Updating package database..."
if [ "$OS_TYPE" = "linux" ]; then
    if [ -f /etc/os-release ]; then
        . /etc/os-release
        DISTRO="$ID"
    else
        DISTRO="unknown"
    fi
    case "$DISTRO" in
        ubuntu|debian)
            sudo apt update || warn "Update failed, continuing anyway..."
            ;;
        fedora)
            sudo dnf check-update || warn "Update check failed, continuing anyway..."
            ;;
        centos|rhel)
            sudo yum check-update || warn "Update check failed, continuing anyway..."
            ;;
        arch)
            sudo pacman -Sy || warn "Update failed, continuing anyway..."
            ;;
        *)
            warn "Automatic package update not supported for $DISTRO. Skipping."
            ;;
    esac
elif [ "$OS_TYPE" = "macos" ]; then
    if command -v brew &>/dev/null; then
        brew update || warn "Homebrew update failed, continuing anyway..."
    else
        warn "Homebrew not found, skipping package update."
    fi
fi

# ==== 4. Ensure basic tools ====
# Check for the presence of cmake, make, and pkg-config; offer to install if missing.
info "Checking required tools..."

# :install_tool - Install a given package using the appropriate system package manager.
install_tool() {
    local tool=$1
    local pkg_name=$2
    prompt "Tool '$tool' is missing. Install it? (y/N): "
    read -r answer
    if [[ "$answer" =~ ^[Yy]$ ]]; then
        if [ "$OS_TYPE" = "linux" ]; then
            case "$DISTRO" in
                ubuntu|debian) sudo apt install -y "$pkg_name" ;;
                fedora)        sudo dnf install -y "$pkg_name" ;;
                centos|rhel)   sudo yum install -y "$pkg_name" ;;
                arch)          sudo pacman -S --noconfirm "$pkg_name" ;;
                *)             error "Cannot install $tool automatically on this distro." ;;
            esac
        elif [ "$OS_TYPE" = "macos" ]; then
            if command -v brew &>/dev/null; then
                brew install "$pkg_name"
            else
                error "Homebrew not found, cannot install $tool."
            fi
        fi
    else
        error "Tool '$tool' is required. Aborting."
    fi
}

if ! command -v cmake &>/dev/null; then
    install_tool cmake cmake
fi
if ! command -v make &>/dev/null; then
    install_tool make make
fi
if ! command -v pkg-config &>/dev/null; then
    if [ "$OS_TYPE" = "linux" ]; then
        install_tool pkg-config pkg-config
    elif [ "$OS_TYPE" = "macos" ]; then
        install_tool pkg-config pkg-config
    fi
fi

info "All basic tools are present."

# ==== 5. Build type selection ====
# Let the user choose between Debug and Release build.
BUILD_TYPE="Release"
prompt "Select build type (debug/release) [default=release]: "
read -r choice
if [[ "$choice" =~ ^[Dd]ebug$ ]]; then
    BUILD_TYPE="Debug"
elif [[ "$choice" =~ ^[Rr]elease$ ]]; then
    BUILD_TYPE="Release"
elif [[ -z "$choice" ]]; then
    BUILD_TYPE="Release"
else
    warn "Invalid input, using Release."
fi
info "Build type: $BUILD_TYPE"

# ==== 6. Compiler selection & installation ====
# List available C++ compilers (g++, clang++, clang) and let the user pick one.
COMPILERS=()
for comp in g++ clang++ clang; do
    if command -v "$comp" &>/dev/null; then
        COMPILERS+=("$comp")
    fi
done

if [ ${#COMPILERS[@]} -eq 0 ]; then
    error "No C++ compiler found. Please install one (g++, clang++)."
fi

echo ""
info "Available C++ compilers:"
for i in "${!COMPILERS[@]}"; do
    echo "  [$((i+1))] ${COMPILERS[$i]}"
done
prompt "Select compiler (enter number, default=1): "
read -r comp_choice
if [[ -z "$comp_choice" ]]; then
    comp_choice=1
fi
if ! [[ "$comp_choice" =~ ^[0-9]+$ ]] || [ "$comp_choice" -lt 1 ] || [ "$comp_choice" -gt ${#COMPILERS[@]} ]; then
    error "Invalid selection."
fi
SELECTED_COMPILER="${COMPILERS[$((comp_choice-1))]}"
info "Selected compiler: $SELECTED_COMPILER"

# If the user selected plain 'clang', suggest switching to clang++.
if [ "$SELECTED_COMPILER" = "clang" ]; then
    warn "You selected 'clang' (C compiler). For C++ code, it's recommended to use 'clang++'."
    prompt "Do you want to use 'clang++' instead? (Y/n): "
    read -r use_clangpp
    if [[ -z "$use_clangpp" || "$use_clangpp" =~ ^[Yy]$ ]]; then
        SELECTED_COMPILER="clang++"
        info "Switched to compiler: $SELECTED_COMPILER"
        # Ensure clang++ is installed (package name may be 'clang').
        if ! command -v clang++ &>/dev/null; then
            install_tool clang++ clang
        fi
    else
        info "Proceeding with 'clang'."
    fi
fi

# ==== 7. Builder selection ====
# Ask the user whether to build with CMake or plain Make.
BUILDER=""
prompt "Select builder: [1] CMake, [2] Make (default=1): "
read -r builder_choice
if [[ -z "$builder_choice" || "$builder_choice" == "1" ]]; then
    BUILDER="cmake"
    if [ ! -f "$(dirname "$0")/CMakeLists.txt" ]; then
        error "CMakeLists.txt not found in $(dirname "$0"). Cannot use CMake builder."
    fi
elif [[ "$builder_choice" == "2" ]]; then
    BUILDER="make"
    if [ ! -f "$(dirname "$0")/Makefile" ]; then
        error "Makefile not found in $(dirname "$0"). Cannot use Make builder."
    fi
else
    error "Invalid builder selection."
fi
info "Selected builder: $BUILDER"

# ==== 8. Preparation ====
# Change to the script's directory and remove previous build artifacts.
cd "$(dirname "$0")" || error "Cannot cd to script directory."

info "Cleaning up old build artifacts..."
if [ -e CMakeCache.txt ]; then rm -f CMakeCache.txt; fi
if [ -d CMakeFiles ]; then rm -rf CMakeFiles; fi
if [ -d MakeFiles ]; then rm -rf MakeFiles; fi

# Check for required libraries: libsodium and ICU.
info "Checking required libraries (libsodium, ICU)..."

# :install_pkg - Install one or more packages using the system package manager.
install_pkg() {
    local pkg_names=("$@")
    if [ "$OS_TYPE" = "linux" ]; then
        case "$DISTRO" in
            ubuntu|debian) sudo apt install -y "${pkg_names[@]}" ;;
            fedora)        sudo dnf install -y "${pkg_names[@]}" ;;
            centos|rhel)   sudo yum install -y "${pkg_names[@]}" ;;
            arch)          sudo pacman -S --noconfirm "${pkg_names[@]}" ;;
            *)             error "Cannot install packages automatically on this distro." ;;
        esac
    elif [ "$OS_TYPE" = "macos" ]; then
        if command -v brew &>/dev/null; then
            brew install "${pkg_names[@]}"
        else
            error "Homebrew not found, cannot install packages."
        fi
    fi
}

# Verify libsodium via pkg-config.
if ! pkg-config --exists libsodium; then
    warn "libsodium not found."
    prompt "Install libsodium? (y/N): "
    read -r ans
    if [[ "$ans" =~ ^[Yy]$ ]]; then
        if [ "$OS_TYPE" = "linux" ]; then
            case "$DISTRO" in
                ubuntu|debian) install_pkg libsodium-dev ;;
                fedora)        install_pkg libsodium-devel ;;
                centos|rhel)   install_pkg libsodium-devel ;;
                arch)          install_pkg libsodium ;;
                *)             error "Unsupported distro." ;;
            esac
        elif [ "$OS_TYPE" = "macos" ]; then
            install_pkg libsodium
        fi
    else
        error "libsodium is required. Aborting."
    fi
fi

# Verify ICU libraries (icu-uc and icu-i18n).
if ! pkg-config --exists icu-uc || ! pkg-config --exists icu-i18n; then
    warn "ICU libraries not found."
    prompt "Install ICU development packages? (y/N): "
    read -r ans
    if [[ "$ans" =~ ^[Yy]$ ]]; then
        if [ "$OS_TYPE" = "linux" ]; then
            case "$DISTRO" in
                ubuntu|debian) install_pkg libicu-dev ;;
                fedora)        install_pkg libicu-devel ;;
                centos|rhel)   install_pkg libicu-devel ;;
                arch)          install_pkg icu ;;
                *)             error "Unsupported distro." ;;
            esac
        elif [ "$OS_TYPE" = "macos" ]; then
            install_pkg icu4c
        fi
    else
        error "ICU is required. Aborting."
    fi
fi

info "All required libraries are present."

# ==== 9. Build ====
# Execute the actual build using the chosen builder and compiler.
if [ "$BUILDER" = "cmake" ]; then
    info "Configuring with CMake..."
    export CXX="$SELECTED_COMPILER"
    # Special flags when using clang as the C++ compiler.
    if [ "$SELECTED_COMPILER" = "clang" ]; then
        export CXXFLAGS="-stdlib=libstdc++"
        export LDFLAGS="-lstdc++"
    fi
    cmake -DCMAKE_BUILD_TYPE="$BUILD_TYPE" .
    if [ $? -ne 0 ]; then
        error "CMake configuration failed."
    fi
    info "Building..."
    cmake --build . --parallel "$(nproc)"
elif [ "$BUILDER" = "make" ]; then
    info "Building with Make..."
    export CXX="$SELECTED_COMPILER"
    BUILD_TYPE_LOWER=$(echo "$BUILD_TYPE" | tr '[:upper:]' '[:lower:]')
    if [ "$SELECTED_COMPILER" = "clang" ]; then
        export CXXFLAGS="$CXXFLAGS -stdlib=libstdc++"
        export LDFLAGS="$LDFLAGS -lstdc++"
    fi
    make BUILD="$BUILD_TYPE_LOWER" -j"$(nproc)"
fi

if [ $? -ne 0 ]; then
    error "Build failed."
fi

# ==== 10. Install ====
# Offer to install the compiled executable to /usr/local/bin.
prompt "Do you want to install the executable to /usr/local/bin? (y/N): "
read -r install_ans
if [[ "$install_ans" =~ ^[Yy]$ ]]; then
    if [ "$OS_TYPE" = "linux" ] || [ "$OS_TYPE" = "macos" ]; then
        sudo install -m 755 passport-cli /usr/local/bin/
        info "Installed to /usr/local/bin/passport-cli"
    else
        warn "Install not supported on this OS. Please copy the executable manually."
    fi
else
    info "Installation skipped."
fi

info "Build script finished successfully."