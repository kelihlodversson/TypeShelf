# Type Shelf -- the design is in docs/superpowers/specs/
#
#   make test            host unit tests
#   make                 build/m68k/TYPESHLF.PRG
#   make check-arm       compile the portable sources for ARM (no link)
#   make check-x86_64    compile them for x32 (no link)
#   make clean
#
# Needs GNU make, and for m68k the m68k-atari-mintelf- toolchain on PATH
# (docs/toolchains.md).  On macOS use gmake.

GEMLIB   = third_party/gemlib
LIBCMINI = third_party/libcmini

LOGIC_SRC =
LOGIC_SRC += src/fontdb/split.c
LOGIC_SRC += src/fontdb/group.c
GEM_SRC   = src/main.c
M68K_PLATFORM_SRC =
GENERIC_PLATFORM_SRC =

STD      = -std=gnu99
WARN     = -Wall -Wextra
INCLUDES = -Isrc -I$(GEMLIB) -I$(LIBCMINI)/include

.PHONY: all test check-arm check-x86_64 clean
all: build/m68k/TYPESHLF.PRG

# ---- host unit tests ---------------------------------------------------

TEST_SRC   =
TEST_SRC  += tests/main.c
TEST_SRC  += tests/test_split.c
TEST_SRC  += tests/test_group.c
CC_HOST    ?= cc
HOST_FLAGS ?= $(STD) $(WARN) -Werror -g -Isrc -Itests

build/host/tests: $(LOGIC_SRC) $(TEST_SRC) $(wildcard src/*/*.h tests/*.h)
	mkdir -p build/host
	$(CC_HOST) $(HOST_FLAGS) $(LOGIC_SRC) $(TEST_SRC) -o $@

test: build/host/tests
	./build/host/tests

# ---- the libraries -----------------------------------------------------

M68K_PREFIX ?= m68k-atari-mintelf-
M68K_CC      = $(M68K_PREFIX)gcc

GEMLIB_H     = $(GEMLIB)/mt_gem.h
GEMLIB_A     = $(GEMLIB)/.lib68000/libgem.a
LIBCMINI_A   = $(LIBCMINI)/build/libcmini.a
LIBCMINI_CRT = $(LIBCMINI)/build/objs/crt0.o

# gemlib's headers are generated from mt_gem.h.in
$(GEMLIB_H):
	$(MAKE) -C $(GEMLIB) mt_gem.h

$(GEMLIB_A): $(GEMLIB_H)
	$(MAKE) -C $(GEMLIB) CROSS_TOOL=$(patsubst %-,%,$(M68K_PREFIX)) .lib68000/libgem.a

$(LIBCMINI_A):
	$(MAKE) -C $(LIBCMINI) mintelf_defconfig
	$(MAKE) -C $(LIBCMINI) ONLY_68K=y dirs libs startups

$(LIBCMINI_CRT): $(LIBCMINI_A)

# ---- m68k --------------------------------------------------------------

M68K_CFLAGS = $(STD) $(WARN) -O2 -m68000 -fomit-frame-pointer $(INCLUDES)
M68K_SRC    = $(LOGIC_SRC) $(GEM_SRC) $(M68K_PLATFORM_SRC)
M68K_OBJ    = $(patsubst src/%.c,build/m68k/%.o,$(M68K_SRC))

build/m68k/%.o: src/%.c $(wildcard src/*/*.h) $(GEMLIB_H)
	@mkdir -p $(dir $@)
	$(M68K_CC) $(M68K_CFLAGS) -c $< -o $@

build/m68k/TYPESHLF.PRG: $(M68K_OBJ) $(GEMLIB_A) $(LIBCMINI_A) $(LIBCMINI_CRT)
	$(M68K_CC) -m68000 -nostdlib $(LIBCMINI_CRT) $(M68K_OBJ) \
	    -L$(GEMLIB)/.lib68000 -L$(LIBCMINI)/build \
	    -lcmini -lgem -lgcc -lcmini -o $@

# ---- ARM and x86: compile only, until gemlib has their trap layers -----

CHECK_SRC    = $(LOGIC_SRC) $(GEM_SRC) $(GENERIC_PLATFORM_SRC)

ARM_CC       ?= arm-none-eabi-gcc
ARM_FLAGS    ?=
X86_CC       ?= gcc
X86_FLAGS    ?= -mx32
CHECK_CFLAGS = $(STD) $(WARN) -Werror -O2 -ffreestanding -Isrc/compat $(INCLUDES)

check-arm: $(GEMLIB_H)
	for f in $(CHECK_SRC); do \
	    $(ARM_CC) $(ARM_FLAGS) $(CHECK_CFLAGS) -c $$f -o /dev/null || exit 1; \
	done

check-x86_64: $(GEMLIB_H)
	for f in $(CHECK_SRC); do \
	    $(X86_CC) $(X86_FLAGS) $(CHECK_CFLAGS) -c $$f -o /dev/null || exit 1; \
	done

clean:
	rm -rf build
