# Toolchains

## CI: pinned official pTOS SDKs

TypeShelf CI uses the official **0.2.0** pTOS SDK packages on Ubuntu 24.04,
installed by [the local setup action](../.github/actions/setup-ptos/action.yml).
The build and release workflows share that action, so compiler versions, Python
setup and ABI choices do not drift. Each cross-build installs only its SDK;
host unit tests continue to use the system compiler.

The action uses `https://apt.ptos.dev` with a dedicated `signed-by` keyring and
an exact package version. Since Pages retains a bounded set of recent versions,
a pin that has left APT is downloaded from the permanent GitHub Release archive,
verified against its SHA256SUMS, then installed with APT for host dependencies.
SDK downloads are public and need no production credentials or additional token.
SDKs are never compiled in these workflows. To change the pin, update the action's
version default and validate all targets, including the x32 emulator job.

To install all SDKs locally on Ubuntu 24.04/26.04 amd64:

```sh
curl -fsSL https://apt.ptos.dev/ptos-archive-keyring.gpg |
  sudo tee /usr/share/keyrings/ptos-archive-keyring.gpg >/dev/null
echo 'deb [arch=amd64 signed-by=/usr/share/keyrings/ptos-archive-keyring.gpg] https://apt.ptos.dev stable main' |
  sudo tee /etc/apt/sources.list.d/ptos.list
sudo apt update
sudo apt install ptos-toolchain=0.2.0
python3 -m venv .venv
. .venv/bin/activate
python3 -m pip install kconfiglib==14.1.0
```

Build with the SDK prefixes (local legacy compiler defaults remain supported):

```sh
make all M68K_PREFIX=m68k-unknown-ptos-prgelf- M68K_LDFLAGS=-mmintelf
make arm ARM_PREFIX=arm-unknown-ptos-elf-
make x32 PTOS=/path/to/built/pTOS X32_PREFIX=x86_64-unknown-ptos-elfx32-
```

`-mmintelf` keeps the m68k output a GEMDOS PRG/ELF hybrid rather than the SDK's
default plain ELF. The selected prefixes are forwarded to pinned libcmini and
GEMlib submodule builds, preserving the projects' own library configurations.
The x32 CI fixture keeps its existing pinned pTOS commit and Ubuntu 24.04/QEMU/
OVMF combination; it now passes the SDK prefix for the runtime and kernel too.
GUI, shared-library dependency checks and artifacts are retained.

The legacy alternatives below describe the local defaults, not CI installation.

## m68k (the real target)

The `m68k-atari-mintelf-` toolchain. It links a hybrid executable: a GEMDOS PRG
that is also an ELF, so the same `TYPESHLF.PRG` loads on real TOS, EmuTOS and
pTOS.

Legacy Ubuntu toolchain (not used by CI):

    sudo add-apt-repository -y ppa:vriviere/mintelf
    sudo apt-get update
    sudo apt-get install -y cross-mintelf-essential

macOS: Thorsten Otto's archives from <https://tho-otto.m68k.eu/crossmint.php>
(`binutils-*-mintelf-*-bin-macos`, `gcc-*-mintelf-*-bin-macos` and
`mintlib-*-mintelf-*-dev`), extracted into `/opt/cross-mint`, then:

    export PATH=/opt/cross-mint/bin:$PATH

libcmini's configuration also needs Python 3 and kconfiglib in an active
virtual environment, as shown above.

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
CI pins the coordinated pTOS #955 runtime to
`41e77f9d8bd1215207c511ebb318ada3eb1d6018`. Initialize pTOS's submodules
before building:

    git submodule update --init --recursive
    make pc-x86_64-libaes-desktop_defconfig
    make CROSS_COMPILE=x86_64-unknown-ptos-elfx32- -j4

This configuration uses the external shared libaes desktop with kernel AES
disabled (`CONF_WITH_AES=n`). CI checks that the desktop links libaes and
that the kernel contains no retired AES service entry points.

The build also requires `obj/gemlib/libgem.a` from that pTOS tree. It supplies
the classic AES bindings and TypeShelf's own `aes_global`, `gl_apid` and
`gl_ap_version`; `libaes.so` now exports the `mt_*` implementations. The archive
must precede `-laes` in the link. Older pTOS builds with the classic bindings
inside `libaes.so` do not provide this interface.

CI uses the pTOS SDK for the application, shared runtime and EFI kernel.
The local default remains native GCC 12 or newer and `objcopy`; set
`X32_PREFIX=x86_64-unknown-ptos-elfx32-` to use the SDK for the application. It builds a separate x32
copy of libcmini, so m68k, ARM and x32 builds can run in parallel.

Install the matching pTOS runtime (`LD.SO`, `libptos.so`, `libaes.so`) in
`\GEMSYS\LIB\X32` on the guest. Keep `TYPESHLF.PRG` and `TYPESHLF.RSC`
together. This is a pTOS executable, not a Linux x32 application.

The x32 target is experimental: the pinned pTOS VDI client does not yet
implement the font-name and metric queries TypeShelf needs, or font loading
and scalable-font inquiries. Browsing and installing fonts therefore require
further pTOS VDI support. Rejected queries return deterministic empty results.
CI checks linking, ELF dependencies and ownership of the AES bindings, then
boots TypeShelf, sends view/window shortcuts and checks its clean exit. This
smoke test does not verify font browsing or installation.

To run the same GUI smoke test, install QEMU, OVMF, mtools and dosfstools, then
build pTOS's test desktop instead of the normal configuration above:

    cd ../pTOS
    make pc-x86_64-desktop-tests-builtin-cli_defconfig
    make CROSS_COMPILE=x86_64-unknown-ptos-elfx32- -j4
    make CROSS_COMPILE=x86_64-unknown-ptos-elfx32- -j4 x32-desktop-test
    cd ../TypeShelf
    make x32 PTOS=../pTOS
    python3 tests/emulator/x32.py --ptos ../pTOS \
        --ovmf-code /usr/share/OVMF/OVMF_CODE_4M.fd \
        --ovmf-vars /usr/share/OVMF/OVMF_VARS_4M.fd

Screenshots and the guest log are saved under `build/emulator/x32/`. Override
the firmware paths or pass `--qemu` for another installation.

`make test-x32-adapter` runs host regression checks for native VDI failures,
workstation capabilities and unavailable GDOS information. It needs the GEMlib
submodule headers but no pTOS runtime, and also runs in the x32 CI job.

`make check-x86_64` remains available as a compile-only portability check.
