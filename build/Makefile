BUILD ?= release
ifeq ($(BUILD),debug)
    BUILD_MODE := Debug
else ifeq ($(BUILD),release)
    BUILD_MODE := Release
else
    $(error BUILD must be either 'debug' or 'release' (default: release))
endif

TARGET = passport-cli
SRC_DIR = ../source
OBJ_DIR = ./MakeFiles/$(TARGET).dir

SOURCES := $(shell LC_ALL=C find $(SRC_DIR) -name "*.cpp" | LC_ALL=C sort)
OBJECTS := $(addprefix $(OBJ_DIR)/,$(patsubst $(SRC_DIR)/%,%,$(SOURCES:.cpp=.cpp.o)))

ifeq ($(origin CXX),default)
    CXX := $(shell command -v g++ 2>/dev/null || command -v clang++ 2>/dev/null || echo "none")
    ifeq ($(CXX),none)
        $(error Neither g++ nor clang++ found. Please install a C++ compiler.)
    endif
endif

CXXFLAGS = -std=c++20
LDFLAGS =
LIBS =

PKG_CONFIG := $(shell command -v pkg-config 2>/dev/null)

LIBSODIUM_FOUND := 0
ifdef SODIUMROOT
    ifeq (,$(wildcard $(SODIUMROOT)/include/sodium.h))
        $(error SODIUMROOT is set but sodium.h not found in $(SODIUMROOT)/include)
    endif
    CXXFLAGS += -I$(SODIUMROOT)/include
    LDFLAGS  += -L$(SODIUMROOT)/lib
    LIBS     += -lsodium
    LIBSODIUM_FOUND := 1
else
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
ifeq ($(LIBSODIUM_FOUND),0)
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
ifeq ($(LIBSODIUM_FOUND),0)
    $(error Libsodium not found. Install libsodium development package.)
endif

ICU_FOUND := 0
ifndef ICU_LIBS_NAMES
    ifdef PKG_CONFIG
        ICU_LIBS := $(shell $(PKG_CONFIG) --libs icu-uc icu-i18n 2>/dev/null)
        ifneq ($(ICU_LIBS),)
            ICU_LIBS_NAMES := $(ICU_LIBS)
        endif
    endif
    ifndef ICU_LIBS_NAMES
        ifneq (,$(wildcard /usr/lib/libicui18n.so /usr/local/lib/libicui18n.so /usr/lib/libicui18n.a /usr/local/lib/libicui18n.a))
            ICU_LIBS_NAMES := -licuuc -licui18n -licudata
        else ifneq (,$(wildcard /usr/lib/libicuin.a /usr/local/lib/libicuin.a /usr/lib/libicuin.so /usr/local/lib/libicuin.so))
            ICU_LIBS_NAMES := -licuuc -licuin -licudt
        else
            ICU_LIBS_NAMES := -licuuc -licui18n -licudata
        endif
    endif
endif
ifdef ICUROOT
    ifeq (,$(wildcard $(ICUROOT)/include/unicode/unistr.h))
        $(error ICUROOT is set but unicode/unistr.h not found in $(ICUROOT)/include)
    endif
    CXXFLAGS += -I$(ICUROOT)/include
    LDFLAGS  += -L$(ICUROOT)/lib
    LIBS     += $(ICU_LIBS_NAMES)
    ICU_FOUND := 1
else
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
ifeq ($(ICU_FOUND),0)
    ifneq (,$(wildcard /usr/include/unicode/unistr.h))
        CXXFLAGS += -I/usr/include
        ICU_FOUND := 1
    else ifneq (,$(wildcard /usr/local/include/unicode/unistr.h))
        CXXFLAGS += -I/usr/local/include
        ICU_FOUND := 1
    endif
    ifeq ($(ICU_FOUND),1)
        LIBS += $(ICU_LIBS_NAMES)
    endif
endif
ifeq ($(ICU_FOUND),0)
    $(error ICU not found. Install ICU development package or set ICUROOT and ICU_LIBS_NAMES)
endif

CXXFLAGS += -Wall -Wextra -pedantic -finput-charset=UTF-8 -fexec-charset=UTF-8

ifeq ($(BUILD),debug)
    SRC_ABS := $(abspath $(SRC_DIR))
    CUR_ABS := $(CURDIR)
    CXXFLAGS += -O0 -g \
        -fdebug-prefix-map=$(SRC_ABS)=. \
        -fdebug-prefix-map=$(SRC_DIR)=. \
        -fdebug-prefix-map=$(CUR_ABS)=. \
        -fno-ident \
        -gno-record-gcc-switches
else ifeq ($(BUILD),release)
    CXXFLAGS += -O2 -DNDEBUG -g0
    LDFLAGS  += -s -Wl,--build-id=none
endif

CXXFLAGS += -I$(SRC_DIR) -I$(SRC_DIR)/modules

VERSION_H = $(SRC_DIR)/modules/core/metadata.hpp
escape_quotes = $(subst ",\",$(1))

PRODUCT_NAME     := $(shell grep -E "#define[[:space:]]+ProductName[[:space:]]+\".*\"" $(VERSION_H) | sed -E 's/#define[[:space:]]+ProductName[[:space:]]+\"([^\"]*)\".*/\1/')
FILE_DESCRIPTION := $(shell grep -E "#define[[:space:]]+FileDescription[[:space:]]+\".*\"" $(VERSION_H) | sed -E 's/#define[[:space:]]+FileDescription[[:space:]]+\"([^\"]*)\".*/\1/')
PRODUCT_VERSION  := $(shell grep -E "#define[[:space:]]+ProductVersion[[:space:]]+\".*\"" $(VERSION_H) | sed -E 's/#define[[:space:]]+ProductVersion[[:space:]]+\"([^\"]*)\".*/\1/')
LICENSE          := $(shell grep -E "#define[[:space:]]+License[[:space:]]+\".*\"" $(VERSION_H) | sed -E 's/#define[[:space:]]+License[[:space:]]+\"([^\"]*)\".*/\1/')
COPYRIGHT        := $(shell grep -E "#define[[:space:]]+Copyright[[:space:]]+\".*\"" $(VERSION_H) | sed -E 's/#define[[:space:]]+Copyright[[:space:]]+\"([^\"]*)\".*/\1/')
AUTHOR           := $(shell grep -E "#define[[:space:]]+Author[[:space:]]+\".*\"" $(VERSION_H) | sed -E 's/#define[[:space:]]+Author[[:space:]]+\"([^\"]*)\".*/\1/')
CONTACT          := $(shell grep -E "#define[[:space:]]+Contact[[:space:]]+\".*\"" $(VERSION_H) | sed -E 's/#define[[:space:]]+Contact[[:space:]]+\"([^\"]*)\".*/\1/')
HOMEPAGE         := $(shell grep -E "#define[[:space:]]+Homepage[[:space:]]+\".*\"" $(VERSION_H) | sed -E 's/#define[[:space:]]+Homepage[[:space:]]+\"([^\"]*)\".*/\1/')

CXXFLAGS += \
    -DPRODUCT_NAME="\"$(call escape_quotes,$(PRODUCT_NAME))\"" \
    -DFILE_DESCRIPTION="\"$(call escape_quotes,$(FILE_DESCRIPTION))\"" \
    -DPRODUCT_VERSION="\"$(call escape_quotes,$(PRODUCT_VERSION))\"" \
    -DLICENSE="\"$(call escape_quotes,$(LICENSE))\"" \
    -DCOPYRIGHT="\"$(call escape_quotes,$(COPYRIGHT))\"" \
    -DAUTHOR="\"$(call escape_quotes,$(AUTHOR))\"" \
    -DCONTACT="\"$(call escape_quotes,$(CONTACT))\"" \
    -DHOMEPAGE="\"$(call escape_quotes,$(HOMEPAGE))\""

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

$(shell mkdir -p $(OBJ_DIR))

all: $(TARGET)

$(TARGET): $(OBJECTS)
	$(CXX) -o $@ $^ $(LIBS) $(LDFLAGS)

$(OBJ_DIR)/%.cpp.o: %.cpp
	@mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	rm -rf $(OBJ_DIR) $(TARGET)

rebuild: clean all

run: all
	./$(TARGET)

.PHONY: all clean rebuild run install