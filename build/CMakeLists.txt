# CMakeLists.txt
# Main build configuration for the passport-cli executable.
# Cross-platform: works on Linux, MSYS2 (MinGW), and others.
# Uses "quiet" detection for ICU library names.

# Specify the minimum CMake version required for features used.
cmake_minimum_required(VERSION 3.15)

# Automatically select the C++ compiler: g++ (priority), otherwise clang++
if(NOT CMAKE_CXX_COMPILER)
    find_program(GXX_COMPILER g++)
    find_program(CLANGXX_COMPILER clang++)
    if(GXX_COMPILER)
        set(CMAKE_CXX_COMPILER "${GXX_COMPILER}" CACHE PATH "C++ compiler" FORCE)
        message(STATUS "Automatically selected C++ compiler: g++")
    elseif(CLANGXX_COMPILER)
        set(CMAKE_CXX_COMPILER "${CLANGXX_COMPILER}" CACHE PATH "C++ compiler" FORCE)
        message(STATUS "Automatically selected C++ compiler: clang++")
    else()
        message(FATAL_ERROR "Neither g++ nor clang++ found. Please install a C++ compiler.")
    endif()
endif()

# Define the project name and declare that only C++ is used.
project(passport-cli LANGUAGES CXX)

# ==== Compiler settings ====
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# ==== Build type ====
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE "Release" CACHE STRING "Build type (Debug or Release)" FORCE)
endif()
message(STATUS "Build type: ${CMAKE_BUILD_TYPE}")

# Absolute path to the root of the source tree.
set(SOURCE_DIR "${CMAKE_CURRENT_LIST_DIR}/../source")

# ==== External libraries: libsodium ====
find_package(PkgConfig QUIET)
if(PKG_CONFIG_FOUND)
    pkg_check_modules(Sodium QUIET libsodium IMPORTED_TARGET)
    if(TARGET PkgConfig::Sodium)
        set(Sodium_FOUND TRUE)
    endif()
endif()

if(NOT Sodium_FOUND)
    find_library(Sodium_LIBRARY sodium)
    find_path(Sodium_INCLUDE_DIR sodium.h)
    if(Sodium_LIBRARY AND Sodium_INCLUDE_DIR)
        add_library(Sodium::Sodium UNKNOWN IMPORTED)
        set_target_properties(Sodium::Sodium PROPERTIES
            IMPORTED_LOCATION ${Sodium_LIBRARY}
            INTERFACE_INCLUDE_DIRECTORIES ${Sodium_INCLUDE_DIR}
        )
        set(Sodium_FOUND TRUE)
    else()
        message(FATAL_ERROR "Libsodium not found. Install libsodium development package.")
    endif()
endif()

# ==== External libraries: ICU (cross‑platform) ====
# ICU (International Components for Unicode) handles text encoding and Unicode.
# We try pkg-config first, then fall back to a manual search that accounts for
# different library names on Linux (icui18n, icudata) and MSYS2/MinGW (icuin, icudt).

# First attempt: pkg-config (most reliable on any system with .pc files)
if(PKG_CONFIG_FOUND)
    pkg_check_modules(ICU_UC QUIET icu-uc IMPORTED_TARGET)
    pkg_check_modules(ICU_I18N QUIET icu-i18n IMPORTED_TARGET)
    if(TARGET PkgConfig::ICU_UC AND TARGET PkgConfig::ICU_I18N)
        set(ICU_FOUND TRUE)
        message(STATUS "Found ICU via pkg-config")
    endif()
endif()

# Manual fallback: search for libraries and header.
# We define lists of possible names, ordered by likelihood.
# The order matters: on Linux, "icui18n" is typical; on MSYS2, "icuin" is typical.
# We also include "lib" prefixed variants for completeness.
if(NOT ICU_FOUND)
    # Try to guess the platform by checking for characteristic files.
    # This is similar to the "quiet" detection in the Makefile.
    if(EXISTS "/usr/lib/libicui18n.so" OR EXISTS "/usr/local/lib/libicui18n.so")
        # Linux-style (icui18n, icudata) – keep default order
        set(ICU_I18N_PREFERRED icui18n icuin libicui18n libicuin)
        set(ICU_DATA_PREFERRED icudata icudt libicudata libicudt)
    elseif(EXISTS "/usr/lib/libicuin.a" OR EXISTS "/usr/local/lib/libicuin.a")
        # MSYS2/MinGW-style (icuin, icudt) – put these first
        set(ICU_I18N_PREFERRED icuin icui18n libicuin libicui18n)
        set(ICU_DATA_PREFERRED icudt icudata libicudt libicudata)
    else()
        # No clue – default to Linux order (most common)
        set(ICU_I18N_PREFERRED icui18n icuin libicui18n libicuin)
        set(ICU_DATA_PREFERRED icudata icudt libicudata libicudt)
    endif()

    # Also define names for the base ICU library (always icuuc)
    set(ICU_UC_NAMES icuuc icuucd libicuuc)

    find_library(ICU_UC_LIBRARY NAMES ${ICU_UC_NAMES})
    find_library(ICU_I18N_LIBRARY NAMES ${ICU_I18N_PREFERRED})
    find_library(ICU_DATA_LIBRARY NAMES ${ICU_DATA_PREFERRED})
    find_path(ICU_INCLUDE_DIR unicode/unistr.h)

    if(ICU_UC_LIBRARY AND ICU_I18N_LIBRARY AND ICU_INCLUDE_DIR)
        # Create an imported target that bundles all ICU dependencies.
        add_library(ICU::ICU UNKNOWN IMPORTED)
        set_target_properties(ICU::ICU PROPERTIES
            IMPORTED_LOCATION "${ICU_UC_LIBRARY}"
            INTERFACE_LINK_LIBRARIES "${ICU_I18N_LIBRARY};${ICU_DATA_LIBRARY}"
            INTERFACE_INCLUDE_DIRECTORIES "${ICU_INCLUDE_DIR}"
        )
        set(ICU_FOUND TRUE)
        message(STATUS "Found ICU via manual search")
    else()
        message(FATAL_ERROR "ICU not found. Install ICU development package.")
    endif()
endif()

# ==== Extract metadata from metadata.hpp ====
set(VERSION_H "${SOURCE_DIR}/modules/core/metadata.hpp")
file(READ "${VERSION_H}" VERSION_CONTENT)

function(get_version_macro name output)
    string(REGEX MATCH "#define[ \t]+${name}[ \t]+\"([^\"]*)\"" match "${VERSION_CONTENT}")
    if(CMAKE_MATCH_1)
        set(${output} "${CMAKE_MATCH_1}" PARENT_SCOPE)
    else()
        set(${output} "" PARENT_SCOPE)
    endif()
endfunction()

get_version_macro("ProductName" PRODUCT_NAME)
get_version_macro("FileDescription" FILE_DESCRIPTION)
get_version_macro("ProductVersion" PRODUCT_VERSION)
get_version_macro("License" LICENSE)
get_version_macro("Copyright" COPYRIGHT)
get_version_macro("Author" AUTHOR)
get_version_macro("Contact" CONTACT)
get_version_macro("Homepage" HOMEPAGE)

# ==== Gather source files ====
file(GLOB_RECURSE SOURCES "${SOURCE_DIR}/*.cpp")
list(SORT SOURCES)

# ==== Define the executable target ====
add_executable(${PROJECT_NAME} ${SOURCES})

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

target_include_directories(${PROJECT_NAME} PRIVATE
    ${SOURCE_DIR}
    ${SOURCE_DIR}/modules
)

# ==== Link external libraries ====
if(TARGET PkgConfig::Sodium)
    target_link_libraries(${PROJECT_NAME} PRIVATE PkgConfig::Sodium)
elseif(TARGET Sodium::Sodium)
    target_link_libraries(${PROJECT_NAME} PRIVATE Sodium::Sodium)
endif()

if(TARGET PkgConfig::ICU_UC)
    target_link_libraries(${PROJECT_NAME} PRIVATE PkgConfig::ICU_UC PkgConfig::ICU_I18N)
elseif(TARGET ICU::ICU)
    target_link_libraries(${PROJECT_NAME} PRIVATE ICU::ICU)
endif()

# ==== Compiler and linker flags ====
target_compile_options(${PROJECT_NAME} PRIVATE
    -Wall -Wextra -pedantic
    -finput-charset=UTF-8 -fexec-charset=UTF-8
)

if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    get_filename_component(SOURCE_ABS "${SOURCE_DIR}" ABSOLUTE)
    get_filename_component(BINARY_ABS "${CMAKE_CURRENT_BINARY_DIR}" ABSOLUTE)
    target_compile_options(${PROJECT_NAME} PRIVATE
        -O0 -g
        -fdebug-prefix-map=${SOURCE_ABS}=.
        -fdebug-prefix-map=${BINARY_ABS}=.
        -fno-ident
        -gno-record-gcc-switches
    )
elseif(CMAKE_BUILD_TYPE STREQUAL "Release")
    target_compile_options(${PROJECT_NAME} PRIVATE -O2 -DNDEBUG -g0)
    target_link_options(${PROJECT_NAME} PRIVATE -s)
    target_link_options(${PROJECT_NAME} PRIVATE -Wl,--build-id=none)
else()
    message(WARNING "Unknown build type '${CMAKE_BUILD_TYPE}'. Defaulting to Debug flags.")
    target_compile_options(${PROJECT_NAME} PRIVATE -O0 -g)
endif()