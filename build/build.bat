@echo off
setlocal enabledelayedexpansion
chcp 65001 >nul  REM Switch console to UTF-8 code page.

:: =============================================================================
:: Universal build script for passport-cli on Windows.
:: Guides the user through dependency checks, compiler selection,
:: and build process using CMake or Make.
:: =============================================================================

:: ---- Helper functions ----
:info
:: Display an informational message with [INFO] prefix.
echo [INFO] %*
exit /b 0

:warn
:: Display a warning message with [WARN] prefix.
echo [WARN] %*
exit /b 0

:error
:: Display an error message with [ERROR] prefix and exit.
echo [ERROR] %*
exit /b 1

:prompt
:: Display a prompt and store the user's input in the variable 'answer'.
set /p "answer=%* "
if "!answer!"=="" set answer=!default!
exit /b 0

:: ==== 1. System detection ====
:: Windows is assumed; no further detection needed.
set OS_TYPE=windows
call :info "Detected OS: Windows"

:: ==== 2. Network check ====
:: Test internet connectivity by pinging google.com.
call :info "Checking internet connection..."
ping -n 1 -w 2000 google.com >nul 2>&1
if %errorlevel% equ 0 (
    call :info "Network is available."
) else (
    call :error "No internet connection. Please check your network and try again."
)

:: ==== 3. Package database update ====
:: Skipped on Windows; no system package manager is used.
call :info "Package update skipped on Windows."

:: ==== 4. Ensure basic tools ====
:: Verify that cmake and make are accessible in the PATH.
call :info "Checking required tools..."

:check_command
:: Check if a given command exists; if not, prompt the user to install it manually.
where %1 >nul 2>nul
if %errorlevel% neq 0 (
    call :warn "Command '%1' not found."
    set /p install_ans="Install it? (y/N): "
    if /i "!install_ans!"=="y" (
        call :error "Please install '%1' manually and add to PATH, then rerun."
    ) else (
        call :error "Tool '%1' is required. Aborting."
    )
)
exit /b 0

call :check_command cmake
call :check_command make

:: ==== 5. Build type selection ====
:: Let the user choose between Debug and Release.
set BUILD_TYPE=Release
set /p choice="Select build type (debug/release) [default=release]: "
if /i "!choice!"=="debug" set BUILD_TYPE=Debug
if /i "!choice!"=="release" set BUILD_TYPE=Release
call :info "Build type: !BUILD_TYPE!"

:: ==== 6. Compiler selection & installation ====
:: Detect available compilers (MSVC, MinGW g++, Clang) and let the user pick.
set "COMPILERS="
set "COMPILER_NAMES="
set COMPILER_INDEX=0

:: Check for Microsoft Visual C++ compiler (cl.exe).
where cl >nul 2>nul
if !errorlevel! equ 0 (
    set /a COMPILER_INDEX+=1
    set "COMPILERS[!COMPILER_INDEX!]=cl"
    set "COMPILER_NAMES[!COMPILER_INDEX!]=Microsoft Visual C++ (cl.exe)"
)

:: Check for MinGW g++.
where g++ >nul 2>nul
if !errorlevel! equ 0 (
    set /a COMPILER_INDEX+=1
    set "COMPILERS[!COMPILER_INDEX!]=g++"
    set "COMPILER_NAMES[!COMPILER_INDEX!]=MinGW g++"
)

:: Check for Clang (clang++ first, then fallback to clang).
where clang++ >nul 2>nul
if !errorlevel! equ 0 (
    set /a COMPILER_INDEX+=1
    set "COMPILERS[!COMPILER_INDEX!]=clang++"
    set "COMPILER_NAMES[!COMPILER_INDEX!]=Clang++"
) else (
    where clang >nul 2>nul
    if !errorlevel! equ 0 (
        set /a COMPILER_INDEX+=1
        set "COMPILERS[!COMPILER_INDEX!]=clang"
        set "COMPILER_NAMES[!COMPILER_INDEX!]=Clang (C compiler, not recommended)"
    )
)

if %COMPILER_INDEX% equ 0 (
    call :error "No C++ compiler found. Install Visual Studio, MinGW, or Clang."
)

echo.
call :info "Available C++ compilers:"
for /l %%i in (1,1,%COMPILER_INDEX%) do (
    echo   [%%i] !COMPILER_NAMES[%%i]!
)
set /p comp_choice="Select compiler (enter number, default=1): "
if "!comp_choice!"=="" set comp_choice=1
set SELECTED_COMPILER=!COMPILERS[%comp_choice%]!
if "!SELECTED_COMPILER!"=="" call :error "Invalid selection."
call :info "Selected compiler: !SELECTED_COMPILER!"

:: If the user chose plain 'clang', suggest switching to clang++.
if "!SELECTED_COMPILER!"=="clang" (
    call :warn "You selected 'clang' (C compiler). For C++ code, it's recommended to use 'clang++'."
    set /p use_clangpp="Do you want to use 'clang++' instead? (Y/n) [default=Y]: "
    if "!use_clangpp!"=="" set use_clangpp=Y
    if /i "!use_clangpp!"=="Y" (
        set SELECTED_COMPILER=clang++
        call :info "Switched to compiler: !SELECTED_COMPILER!"
        where clang++ >nul 2>nul || call :error "clang++ not found. Please install it."
    )
)

:: ==== 7. Builder selection ====
:: Ask whether to build with CMake or Make.
set BUILDER=
set /p builder_choice="Select builder: [1] CMake, [2] Make (default=1): "
if "!builder_choice!"=="" set builder_choice=1
if "!builder_choice!"=="1" (
    set BUILDER=cmake
    if not exist "%~dp0CMakeLists.txt" call :error "CMakeLists.txt not found in %~dp0."
) else if "!builder_choice!"=="2" (
    set BUILDER=make
    if not exist "%~dp0Makefile" call :error "Makefile not found in %~dp0."
) else (
    call :error "Invalid builder selection."
)
call :info "Selected builder: !BUILDER!"

:: ==== 8. Preparation ====
:: Change to the script's directory and clean previous build artifacts.
cd /d "%~dp0" || call :error "Cannot cd to script directory."

call :info "Cleaning up old build artifacts..."
if exist CMakeCache.txt del /Q CMakeCache.txt
if exist CMakeFiles rmdir /S /Q CMakeFiles
if exist MakeFiles rmdir /S /Q MakeFiles

:: Check for required libraries (libsodium and ICU).
:: On Windows, we cannot auto‑install; we ask the user to confirm they are installed.
call :info "Checking required libraries (libsodium, ICU)..."
call :info "Please ensure libsodium and ICU are installed and accessible."
call :info "If using vcpkg, set VCPKG_ROOT and add appropriate flags."
set /p lib_ok="Have you installed libsodium and ICU? (y/N): "
if /i not "!lib_ok!"=="y" call :error "Please install libsodium and ICU before continuing."

:: ==== 9. Build ====
:: Execute the build with the chosen builder and compiler.
if "!BUILDER!"=="cmake" (
    call :info "Configuring with CMake..."
    if "!SELECTED_COMPILER!"=="cl" (
        set "EXTRA_CMAKE_OPTS="
    ) else if "!SELECTED_COMPILER!"=="clang" (
        set "EXTRA_CMAKE_OPTS=-DCMAKE_CXX_FLAGS=-stdlib=libstdc++ -DCMAKE_EXE_LINKER_FLAGS=-lstdc++"
    ) else (
        set "EXTRA_CMAKE_OPTS=-DCMAKE_CXX_COMPILER=!SELECTED_COMPILER!"
    )
    cmake -DCMAKE_BUILD_TYPE=!BUILD_TYPE! !EXTRA_CMAKE_OPTS! .
    if !errorlevel! neq 0 call :error "CMake configuration failed."
    call :info "Building..."
    cmake --build . --config !BUILD_TYPE! --parallel
) else if "!BUILDER!"=="make" (
    call :info "Building with Make..."
    set CXX=!SELECTED_COMPILER!
    set "BUILD_TYPE_LOWER=!BUILD_TYPE!"
    if /i "!BUILD_TYPE_LOWER!"=="DEBUG" set BUILD_TYPE_LOWER=debug
    if /i "!BUILD_TYPE_LOWER!"=="RELEASE" set BUILD_TYPE_LOWER=release
    if "!SELECTED_COMPILER!"=="clang" (
        set CXXFLAGS=!CXXFLAGS! -stdlib=libstdc++
        set LDFLAGS=!LDFLAGS! -lstdc++
    )
    :: Determine number of logical processors for parallel build.
    for /f "tokens=2 delims==" %%i in ('wmic cpu get NumberOfLogicalProcessors /value ^| find "="') do set NPROC=%%i
    if "!NPROC!"=="" set NPROC=1
    make BUILD=!BUILD_TYPE_LOWER! -j!NPROC!
)

if %errorlevel% neq 0 call :error "Build failed."

:: ==== 10. Install ====
:: Build succeeded; inform user about the executable and offer manual copy.
call :info "Build successful!"
echo Executable: %cd%\passport-cli.exe
set /p install_ans="Do you want to copy the executable to a system directory? (y/N): "
if /i "!install_ans!"=="y" (
    call :warn "Please copy passport-cli.exe manually to a directory in your PATH."
) else (
    call :info "Installation skipped."
)

exit /b 0