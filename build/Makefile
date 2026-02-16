# =============================================================================
#   Pure Makefile for passport-cli
#   This file does not depend on CMake; it invokes the compiler directly.
#   It is designed to work on Linux, macOS, and Windows (with MinGW/MSYS).
# =============================================================================

# ---- Target executable name ----
TARGET = passport-cli

# ---- Directories ----
# Location of source files (relative to this Makefile)
SRC_DIR = ../source
# Object files will be placed in the current directory
OBJ_DIR = .

# ---- Find all C++ source files ----
# Recursively locate every .cpp file inside SRC_DIR.
# On Unix-like systems, 'find' works. On Windows, you may need to install
# a Unix-like environment (MSYS, Git Bash) or manually list the files.
SOURCES = $(shell find $(SRC_DIR) -name "*.cpp")

# ---- Generate object file names ----
# This simple approach strips the directory part, so all object files
# land directly in OBJ_DIR. **Warning**: if two source files in different
# subdirectories have the same name, one will overwrite the other.
# For a production build, you would preserve the directory structure.
OBJECTS = $(notdir $(SOURCES:.cpp=.o))

# ---- Compiler and base flags ----
CXX = g++                           # The C++ compiler
CXXFLAGS = -std=c++20 -Wall -Wextra -pedantic   # C++20 standard and warnings
CXXFLAGS += -I$(SRC_DIR) -I$(SRC_DIR)/modules   # Include paths for headers
LDFLAGS =                            # Linker flags (if any)
LIBS =                               # Libraries to link

# ---- Detect operating system and adjust for libsodium ----
UNAME_S := $(shell uname -s 2>/dev/null || echo Windows)
ifeq ($(UNAME_S), Windows)
    # On Windows, assume libsodium is installed in a standard location
    # or that the linker can find it with -lsodium. You may need to adjust
    # CXXFLAGS and LDFLAGS manually if libsodium is in a custom path.
    LIBS += -lsodium
else
    # On Linux/macOS, use pkg-config to obtain the correct compiler and linker flags.
    PKG_CONFIG := pkg-config
    SODIUM_CFLAGS := $(shell $(PKG_CONFIG) --cflags libsodium 2>/dev/null)
    SODIUM_LIBS   := $(shell $(PKG_CONFIG) --libs   libsodium 2>/dev/null)
    ifneq ($(SODIUM_CFLAGS),)
        CXXFLAGS += $(SODIUM_CFLAGS)
        LIBS     += $(SODIUM_LIBS)
    else
        # If pkg-config fails, fall back to a simple -lsodium (may work if libsodium
        # is installed in a standard system path).
        LIBS += -lsodium
    endif
endif

# ---- Extract metadata from metadata.hpp ----
# The version header contains product information in #define macros.
VERSION_H = $(SRC_DIR)/core/metadata.hpp

# Helper shell function to extract a quoted macro value.
# Example: #define ProductName "passport-cli" -> passport-cli
define get_version_macro
$(shell grep -E "#define[[:space:]]+$(1)[[:space:]]+\".*\"" $(VERSION_H) | sed -E 's/#define[[:space:]]+$(1)[[:space:]]+\"([^\"]*)\".*/\1/')
endef

# Extract all metadata fields
PRODUCT_NAME     := $(call get_version_macro,ProductName)
FILE_DESCRIPTION := $(call get_version_macro,FileDescription)
PRODUCT_VERSION  := $(call get_version_macro,ProductVersion)
LICENSE          := $(call get_version_macro,License)
COPYRIGHT        := $(call get_version_macro,Copyright)
AUTHOR           := $(call get_version_macro,Author)
CONTACT          := $(call get_version_macro,Contact)
HOMEPAGE         := $(call get_version_macro,Homepage)

# Add metadata as preprocessor definitions (-D) so they can be used in code.
CXXFLAGS += \
    -DPRODUCT_NAME=\"$(PRODUCT_NAME)\" \
    -DFILE_DESCRIPTION=\"$(FILE_DESCRIPTION)\" \
    -DPRODUCT_VERSION=\"$(PRODUCT_VERSION)\" \
    -DLICENSE=\"$(LICENSE)\" \
    -DCOPYRIGHT=\"$(COPYRIGHT)\" \
    -DLEGAL_COPYRIGHT=\"$(LEGAL_COPYRIGHT)\" \
    -DCONTACT=\"$(CONTACT)\" \
    -DHOMEPAGE=\"$(HOMEPAGE)\"

# ---- VPATH: tell make where to look for source files ----
# Since source files are scattered in subdirectories, we list all possible
# paths so that the %.o rule can find the corresponding .cpp.
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
# (Add more subdirectories here as needed)

# ===== Build rules =====

# Default target: build the executable
all: $(TARGET)

# Link object files into the final executable
$(TARGET): $(OBJECTS)
	$(CXX) $(LDFLAGS) -o $@ $^ $(LIBS)

# Compile any .cpp file into a .o file.
# The VPATH setting ensures that make can find the source file even if it is
# in a subdirectory.
%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Clean up all generated files
clean:
	rm -f $(OBJECTS) $(TARGET)
	# On Windows, you might use: del /Q $(subst /,\,$(OBJECTS)) $(TARGET) 2>nul

# Rebuild everything from scratch
rebuild: clean all

# Run the program after building
run: all
	./$(TARGET)

# Install the executable to a system directory (Unix-like only)
PREFIX = /usr/local
install: $(TARGET)
ifeq ($(UNAME_S), Windows)
	@echo "Install not supported on Windows. Please copy the executable manually."
else
	install -m 755 $(TARGET) $(PREFIX)/bin/$(TARGET)
	@echo "Installed to $(PREFIX)/bin/$(TARGET)"
endif

# Mark these targets as "phony" (they do not represent actual files)
.PHONY: all clean rebuild run install