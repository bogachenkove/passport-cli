# =============================================================================
#   CMake build script for passport-cli
#   Minimum CMake version: 3.15
#   Project name: passport-cli
#   Language: C++
# =============================================================================

cmake_minimum_required(VERSION 3.15)

# ---- Project definition ----
# Define the project name, version, and language.
# The version here is a fallback; actual metadata will be taken from metadata.hpp.
project(passport-cli LANGUAGES CXX)

# ---- Compiler settings ----
# Force C++20 standard, require it, and disable compiler extensions.
# This ensures consistent behavior across different compilers.
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# ---- Source directory ----
# The source code is located one level above the build folder,
# in a directory named "source". Store the absolute path for later use.
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

# ---- Extract metadata from metadata.hpp ----
# Path to the version header (adjust if necessary)
set(VERSION_H "${SOURCE_DIR}/core/metadata.hpp")

# Read the entire version header file into a variable.
file(READ "${VERSION_H}" VERSION_CONTENT)

# Helper function to extract a string macro value.
# Searches for lines like: #define NAME "value" and returns the quoted string.
function(get_version_macro name output)
    string(REGEX MATCH "#define[ \t]+${name}[ \t]+\"([^\"]*)\"" match "${VERSION_CONTENT}")
    if(CMAKE_MATCH_1)
        set(${output} "${CMAKE_MATCH_1}" PARENT_SCOPE)
    else()
        set(${output} "" PARENT_SCOPE)
    endif()
endfunction()

# Extract all needed metadata fields.
get_version_macro("ProductName" PRODUCT_NAME)
get_version_macro("FileDescription" FILE_DESCRIPTION)
get_version_macro("ProductVersion" PRODUCT_VERSION)
get_version_macro("License" LICENSE)
get_version_macro("Copyright" COPYRIGHT)
get_version_macro("Author" AUTHOR)
get_version_macro("Contact" CONTACT)
get_version_macro("Homepage" HOMEPAGE)

# ---- Gather all source files ----
# Recursively collect every .cpp file inside SOURCE_DIR.
# Using GLOB is convenient, but note that CMake will not automatically
# detect new files; you may need to re-run CMake manually.
file(GLOB_RECURSE SOURCES "${SOURCE_DIR}/*.cpp")

# ---- Create the executable ----
add_executable(${PROJECT_NAME} ${SOURCES})

# ---- Pass metadata as compile definitions ----
# These definitions can be used in the source code (e.g., for --version output).
target_compile_definitions(${PROJECT_NAME} PRIVATE
    PRODUCT_NAME="${PRODUCT_NAME}"
    FILE_DESCRIPTION="${FILE_DESCRIPTION}"
    PRODUCT_VERSION="${PRODUCT_VERSION}"
    LICENSE="${LICENSE}"
    COPYRIGHT="${COPYRIGHT}"
    AUTHOR="${AUTHOR}"
    CONTACT="${CONTACT}"
    HOMEPAGE="${HOMEPAGE}"
)

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
    target_compile_options(${PROJECT_NAME} PRIVATE /W4 /utf-8)
elseif(CMAKE_COMPILER_IS_GNUCXX)
    # For GCC, set UTF-8 execution charset
    target_compile_options(${PROJECT_NAME} PRIVATE -Wall -Wextra -pedantic -finput-charset=UTF-8 -fexec-charset=UTF-8 -fwide-exec-charset=UTF-8)
elseif(CLANG)
    if(MSVC_TOOLCHAIN)
        target_compile_options(${PROJECT_NAME} PRIVATE /utf-8)
    else()
        # For Clang, set a similar set of flags
        target_compile_options(${PROJECT_NAME} PRIVATE -Wall -Wextra -pedantic -finput-charset=UTF-8 -fexec-charset=UTF-8)
    endif()
endif()