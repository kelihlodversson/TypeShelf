# Type Shelf -- the design is in docs/superpowers/specs/
#
#   make test            host unit tests
#   make                 build/m68k/TYPESHLF.PRG
#   make arm             build/arm/TYPESHLF.elf   (links; not yet run on pTOS)
#   make check-x86_64    compile the sources for x32 (no link)
#   make clean
#
# Needs GNU make, and for m68k the m68k-atari-mintelf- toolchain on PATH
# (docs/toolchains.md).  On macOS use gmake.

GEMLIB   = third_party/gemlib
LIBCMINI = third_party/libcmini

LOGIC_SRC =
LOGIC_SRC += src/fontdb/split.c
LOGIC_SRC += src/fontdb/group.c
LOGIC_SRC += src/gdosinfo/decode.c
LOGIC_SRC += src/layout/layout.c
LOGIC_SRC += src/fontdb/build.c
GEM_SRC   = src/main.c
GEM_SRC  += src/vdi_if/vdi_if_gem.c
M68K_PLATFORM_SRC = src/platform/m68k/platform.c
GENERIC_PLATFORM_SRC = src/platform/generic/platform.c

STD      = -std=gnu99
WARN     = -Wall -Wextra
INCLUDES = -Isrc -I$(GEMLIB) -I$(LIBCMINI)/include

.PHONY: all test arm check-x86_64 clean
all: build/m68k/TYPESHLF.PRG

# ---- host unit tests ---------------------------------------------------

TEST_SRC   =
TEST_SRC  += tests/main.c
TEST_SRC  += tests/test_split.c
TEST_SRC  += tests/test_group.c
TEST_SRC  += tests/test_decode.c
TEST_SRC  += tests/test_layout.c
TEST_SRC  += tests/test_build.c
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
LIBCMINI_A   = build/m68k/lib/libcmini.a
LIBCMINI_CRT = build/m68k/lib/crt0.o

# gemlib's headers are generated from mt_gem.h.in
$(GEMLIB_H):
	$(MAKE) -C $(GEMLIB) mt_gem.h

# gemlib does its own dependency checking, so ask it on every build; it leaves
# the archive alone (and so does not relink) when nothing changed.
$(GEMLIB_A): $(GEMLIB_H) FORCE
	$(MAKE) -C $(GEMLIB) CROSS_TOOL=$(patsubst %-,%,$(M68K_PREFIX)) .lib68000/libgem.a

# libcmini keeps one build/ directory and one .config for every target, so each
# target builds in its own copy of it (a parallel `make all arm` cannot make the
# two clash) and keeps the two files it needs from there.
LIBCMINI_COPY = rsync -a --exclude=.git --exclude=/build --exclude=/obj \
    --exclude=/.config --exclude=/.config.old $(LIBCMINI)/

# libcmini is copied, so a stamp fingerprinting the submodule's files (contents,
# not just names, so editing a modified file again counts) tells make when the
# copies are out of date.  The stamp keeps its timestamp while nothing changes.
LIBCMINI_REV = build/libcmini.rev

$(LIBCMINI_REV): FORCE
	@mkdir -p $(dir $@)
	@(cd $(LIBCMINI) && git rev-parse HEAD && \
	    git ls-files -co --exclude-standard | LC_ALL=C sort | \
	    while read -r f; do cksum "$$f" 2>/dev/null; done) | cksum > $@.new
	@cmp -s $@.new $@ && rm $@.new || mv $@.new $@

FORCE:
.PHONY: FORCE

$(LIBCMINI_A): $(LIBCMINI_REV)
	rm -rf build/m68k/libcmini
	mkdir -p build/m68k/libcmini $(dir $@)
	$(LIBCMINI_COPY) build/m68k/libcmini/
	$(MAKE) -C build/m68k/libcmini mintelf_defconfig
	$(MAKE) -C build/m68k/libcmini ONLY_68K=y dirs libs startups
	cp build/m68k/libcmini/build/libcmini.a build/m68k/libcmini/build/objs/crt0.o $(dir $@)

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
	    -L$(GEMLIB)/.lib68000 -Lbuild/m68k/lib \
	    -lcmini -lgem -lgcc -lcmini -o $@

# ---- ARM ---------------------------------------------------------------

ARM_PREFIX   ?= arm-none-eabi-
ARM_CC        = $(ARM_PREFIX)gcc
ARM_ABI       = -mfloat-abi=hard -mfpu=vfp -mword-relocations
ARM_LIBGEM    = build/arm/lib/libgem.a
ARM_LIBCMINI  = build/arm/lib/libcmini.a
ARM_CRT       = build/arm/lib/crt0.o

# gemlib's own include/arm directory has the compiler.h its headers need
ARM_CFLAGS = $(STD) $(WARN) -Werror -O2 -ffreestanding $(ARM_ABI) \
    -Isrc -I$(GEMLIB)/include/arm -I$(GEMLIB) -I$(LIBCMINI)/include
ARM_SRC    = $(LOGIC_SRC) $(GEM_SRC) $(GENERIC_PLATFORM_SRC)
ARM_OBJ    = $(patsubst src/%.c,build/arm/%.o,$(ARM_SRC))

$(ARM_LIBGEM): $(GEMLIB_H) FORCE
	$(MAKE) -C $(GEMLIB) ARCH=arm .libarm/libgem.a
	@mkdir -p $(dir $@)
	@cmp -s $(GEMLIB)/.libarm/libgem.a $@ || cp $(GEMLIB)/.libarm/libgem.a $@

$(ARM_LIBCMINI): $(LIBCMINI_REV)
	rm -rf build/arm/libcmini
	mkdir -p build/arm/libcmini $(dir $@)
	$(LIBCMINI_COPY) build/arm/libcmini/
	$(MAKE) -C build/arm/libcmini ptos_arm_defconfig
	$(MAKE) -C build/arm/libcmini dirs libs startups
	cp build/arm/libcmini/build/libcmini.a build/arm/libcmini/build/objs/crt0.o $(dir $@)

$(ARM_CRT): $(ARM_LIBCMINI)

build/arm/%.o: src/%.c $(wildcard src/*/*.h) $(GEMLIB_H)
	@mkdir -p $(dir $@)
	$(ARM_CC) $(ARM_CFLAGS) -c $< -o $@

build/arm/TYPESHLF.elf: $(ARM_OBJ) $(ARM_LIBGEM) $(ARM_LIBCMINI) $(ARM_CRT)
	$(ARM_CC) $(ARM_ABI) -nostdlib $(ARM_CRT) $(ARM_OBJ) \
	    -Lbuild/arm/lib -lcmini -lgem -lgcc -lcmini -o $@

arm: build/arm/TYPESHLF.elf

# ---- x86: compile only, until gemlib has a trap layer for it -----------

CHECK_SRC    = $(LOGIC_SRC) $(GEM_SRC) $(GENERIC_PLATFORM_SRC)

X86_CC       ?= gcc
X86_FLAGS    ?= -mx32
CHECK_CFLAGS = $(STD) $(WARN) -Werror -O2 -ffreestanding -Isrc/compat $(INCLUDES)

check-x86_64: $(GEMLIB_H)
	for f in $(CHECK_SRC); do \
	    $(X86_CC) $(X86_FLAGS) $(CHECK_CFLAGS) -c $$f -o /dev/null || exit 1; \
	done

clean:
	rm -rf build
