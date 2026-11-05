# Toolchains

Type Shelf needs a cross toolchain per target. The pTOS repository has the
full, tested instructions, including installing without sudo:
<https://github.com/kelihlodversson/pTOS/blob/master/.claude/skills/toolchain-install/SKILL.md>

## m68k (the real target)

The `m68k-atari-mintelf-` toolchain. It links a hybrid executable: a GEMDOS PRG
that is also an ELF, so the same `TYPESHLF.PRG` loads on real TOS, EmuTOS and
pTOS.

Ubuntu (this is what CI does):

    sudo add-apt-repository -y ppa:vriviere/mintelf
    sudo apt-get update
    sudo apt-get install -y cross-mintelf-essential

macOS: Thorsten Otto's archives from <https://tho-otto.m68k.eu/crossmint.php>
(`binutils-*-mintelf-*-bin-macos`, `gcc-*-mintelf-*-bin-macos` and
`mintlib-*-mintelf-*-dev`), extracted into `/opt/cross-mint`, then:

    export PATH=/opt/cross-mint/bin:$PATH

libcmini's configuration also needs Python 3 and kconfiglib:

    pip3 install kconfiglib

## ARM (runs on pTOS ARM)

    sudo apt-get install -y gcc-arm-none-eabi       # Ubuntu
    brew install gcc-arm-embedded                   # macOS

`make arm` builds libcmini and gemlib for ARM from the submodules and links
`build/arm/TYPESHLF.PRG`. It needs kconfiglib, as the m68k build does.

## x32 (pTOS)

    sudo apt-get install -y gcc gcc-multilib libc6-dev-x32     # Ubuntu
    brew install x86_64-elf-gcc x86_64-elf-binutils    # macOS

The compile-only `check-x86_64` target uses host `gcc` on x86_64 Linux and
`x86_64-elf-gcc` elsewhere; override that compiler with `X86_CC`.

`make x32 PTOS=../pTOS` links `build/x32/TYPESHLF.PRG` and generates its
`TYPESHLF.RSC`. `PTOS` must point at a built pTOS source tree with
`CONF_WITH_ELF_SHARED=y`, `CONF_WITH_LIBAES=y` and the x32 VDI enabled.
CI pins pTOS master after the #681 libaes cutover to `9193f7c7b3773b5cd418144fc8dee6e64c5f1473` and builds it with:

    make pc-x86_64-libaes-desktop_defconfig
    make CROSS_COMPILE= -j4

This configuration uses the external shared libaes desktop with kernel AES
disabled (`CONF_WITH_AES=n`). CI checks that the desktop links libaes and
that the kernel contains no retired AES service entry points.

The TypeShelf build uses native GCC 12 or newer and `objcopy` by default; override
`X32_CC` and `X32_OBJCOPY` for a cross toolchain. It builds a separate x32
copy of libcmini, so m68k, ARM and x32 builds can run in parallel.

Install the matching pTOS runtime (`LD.SO`, `libptos.so`, `libaes.so`) in
`\GEMSYS\LIB\X32` on the guest. Keep `TYPESHLF.PRG` and `TYPESHLF.RSC`
together. This is a pTOS executable, not a Linux x32 application.

The x32 target is experimental: the pinned pTOS VDI client does not yet
implement the font-name and metric queries TypeShelf needs, or font loading
and scalable-font inquiries. Browsing and installing fonts therefore require
further pTOS VDI support. Rejected queries return deterministic empty results.
CI checks linking and ELF dependencies; it does not test interactive operation.

`make check-x86_64` remains available as a compile-only portability check.
