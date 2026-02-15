# =============================================================================
#   CMake build script for passport-cli
#   Minimum CMake version: 3.15
#   Project name: passport-cli
#   Language: C++
# =============================================================================

cmake_minimum_required(VERSION 3.15)
project(passport-cli VERSION 0.0.1 LANGUAGES CXX)

# ---- Compiler settings ----
# Force C++20 standard, require it, and disable compiler extensions
# to ensure consistent behaviour across different compilers.
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# ---- Source directory ----
# The source code is located one level above the build folder,
# in a directory named "source". We store the absolute path for later use.
set(SOURCE_DIR "${CMAKE_CURRENT_LIST_DIR}/../source")

# ---- Find libsodium (required for encryption) ----
# First try using pkg-config (common on Unix systems).
find_package(PkgConfig QUIET)
if(PKG_CONFIG_FOUND)
    pkg_check_modules(Sodium REQUIRED libsodium IMPORTED_TARGET)
    if(TARGET PkgConfig::Sodium)
        set(Sodium_FOUND TRUE)
    endif()
endif()

# If pkg-config didn't work, fall back to manual search.
# This helps on Windows or when pkg-config is not available.
if(NOT Sodium_FOUND)
    find_library(Sodium_LIBRARY sodium)
    find_path(Sodium_INCLUDE_DIR sodium.h)
    if(Sodium_LIBRARY AND Sodium_INCLUDE_DIR)
        # Create an imported target so we can link against it uniformly.
        add_library(Sodium::Sodium UNKNOWN IMPORTED)
        set_target_properties(Sodium::Sodium PROPERTIES
            IMPORTED_LOCATION ${Sodium_LIBRARY}
            INTERFACE_INCLUDE_DIRECTORIES ${Sodium_INCLUDE_DIR}
        )
        set(Sodium_FOUND TRUE)
    else()
        # If libsodium is not found, stop with a clear error message.
        message(FATAL_ERROR "libsodium not found. Install libsodium development package.")
    endif()
endif()

# ---- Gather all source files ----
# Recursively collect every .cpp file inside SOURCE_DIR.
# Using GLOB is convenient, but note that CMake will not automatically
# detect new files; you may need to re-run CMake manually.
file(GLOB_RECURSE SOURCES "${SOURCE_DIR}/*.cpp")

# ---- Create the executable ----
add_executable(${PROJECT_NAME} ${SOURCES})

# ---- Set include directories ----
# Tell the compiler where to look for header files.
# Both the root source directory and the modules subdirectory are included.
target_include_directories(${PROJECT_NAME} PRIVATE
    ${SOURCE_DIR}
    ${SOURCE_DIR}/modules
)

# ---- Link with libsodium ----
# Use either the pkg-config target or our manually created target.
if(TARGET PkgConfig::Sodium)
    target_link_libraries(${PROJECT_NAME} PRIVATE PkgConfig::Sodium)
elseif(TARGET Sodium::Sodium)
    target_link_libraries(${PROJECT_NAME} PRIVATE Sodium::Sodium)
endif()

# ---- Compiler warnings (optional but recommended) ----
if(MSVC)
    # On MSVC, enable level 4 warnings.
    target_compile_options(${PROJECT_NAME} PRIVATE /W4)
else()
    # On GCC/Clang, enable a set of common warning flags.
    target_compile_options(${PROJECT_NAME} PRIVATE -Wall -Wextra -pedantic)
endif()