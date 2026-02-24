# CMakeLists.txt
# Main build configuration for the passport-cli executable.

# Specify the minimum CMake version required for features used.
cmake_minimum_required(VERSION 3.15)

# Automatically select the C++ compiler: g++ (priority), otherwise clang++
# If the compiler is already specified in the cache (by the user or a previous build), the block is not executed.
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
# Enforce the C++20 standard and disable compiler-specific extensions.
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# ==== Build type ====
# If the user did not choose a build type, default to Release.
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE "Release" CACHE STRING "Build type (Debug or Release)" FORCE)
endif()
message(STATUS "Build type: ${CMAKE_BUILD_TYPE}")

# Absolute path to the root of the source tree.
set(SOURCE_DIR "${CMAKE_CURRENT_LIST_DIR}/../source")

# ==== External libraries: libsodium ====
# libsodium provides cryptographic primitives. It is required.

# First attempt to locate libsodium using pkg-config (clean and portable).
find_package(PkgConfig QUIET)
if(PKG_CONFIG_FOUND)
    pkg_check_modules(Sodium QUIET libsodium IMPORTED_TARGET)
    if(TARGET PkgConfig::Sodium)
        set(Sodium_FOUND TRUE)
    endif()
endif()

# If pkg-config failed, fall back to a manual search of library and header.
if(NOT Sodium_FOUND)
    find_library(Sodium_LIBRARY sodium)
    find_path(Sodium_INCLUDE_DIR sodium.h)
    if(Sodium_LIBRARY AND Sodium_INCLUDE_DIR)
        # Create an imported target so we can refer to it uniformly.
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

# ==== External libraries: ICU ====
# ICU (International Components for Unicode) handles text encoding and Unicode.

# Try pkg-config first for ICU components.
if(PKG_CONFIG_FOUND)
    pkg_check_modules(ICU_UC QUIET icu-uc IMPORTED_TARGET)
    pkg_check_modules(ICU_I18N QUIET icu-i18n IMPORTED_TARGET)
    if(TARGET PkgConfig::ICU_UC AND TARGET PkgConfig::ICU_I18N)
        set(ICU_FOUND TRUE)
    endif()
endif()

# Manual fallback: search for the libraries and header.
if(NOT ICU_FOUND)
    # Library names may vary across platforms and ICU versions.
    set(ICU_UC_NAMES icuuc icuucd libicuuc)
    set(ICU_I18N_NAMES icui18n icuin icui18nd icuind libicui18n)
    set(ICU_DATA_NAMES icudata icudt icudt72 icudt73 libicudata)

    find_library(ICU_UC_LIBRARY NAMES ${ICU_UC_NAMES})
    find_library(ICU_I18N_LIBRARY NAMES ${ICU_I18N_NAMES})
    find_library(ICU_DATA_LIBRARY NAMES ${ICU_DATA_NAMES})
    find_path(ICU_INCLUDE_DIR unicode/unistr.h)

    if(ICU_UC_LIBRARY AND ICU_I18N_LIBRARY AND ICU_INCLUDE_DIR)
        # Create a single imported target that bundles all ICU dependencies.
        add_library(ICU::ICU UNKNOWN IMPORTED)
        set_target_properties(ICU::ICU PROPERTIES
            IMPORTED_LOCATION "${ICU_UC_LIBRARY}"
            INTERFACE_LINK_LIBRARIES "${ICU_I18N_LIBRARY};${ICU_DATA_LIBRARY}"
            INTERFACE_INCLUDE_DIRECTORIES "${ICU_INCLUDE_DIR}"
        )
        set(ICU_FOUND TRUE)
    else()
        message(FATAL_ERROR "ICU not found. Install ICU development package.")
    endif()
endif()

# ==== Extract metadata from metadata.hpp ====
# The file metadata.hpp contains project information such as version,
# author, copyright, etc. We read these macros to embed them into the binary.

set(VERSION_H "${SOURCE_DIR}/modules/core/metadata.hpp")
file(READ "${VERSION_H}" VERSION_CONTENT)

# Helper: extract a quoted string macro from the header.
function(get_version_macro name output)
    string(REGEX MATCH "#define[ \t]+${name}[ \t]+\"([^\"]*)\"" match "${VERSION_CONTENT}")
    if(CMAKE_MATCH_1)
        set(${output} "${CMAKE_MATCH_1}" PARENT_SCOPE)
    else()
        set(${output} "" PARENT_SCOPE)
    endif()
endfunction()

# Retrieve all metadata fields.
get_version_macro("ProductName" PRODUCT_NAME)
get_version_macro("FileDescription" FILE_DESCRIPTION)
get_version_macro("ProductVersion" PRODUCT_VERSION)
get_version_macro("License" LICENSE)
get_version_macro("Copyright" COPYRIGHT)
get_version_macro("Author" AUTHOR)
get_version_macro("Contact" CONTACT)
get_version_macro("Homepage" HOMEPAGE)

# ==== Gather source files ====
# Recursively collect all .cpp files under SOURCE_DIR and sort them
# for reproducible builds.
file(GLOB_RECURSE SOURCES "${SOURCE_DIR}/*.cpp")
list(SORT SOURCES)

# ==== Define the executable target ====
add_executable(${PROJECT_NAME} ${SOURCES})

# Embed the metadata as preprocessor definitions so they can be used in code.
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

# Add source directories to the include path.
target_include_directories(${PROJECT_NAME} PRIVATE
    ${SOURCE_DIR}
    ${SOURCE_DIR}/modules
)

# ==== Link external libraries ====
# Link against libsodium using either the pkg-config target or our own.
if(TARGET PkgConfig::Sodium)
    target_link_libraries(${PROJECT_NAME} PRIVATE PkgConfig::Sodium)
elseif(TARGET Sodium::Sodium)
    target_link_libraries(${PROJECT_NAME} PRIVATE Sodium::Sodium)
endif()

# Link ICU similarly.
if(TARGET PkgConfig::ICU_UC)
    target_link_libraries(${PROJECT_NAME} PRIVATE PkgConfig::ICU_UC PkgConfig::ICU_I18N)
elseif(TARGET ICU::ICU)
    target_link_libraries(${PROJECT_NAME} PRIVATE ICU::ICU)
endif()

# ==== Compiler and linker flags ====
# Set warnings, UTF-8 handling, and flags specific to the build type.
target_compile_options(${PROJECT_NAME} PRIVATE
    -Wall -Wextra -pedantic
    -finput-charset=UTF-8 -fexec-charset=UTF-8
)

# Build‑type specific flags: Debug vs Release.
if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    # Normalize source and binary paths in debug info for reproducible builds.
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