# Makefile
# Production Makefile for passport-cli. Supports Linux, macOS, and Windows (MinGW/MSYS).

# ==== Build type selection ====
# User can set BUILD=debug or BUILD=release. Default is release.
BUILD ?= release
ifeq ($(BUILD),debug)
    BUILD_MODE := Debug
else ifeq ($(BUILD),release)
    BUILD_MODE := Release
else
    $(error BUILD must be either 'debug' or 'release' (default: release))
endif

# Name of the final executable.
TARGET = passport-cli

# ==== Directory layout ====
# Source code root (relative to this Makefile's location).
SRC_DIR = ../source
# Directory where object files will be placed.
OBJ_DIR = ./MakeFiles/$(TARGET).dir

# ==== Source files ====
# Recursively find all .cpp files under SRC_DIR and sort them.
SOURCES := $(shell find $(SRC_DIR) -name "*.cpp" | sort)
# Transform source paths into object paths (preserving subdirectory structure).
OBJECTS := $(addprefix $(OBJ_DIR)/,$(patsubst $(SRC_DIR)/%,%,$(SOURCES:.cpp=.cpp.o)))

# ==== Compiler and base flags ====
CXX = g++
CXXFLAGS = -std=c++20
LDFLAGS =
LIBS =

# Detect the operating system for platform-specific adjustments.
UNAME_S := $(shell uname -s 2>/dev/null || echo Windows)

# ==== Dependency: libsodium ====
# libsodium provides cryptographic functions. It is mandatory.

# Check if pkg-config is available.
PKG_CONFIG := $(shell command -v pkg-config 2>/dev/null)
LIBSODIUM_FOUND := 0

# If SODIUMROOT is set, use it as a prefix (manual installation).
ifdef SODIUMROOT
    ifeq (,$(wildcard $(SODIUMROOT)/include/sodium.h))
        $(error SODIUMROOT is set but sodium.h not found in $(SODIUMROOT)/include)
    endif
    CXXFLAGS += -I$(SODIUMROOT)/include
    LDFLAGS  += -L$(SODIUMROOT)/lib
    LIBS     += -lsodium
    LIBSODIUM_FOUND := 1
else
    # Otherwise try to get flags from pkg-config.
    ifdef PKG_CONFIG
        SODIUM_CFLAGS := $(shell $(PKG_CONFIG) --cflags libsodium 2>/dev/null)
        SODIUM_LIBS   := $(shell $(PKG_CONFIG) --libs   libsodium 2>/dev/null)
        ifneq ($(SODIUM_CFLAGS),)
            CXXFLAGS += $(SODIUM_CFLAGS)
            LIBS     += $(SODIUM_LIBS)
            LIBSODIUM_FOUND := 1
        endif
    endif
endif

# If still not found, search in standard system paths (Unix only).
ifeq ($(LIBSODIUM_FOUND),0)
    ifneq ($(UNAME_S), Windows)
        ifneq (,$(wildcard /usr/include/sodium.h))
            CXXFLAGS += -I/usr/include
            LIBSODIUM_FOUND := 1
        else ifneq (,$(wildcard /usr/local/include/sodium.h))
            CXXFLAGS += -I/usr/local/include
            LIBSODIUM_FOUND := 1
        endif
        ifeq ($(LIBSODIUM_FOUND),1)
            LIBS += -lsodium
        endif
    endif
endif

# Abort if libsodium is still missing.
ifeq ($(LIBSODIUM_FOUND),0)
    $(error Libsodium not found. Install libsodium development package.)
endif

# ==== Dependency: ICU ====
# ICU handles Unicode and internationalization.

ICU_FOUND := 0

# If ICUROOT is set, use it.
ifdef ICUROOT
    ifeq (,$(wildcard $(ICUROOT)/include/unicode/unistr.h))
        $(error ICUROOT is set but unicode/unistr.h not found in $(ICUROOT)/include)
    endif
    CXXFLAGS += -I$(ICUROOT)/include
    LDFLAGS  += -L$(ICUROOT)/lib
    LIBS     += -licuuc -licui18n -licudata
    ICU_FOUND := 1
else
    # Try pkg-config.
    ifdef PKG_CONFIG
        ICU_CFLAGS := $(shell $(PKG_CONFIG) --cflags icu-uc icu-i18n 2>/dev/null)
        ICU_LIBS   := $(shell $(PKG_CONFIG) --libs   icu-uc icu-i18n 2>/dev/null)
        ifneq ($(ICU_CFLAGS),)
            CXXFLAGS += $(ICU_CFLAGS)
            LIBS     += $(ICU_LIBS)
            ICU_FOUND := 1
        endif
    endif
endif

# Manual system path search (Unix only).
ifeq ($(ICU_FOUND),0)
    ifneq ($(UNAME_S), Windows)
        ifneq (,$(wildcard /usr/include/unicode/unistr.h))
            CXXFLAGS += -I/usr/include
            ICU_FOUND := 1
        else ifneq (,$(wildcard /usr/local/include/unicode/unistr.h))
            CXXFLAGS += -I/usr/local/include
            ICU_FOUND := 1
        endif
        ifeq ($(ICU_FOUND),1)
            LIBS += -licuuc -licui18n -licudata
        endif
    endif
endif

# Abort if ICU is still missing.
ifeq ($(ICU_FOUND),0)
    $(error ICU not found. Install ICU development package.)
endif

# ==== Compiler detection and base flags ====
# Determine which compiler is being used to apply appropriate flags.
COMPILER := $(notdir $(CXX))

ifeq ($(COMPILER),cl)
    # Microsoft Visual C++
    CXXFLAGS += /std:c++20 /W4 /utf-8
else ifneq (,$(findstring clang-cl,$(COMPILER)))
    # clang-cl (MSVC‑compatible frontend)
    CXXFLAGS += /std:c++20 /utf-8
else ifneq (,$(findstring g++,$(COMPILER)))
    # GCC (including MinGW)
    CXXFLAGS += -Wall -Wextra -pedantic -finput-charset=UTF-8 -fexec-charset=UTF-8
else ifneq (,$(findstring clang,$(COMPILER)))
    # Native Clang
    CXXFLAGS += -Wall -Wextra -pedantic -finput-charset=UTF-8 -fexec-charset=UTF-8
endif

# ==== Build‑type specific flags ====
# Add optimisation, debug symbols, and other flags based on BUILD.
ifeq ($(BUILD),debug)
    ifeq ($(COMPILER),cl)
        CXXFLAGS += /Od /Zi /MDd
        LDFLAGS  += /DEBUG
    else ifneq (,$(findstring clang-cl,$(COMPILER)))
        CXXFLAGS += /Od /Zi /MDd
        LDFLAGS  += /DEBUG
    else
        # Normalize source and build paths in debug info for reproducibility.
        SRC_ABS := $(abspath $(SRC_DIR))
        CUR_ABS := $(CURDIR)
        CXXFLAGS += -O0 -g \
            -fdebug-prefix-map=$(SRC_ABS)=. \
            -fdebug-prefix-map=$(SRC_DIR)=. \
            -fdebug-prefix-map=$(CUR_ABS)=. \
            -fno-ident \
            -gno-record-gcc-switches
    endif
else ifeq ($(BUILD),release)
    ifeq ($(COMPILER),cl)
        CXXFLAGS += /O2 /DNDEBUG /MD
        LDFLAGS  += /DEBUG:NONE
    else ifneq (,$(findstring clang-cl,$(COMPILER)))
        CXXFLAGS += /O2 /DNDEBUG /MD
        LDFLAGS  += /DEBUG:NONE
    else
        CXXFLAGS += -O2 -DNDEBUG -g0
        LDFLAGS  += -s
    endif
endif

# Add source and module directories to the include path.
CXXFLAGS += -I$(SRC_DIR) -I$(SRC_DIR)/modules

# ==== Embed metadata from metadata.hpp ====
# Extract project information from the header and pass them as macros.
VERSION_H = $(SRC_DIR)/modules/core/metadata.hpp

# Helper to escape double quotes inside a string (for safe macro expansion).
escape_quotes = $(subst ",\",$(1))

# Extract each macro's value using grep and sed.
PRODUCT_NAME     := $(shell grep -E "#define[[:space:]]+ProductName[[:space:]]+\".*\"" $(VERSION_H) | sed -E 's/#define[[:space:]]+ProductName[[:space:]]+\"([^\"]*)\".*/\1/')
FILE_DESCRIPTION := $(shell grep -E "#define[[:space:]]+FileDescription[[:space:]]+\".*\"" $(VERSION_H) | sed -E 's/#define[[:space:]]+FileDescription[[:space:]]+\"([^\"]*)\".*/\1/')
PRODUCT_VERSION  := $(shell grep -E "#define[[:space:]]+ProductVersion[[:space:]]+\".*\"" $(VERSION_H) | sed -E 's/#define[[:space:]]+ProductVersion[[:space:]]+\"([^\"]*)\".*/\1/')
LICENSE          := $(shell grep -E "#define[[:space:]]+License[[:space:]]+\".*\"" $(VERSION_H) | sed -E 's/#define[[:space:]]+License[[:space:]]+\"([^\"]*)\".*/\1/')
COPYRIGHT        := $(shell grep -E "#define[[:space:]]+Copyright[[:space:]]+\".*\"" $(VERSION_H) | sed -E 's/#define[[:space:]]+Copyright[[:space:]]+\"([^\"]*)\".*/\1/')
AUTHOR           := $(shell grep -E "#define[[:space:]]+Author[[:space:]]+\".*\"" $(VERSION_H) | sed -E 's/#define[[:space:]]+Author[[:space:]]+\"([^\"]*)\".*/\1/')
CONTACT          := $(shell grep -E "#define[[:space:]]+Contact[[:space:]]+\".*\"" $(VERSION_H) | sed -E 's/#define[[:space:]]+Contact[[:space:]]+\"([^\"]*)\".*/\1/')
HOMEPAGE         := $(shell grep -E "#define[[:space:]]+Homepage[[:space:]]+\".*\"" $(VERSION_H) | sed -E 's/#define[[:space:]]+Homepage[[:space:]]+\"([^\"]*)\".*/\1/')

# Add the extracted values as preprocessor definitions, properly escaped.
CXXFLAGS += \
    -DPRODUCT_NAME="\"$(call escape_quotes,$(PRODUCT_NAME))\"" \
    -DFILE_DESCRIPTION="\"$(call escape_quotes,$(FILE_DESCRIPTION))\"" \
    -DPRODUCT_VERSION="\"$(call escape_quotes,$(PRODUCT_VERSION))\"" \
    -DLICENSE="\"$(call escape_quotes,$(LICENSE))\"" \
    -DCOPYRIGHT="\"$(call escape_quotes,$(COPYRIGHT))\"" \
    -DAUTHOR="\"$(call escape_quotes,$(AUTHOR))\"" \
    -DCONTACT="\"$(call escape_quotes,$(CONTACT))\"" \
    -DHOMEPAGE="\"$(call escape_quotes,$(HOMEPAGE))\""

# ==== VPATH: source directories ====
# Tell make where to look for .cpp files (helps with pattern rules).
vpath %.cpp $(SRC_DIR)
vpath %.cpp $(SRC_DIR)/modules
vpath %.cpp $(SRC_DIR)/modules/app/commands
vpath %.cpp $(SRC_DIR)/modules/app/utils
vpath %.cpp $(SRC_DIR)/modules/core
vpath %.cpp $(SRC_DIR)/modules/crypto
vpath %.cpp $(SRC_DIR)/modules/interface
vpath %.cpp $(SRC_DIR)/modules/models
vpath %.cpp $(SRC_DIR)/modules/storage
vpath %.cpp $(SRC_DIR)/modules/ui
vpath %.cpp $(SRC_DIR)/modules/validation

# Ensure the object directory exists before any compilation.
$(shell mkdir -p $(OBJ_DIR))

# ==== Build rules ====
# Default target: build the executable.
all: $(TARGET)

# Link the executable from all object files.
$(TARGET): $(OBJECTS)
	$(CXX) $(LDFLAGS) -o $@ $^ $(LIBS)

# Compile a single C++ source file into an object file.
$(OBJ_DIR)/%.cpp.o: %.cpp
	@mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Remove all build artifacts.
clean:
	rm -rf $(OBJ_DIR) $(TARGET)

# Clean then rebuild.
rebuild: clean all

# Build and then run the executable.
run: all
	./$(TARGET)

# Installation prefix for Unix-like systems.
PREFIX = /usr/local

# Install the executable (Unix only; Windows users copy manually).
install: $(TARGET)
ifeq ($(UNAME_S), Windows)
	@echo "Install not supported on Windows. Please copy the executable manually."
else
	install -m 755 $(TARGET) $(PREFIX)/bin/$(TARGET)
	@echo "Installed to $(PREFIX)/bin/$(TARGET)"
endif

# Declare phony targets (not real files).
.PHONY: all clean rebuild run install