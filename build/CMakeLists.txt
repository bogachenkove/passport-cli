cmake_minimum_required(VERSION 3.15)

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

project(passport-cli LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE "Release" CACHE STRING "Build type (Debug or Release)" FORCE)
endif()
message(STATUS "Build type: ${CMAKE_BUILD_TYPE}")

set(SOURCE_DIR "${CMAKE_CURRENT_LIST_DIR}/../source")

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

if(PKG_CONFIG_FOUND)
    pkg_check_modules(ICU_UC QUIET icu-uc IMPORTED_TARGET)
    pkg_check_modules(ICU_I18N QUIET icu-i18n IMPORTED_TARGET)
    if(TARGET PkgConfig::ICU_UC AND TARGET PkgConfig::ICU_I18N)
        set(ICU_FOUND TRUE)
        message(STATUS "Found ICU via pkg-config")
    endif()
endif()

if(NOT ICU_FOUND)
    if(EXISTS "/usr/lib/libicui18n.so" OR EXISTS "/usr/local/lib/libicui18n.so" OR
       EXISTS "/usr/lib/libicui18n.a" OR EXISTS "/usr/local/lib/libicui18n.a")
        set(ICU_I18N_NAMES icui18n icuin)
        set(ICU_DATA_NAMES icudata icudt)
    elseif(EXISTS "/usr/lib/libicuin.a" OR EXISTS "/usr/local/lib/libicuin.a" OR
           EXISTS "/usr/lib/libicuin.so" OR EXISTS "/usr/local/lib/libicuin.so")
        set(ICU_I18N_NAMES icuin icui18n)
        set(ICU_DATA_NAMES icudt icudata)
    else()
        set(ICU_I18N_NAMES icui18n icuin)
        set(ICU_DATA_NAMES icudata icudt)
    endif()

    set(ICU_UC_NAMES icuuc icuucd)

    find_library(ICU_UC_LIBRARY NAMES ${ICU_UC_NAMES})
    find_library(ICU_I18N_LIBRARY NAMES ${ICU_I18N_NAMES})
    find_library(ICU_DATA_LIBRARY NAMES ${ICU_DATA_NAMES})
    find_path(ICU_INCLUDE_DIR unicode/unistr.h)

    if(ICU_UC_LIBRARY AND ICU_I18N_LIBRARY AND ICU_INCLUDE_DIR)
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

file(GLOB_RECURSE SOURCES "${SOURCE_DIR}/*.cpp")
list(SORT SOURCES)

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
    target_link_options(${PROJECT_NAME} PRIVATE -s -Wl,--build-id=none)
else()
    message(WARNING "Unknown build type '${CMAKE_BUILD_TYPE}'. Defaulting to Debug flags.")
    target_compile_options(${PROJECT_NAME} PRIVATE -O0 -g)
endif()