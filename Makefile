# Type Shelf -- the design is in docs/superpowers/specs/
#
#   make test            host unit tests
#   make                 build/m68k/TYPESHLF.PRG and TYPESHLF.RSC (keep them together)
#   make arm             build/arm/TYPESHLF.PRG and TYPESHLF.RSC (links; not yet run on pTOS)
#   make check-x86_64    compile the sources for x32 (no link)
#   make clean
#
# Needs GNU make, and for m68k the m68k-atari-mintelf- toolchain on PATH
# (docs/toolchains.md).  On macOS use gmake.

# The version shown in About: "dev" for a build of your own; a release passes
# make VERSION=1.2.3 (.github/workflows/release.yml)
VERSION ?= dev

GEMLIB   = third_party/gemlib
LIBCMINI = third_party/libcmini

LOGIC_SRC =
LOGIC_SRC += src/fontdb/split.c
LOGIC_SRC += src/fontdb/group.c
LOGIC_SRC += src/gdosinfo/decode.c src/gdosinfo/about.c
LOGIC_SRC += src/layout/layout.c
LOGIC_SRC += src/install/install.c
LOGIC_SRC += src/fontdb/build.c
LOGIC_SRC += src/ui/rect.c
LOGIC_SRC += src/ui/command.c
LOGIC_SRC += src/ui/winstate.c
LOGIC_SRC += src/ui/toolbar_layout.c
LOGIC_SRC += src/ui/viewmodel.c
GEM_SRC   = src/main.c
GEM_SRC  += src/vdi_if/vdi_if_gem.c
GEM_SRC  += src/ui/icons.c
GEM_SRC  += src/ui/draw.c src/ui/popup.c
GEM_SRC  += src/ui/views.c
GEM_SRC  += src/ui/menu.c
GEM_SRC  += src/ui/window.c
GEM_SRC  += src/ui/app.c src/ui/about.c src/ui/install.c
GEM_SRC  += src/version.c
M68K_PLATFORM_SRC = src/platform/platform.c src/platform/files.c src/platform/m68k/legacy.c
GENERIC_PLATFORM_SRC = src/platform/platform.c src/platform/files.c src/platform/generic/legacy.c

STD      = -std=gnu99
WARN     = -Wall -Wextra
INCLUDES = -Isrc -Ibuild -I$(GEMLIB) -I$(LIBCMINI)/include

# build/version_def.h has the version; it keeps its timestamp while the version
# does not change, so only a new version recompiles
VERSION_H = build/version_def.h

$(VERSION_H): FORCE
	@mkdir -p $(dir $@)
	@printf '#define TYPESHELF_VERSION "%s"\n' '$(VERSION)' > $@.new
	@cmp -s $@.new $@ && rm $@.new || mv $@.new $@

.DEFAULT_GOAL := all
.PHONY: all test arm check-x86_64 clean
all: build/m68k/TYPESHLF.PRG build/m68k/TYPESHLF.RSC

# ---- host unit tests ---------------------------------------------------

TEST_SRC   =
TEST_SRC  += tests/main.c
TEST_SRC  += tests/test_split.c
TEST_SRC  += tests/test_group.c
TEST_SRC  += tests/test_decode.c tests/test_about.c
TEST_SRC  += tests/test_layout.c
TEST_SRC  += tests/test_build.c
TEST_SRC  += tests/test_ui.c tests/test_install.c
CC_HOST    ?= cc
HOST_FLAGS ?= $(STD) $(WARN) -Werror -g -Isrc -Ibuild -Itests

build/host/tests: $(LOGIC_SRC) $(TEST_SRC) $(wildcard src/*/*.h tests/*.h) $(VERSION_H)
	mkdir -p build/host
	$(CC_HOST) $(HOST_FLAGS) $(LOGIC_SRC) $(TEST_SRC) -o $@

# the resource file with the toolbar's icons, which sits next to the program
ICON_SRC = $(wildcard res/icons/*.txt)
PYTHON  ?= python3

build/%/TYPESHLF.RSC: tools/mkrsc.py $(ICON_SRC)
	@mkdir -p $(dir $@)
	$(PYTHON) tools/mkrsc.py $@

test: build/host/tests build/host/TYPESHLF.RSC
	./build/host/tests
	$(PYTHON) tools/mkrsc.py --check build/host/TYPESHLF.RSC

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

build/m68k/%.o: src/%.c $(wildcard src/*/*.h) $(GEMLIB_H) $(VERSION_H)
	@mkdir -p $(dir $@)
	$(M68K_CC) $(M68K_CFLAGS) -c $< -o $@

build/m68k/TYPESHLF.PRG: $(M68K_OBJ) $(GEMLIB_A) $(LIBCMINI_A) $(LIBCMINI_CRT)
	$(M68K_CC) -m68000 -nostdlib $(LIBCMINI_CRT) $(M68K_OBJ) \
	    -L$(GEMLIB)/.lib68000 -Lbuild/m68k/lib \
	    -lcmini -lgem -lgcc -lcmini -o $@

# ---- ARM ---------------------------------------------------------------

ARM_PREFIX   ?= arm-none-eabi-
ARM_CC        = $(ARM_PREFIX)gcc
# the same CPU flags pTOS builds its own ARM programs with
ARM_ABI       = -march=armv7-a -mtune=cortex-a7 -marm -mfpu=neon-vfpv4 -mfloat-abi=hard \
    -mword-relocations
ARM_LIBGEM    = build/arm/lib/libgem.a
ARM_LIBCMINI  = build/arm/lib/libcmini.a
ARM_CRT       = build/arm/lib/minicrt0.o

# gemlib's own include/arm directory has the compiler.h its headers need
ARM_CFLAGS = $(STD) $(WARN) -Werror -O2 -ffreestanding $(ARM_ABI) \
    -Isrc -Ibuild -I$(GEMLIB)/include/arm -I$(GEMLIB) -I$(LIBCMINI)/include
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
	cp build/arm/libcmini/build/libcmini.a build/arm/libcmini/build/objs/minicrt0.o $(dir $@)

$(ARM_CRT): $(ARM_LIBCMINI)

build/arm/%.o: src/%.c $(wildcard src/*/*.h) $(GEMLIB_H) $(VERSION_H)
	@mkdir -p $(dir $@)
	$(ARM_CC) $(ARM_CFLAGS) -c $< -o $@

# -q keeps the relocations in the file: pTOS loads a program wherever there is
# room and applies them (doc/elfload.txt in pTOS), so it must not be stripped
build/arm/TYPESHLF.PRG: $(ARM_OBJ) $(ARM_LIBGEM) $(ARM_LIBCMINI) $(ARM_CRT)
	$(ARM_CC) $(ARM_ABI) -nostdlib -Wl,-q -Wl,-Ttext=0 -Wl,-e_start $(ARM_CRT) $(ARM_OBJ) \
	    -Lbuild/arm/lib -lcmini -lgem -lgcc -lcmini -o $@

arm: build/arm/TYPESHLF.PRG build/arm/TYPESHLF.RSC

# ---- x86: compile only, until gemlib has a trap layer for it -----------

CHECK_SRC    = $(LOGIC_SRC) $(GEM_SRC) $(GENERIC_PLATFORM_SRC)

# the host's gcc only on x86_64 Linux (it needs -mx32 multilib support); anywhere
# else the bare-metal cross compiler
ifeq ($(shell uname -s -m),Linux x86_64)
X86_CC       ?= gcc
else
X86_CC       ?= x86_64-elf-gcc
endif
X86_FLAGS    ?= -mx32
CHECK_CFLAGS = $(STD) $(WARN) -Werror -O2 -ffreestanding -Isrc/compat $(INCLUDES)

check-x86_64: $(GEMLIB_H) $(VERSION_H)
	for f in $(CHECK_SRC); do \
	    $(X86_CC) $(X86_FLAGS) $(CHECK_CFLAGS) -c $$f -o /dev/null || exit 1; \
	done

clean:
	rm -rf build
