cmake_minimum_required(VERSION 3.15)
project(passport-cli VERSION 0.0.1 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

find_package(PkgConfig QUIET)
if(PKG_CONFIG_FOUND)
    pkg_check_modules(Sodium REQUIRED libsodium IMPORTED_TARGET)
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
        message(FATAL_ERROR "libsodium not found. Install libsodium development package.")
    endif()
endif()

file(GLOB_RECURSE SOURCES "src/*.cpp")

add_executable(${PROJECT_NAME} ${SOURCES})

target_include_directories(${PROJECT_NAME} PRIVATE
    src
    src/modules
)

if(TARGET PkgConfig::Sodium)
    target_link_libraries(${PROJECT_NAME} PRIVATE PkgConfig::Sodium)
elseif(TARGET Sodium::Sodium)
    target_link_libraries(${PROJECT_NAME} PRIVATE Sodium::Sodium)
endif()

if(MSVC)
    target_compile_options(${PROJECT_NAME} PRIVATE /W4)
else()
    target_compile_options(${PROJECT_NAME} PRIVATE -Wall -Wextra -pedantic)
endif()