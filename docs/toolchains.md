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

## ARM (builds and links; not yet run on pTOS)

    sudo apt-get install -y gcc-arm-none-eabi       # Ubuntu
    brew install gcc-arm-embedded                   # macOS

`make arm` builds libcmini and gemlib for ARM from the submodules and links
`build/arm/TYPESHLF.PRG`. It needs kconfiglib, as the m68k build does.

## x86_64 (compile check, x32)

    sudo apt-get install -y gcc gcc-multilib libc6-dev-x32     # Ubuntu
    brew install x86_64-elf-gcc x86_64-elf-binutils                             # macOS

On macOS run `gmake check-x86_64 X86_CC=x86_64-elf-gcc`; the system `gcc` is clang and cannot compile libcmini's headers.
