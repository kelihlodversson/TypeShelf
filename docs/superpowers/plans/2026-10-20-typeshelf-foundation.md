# Type Shelf Foundation Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking. The owner has asked that the subagents run on the **haiku** model, so each task below is self-contained: exact files, exact code, exact commands.

**Goal:** A Type Shelf repository that builds an empty GEM application as `TYPESHLF.PRG` for m68k, with the font-list, GDOS-decoding and layout logic written and unit-tested on the host, a real font source over gemlib, and CI.

**Architecture:** Pure logic (`fontdb`, `gdosinfo`, `layout`) knows nothing of GEM and is tested natively; the VDI is reached only through the `VdiFontSource` seam, whose real implementation (`vdi_if_gem.c`) is built for m68k and compile-checked for ARM and x32. Machine specifics are isolated in `src/platform/<name>/`. gemlib and libcmini are pinned submodules of the owner's forks.

**Tech Stack:** C (`-std=gnu99`), GNU make, `m68k-atari-mintelf-` GCC, gemlib, libcmini, GitHub Actions.

**Spec:** `docs/superpowers/specs/2026-10-20-typeshelf-design.md`

## Global Constraints

Every task's requirements include these.

- m68k: 68000 (`-m68000`), 32-bit `int`, `-nostdlib`, linked as `crt0.o ... -lcmini -lgem -lgcc -lcmini`, toolchain `m68k-atari-mintelf-`; output `TYPESHLF.PRG` (an 8.3 name for real TOS). No MiNT features.
- ARM and x86_64 are compile-check only: `arm-none-eabi-` and `-mx32`, `-c` with no link, until gemlib has their trap layers.
- Licence GPL-2.0-or-later. C is `-std=gnu99`, 4 spaces and never a tab in `.c`/`.h`, `/* */` comments.
- `int` is 32 bits on every target. Anything crossing the AES/VDI arrays is `short` (gemlib's type), never `int`.
- No m68k assumptions outside `src/platform/m68k/`. Machine specifics live only in `src/platform/<name>/`.
- Pure logic (`src/fontdb`, `src/gdosinfo`, `src/layout`) includes no GEM header and has a host test in `tests/`.
- Memory is allocated exactly; no large fixed-size arrays. The target machines have 512 KB.
- Minimum screen is ST-LOW (320x200); the zoom range is 8 to 72 points. (Used from the UI plans on; the layout maths here already honours both.)
- gemlib and libcmini are the user's forks, as git submodules under `third_party/`, pinned. They are never edited from this repo.

## Where this plan sits

The spec describes one application; it is built in four plans, each producing working, tested software on its own:

1. **Foundation (this plan):** build system, the tested logic, the real font source, CI.
2. **UI framework:** windows, the toolbar and its slider and popup, menus, the event loop, redraw and scrolling.
3. **Views:** Icons, Samples, List, Specimen and the single-font Samples.
4. **Install Font, About, and real-machine checks:** the Hatari smoke tests that run `TYPESHLF.PRG` on EmuTOS and pTOS in ST-LOW, ST-MEDIUM and ST-HIGH, and the first run of the assumptions marked in Task 7.

Plans 2 to 4 are written after this one lands, because what the first real run shows about the VDI shapes them.

**Differences from the spec**, all small and made while verifying the build:

- `make TARGET=m68k|arm|x86_64` became the targets `make`, `make check-arm` and `make check-x86_64`; ARM and x32 never link in this plan, so a `TARGET` variable would only select a compile check.
- `src/platform/{m68k,arm,x86_64}/` became `src/platform/{m68k,generic}/`: ARM and x32 would have had identical stubs.
- gemlib's headers include `<compiler.h>`, which only the m68k sysroot has, so `src/compat/compiler.h` is added for the ARM and x32 compiles.
- The NVDI calls (`vqt_ext_name`, `vqt_xfntinfo`, `vst_arbpt`, `XFNT_INFO`) are declared by `<gemx.h>` and present in `libgem.a`, so no declarations of our own are needed.

## Workflow

Follow `CLAUDE.md` (written in Task 1; until then, the same rules): every task is a GitHub issue, and work happens on an epic branch.

- Task 0 creates the epic and one issue per task. Task N below is the Nth sub-issue.
- Create `epic/<epic-issue>-foundation` from `main` and push it. For each task create `feature/<issue>-<title-with-dashes>` off the epic branch, push an empty "Start work on #<issue>" commit, and open a **draft** PR whose base is the epic branch. Do the task, push, and mark the PR ready (`gh pr ready`). The owner merges it into the epic branch.
- Start the next task from the updated epic branch, since each task builds on the last.
- When all eight are merged, open the final PR from the epic branch into `main`.
- Never merge or force-push a PR. End every commit message with the two attribution lines the controller gives you.

## File structure

| File | Responsibility | Task |
| --- | --- | --- |
| `Makefile` | host tests, m68k build, ARM/x32 compile checks, libraries | 1, extended in 2-7 |
| `CLAUDE.md`, `docs/toolchains.md`, `.gitignore` | workflow, conventions, toolchain setup | 1 |
| `src/compat/compiler.h` | stand-in for the mintlib header gemlib includes (ARM, x32) | 1 |
| `src/main.c` | the application: empty in Task 1, lists the fonts from Task 7 | 1, 7 |
| `src/fontdb/split.[ch]` | family and style from a full name | 2 |
| `src/fontdb/group.[ch]` | the family/style list: add, sort, find, free | 3 |
| `src/fontdb/build.[ch]` | build the list from a `VdiFontSource` | 6 |
| `src/gdosinfo/decode.[ch]` | GDOS, FSMC, NVDI and OS values as text | 4 |
| `src/layout/layout.[ch]` | grid, scroll and zoom arithmetic | 5 |
| `src/vdi_if/vdi_if.h` | the seam: `VdiFace`, `VdiFontSource` | 6 |
| `src/vdi_if/gem_source.h`, `vdi_if_gem.c` | the seam's real implementation over gemlib | 7 |
| `src/platform/platform.h`, `m68k/`, `generic/` | cookies and the OS header | 7 |
| `tests/test.h`, `main.c`, `test_*.c` | host unit tests | 2-6 |
| `.github/workflows/build.yml` | CI | 8 |

---

### Task 0: Create the tracking issues

**Files:** none (GitHub only).

- [ ] **Step 1: Create the epic and one issue per task**

```bash
REPO=kelihlodversson/TypeShelf
EPIC_URL=$(gh issue create -R $REPO --title "Epic: Type Shelf foundation (build, font list, GDOS facts, layout maths)" \
    --body "Plan: docs/superpowers/plans/2026-10-20-typeshelf-foundation.md. Spec: docs/superpowers/specs/2026-10-20-typeshelf-design.md.")
EPIC=${EPIC_URL##*/}
i=0
while IFS= read -r title; do
    i=$((i + 1))
    url=$(gh issue create -R $REPO --title "$title" \
        --body "Task $i of the foundation plan (docs/superpowers/plans/2026-10-20-typeshelf-foundation.md). Part of #$EPIC.")
    id=$(gh api repos/$REPO/issues/${url##*/} --jq .id)
    gh api -X POST repos/$REPO/issues/$EPIC/sub_issues -F sub_issue_id=$id --jq .title
done <<'EOF'
Scaffold, and build the libraries and an empty GEM app as TYPESHLF.PRG
fontdb: split a font name into family and style, with the host test harness
fontdb: group fonts into families and styles
gdosinfo: decode vq_vgdos, FSMC, NVDI and OS values for the About dialog
layout: grid, scroll and zoom arithmetic
fontdb: the VDI seam and building the font list from a source
Read the fonts through gemlib, and show what was found
CI: host tests, the m68k build, and the ARM and x32 compile checks
EOF
```

Expected: the eight issue titles are printed, and the epic lists them as sub-issues.

- [ ] **Step 2: Create the epic branch**

```bash
git switch main && git pull
git switch -c epic/<epic-issue>-foundation
git push -u origin HEAD
```

---

### Task 1: Scaffold the repository and build an empty GEM application

**Files:**
- Create: `.gitignore`, `CLAUDE.md`, `docs/toolchains.md`, `Makefile`
- Create: `src/main.c`, `src/compat/compiler.h`
- Create (git): `.gitmodules`, `third_party/gemlib`, `third_party/libcmini`

**Interfaces:**
- Consumes: The user's forks `kelihlodversson/gemlib` and `kelihlodversson/libcmini` (public, branch `master`).
- Produces: `make` builds `build/m68k/TYPESHLF.PRG` (a GEMDOS PRG that is also an ELF) from `src/main.c`, building gemlib and libcmini from the submodules.
- Produces: `make check-arm` and `make check-x86_64` compile `CHECK_SRC` for ARM and x32.
- Produces: Makefile variables later tasks extend: `LOGIC_SRC` (portable sources, `+=` one per line), `GEM_SRC`, `M68K_PLATFORM_SRC`, `GENERIC_PLATFORM_SRC`.

- [ ] **Step 1: Add the two libraries as submodules**

Building generates files inside the submodules; `ignore = dirty` keeps `git status` clean.

Run:

```bash
git submodule add https://github.com/kelihlodversson/gemlib.git third_party/gemlib
git submodule add https://github.com/kelihlodversson/libcmini.git third_party/libcmini
git config -f .gitmodules submodule.third_party/gemlib.ignore dirty
git config -f .gitmodules submodule.third_party/libcmini.ignore dirty
git submodule status
```

Expected: it succeeds and the output matches `third_party/libcmini`.

- [ ] **Step 2: Create the ignore file**

Create `.gitignore`:

```gitignore
build/
```

- [ ] **Step 3: Create the project instructions**

Create `CLAUDE.md`:

```markdown
# Type Shelf

A Font Book style font viewer for GEM: it lists the fonts the VDI/GDOS has
installed, grouped by family and style, and reports which VDI/GDOS is in use.
It targets m68k first (TOS, EmuTOS and pTOS), built on gemlib and libcmini.

The design is `docs/superpowers/specs/2026-10-20-typeshelf-design.md`; read it
before changing anything.

## Workflow

Every change starts from a GitHub issue. If none exists for the work, create
it first.

1. Create a branch off `main` named `{type}/{issue}-{title-with-dashes}`,
   where `{type}` is `feature`, `bugfix`, `chore`, ... and `{issue}` is the
   issue number, e.g. `feature/4-name-splitting`.
2. Immediately push an empty commit and open a **draft** PR from it, before
   any real work exists:

       git commit --allow-empty -m "Start work on #<issue>"
       git push -u origin HEAD
       gh pr create --draft --title "..." --body "Fixes #<issue>"

3. Then do the work, committing and pushing as you go.
4. When it is done, mark the PR ready for review (`gh pr ready`).

The owner merges the PR. Never merge or force-push a PR as part of this flow.

### Epic branches

For work that spans several issues, create an epic branch named
`epic/{issue}-{title-with-dashes}`. Each sub-issue gets its own branch off the
epic branch, and its PR targets the epic branch, so every sub-issue goes
through CI and review before it is merged into the epic. When all sub-issues
are done, a final PR merges the epic branch into `main`.

## Building

    make test             host unit tests (native compiler)
    make                  build/m68k/TYPESHLF.PRG
    make check-arm        compile the portable sources for ARM (no link)
    make check-x86_64     compile them for x32 (no link)
    make clean

Needs GNU make (on macOS: `gmake`, Apple's is 3.81), and for m68k the
`m68k-atari-mintelf-` toolchain on `PATH`, plus `pip3 install kconfiglib` for
libcmini's configuration. `docs/toolchains.md` says where to get them.

The submodules must be checked out: `git submodule update --init`.

## Code conventions

- C99 with GNU extensions (`-std=gnu99`); 4 spaces, never a tab, in `.c` and
  `.h`; `/* */` comments. The Makefile uses tabs, as it must.
- `int` is 32 bits on every target. Use `short` for the elements of AES and
  VDI arrays, as gemlib does.
- Pure logic (`src/fontdb`, `src/gdosinfo`, `src/layout`) includes no GEM
  header and has a test in `tests/`. Write the test first and watch it fail.
- Code that calls the VDI or AES is in `src/vdi_if/` and the UI modules.
  Machine specifics are only in `src/platform/<name>/`.
- Allocate memory exactly. The target machines have 512 KB; no big fixed
  arrays of names.
- `third_party/` holds submodules of the user's forks of gemlib and libcmini.
  Do not edit them here: change them in their own repository, then update the
  pinned commit.
```

- [ ] **Step 4: Create the toolchain notes**

Create `docs/toolchains.md`:

```markdown
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

## ARM (compile check)

    sudo apt-get install -y gcc-arm-none-eabi       # Ubuntu
    brew install gcc-arm-embedded                   # macOS

## x86_64 (compile check, x32)

    sudo apt-get install -y gcc gcc-multilib libc6-dev-x32     # Ubuntu
    brew install x86_64-elf-gcc                                # macOS

On macOS run `make check-x86_64 X86_CC=x86_64-elf-gcc`.
```

- [ ] **Step 5: Create the compiler.h stand-in**

gemlib's headers include `<compiler.h>`, which the m68k sysroot provides. ARM and x86 have no such sysroot, so their checks put this directory first on the include path.

Create `src/compat/compiler.h`:

```c
/*
 * compiler.h - stand-in for the mintlib header gemlib's headers include
 *
 * On m68k it comes with the toolchain's sysroot.  The ARM and x86 builds have
 * no such sysroot, so they put this directory on the include path instead.
 */
#ifndef TYPESHELF_COMPAT_COMPILER_H
#define TYPESHELF_COMPAT_COMPILER_H

#ifdef __cplusplus
#define __BEGIN_DECLS extern "C" {
#define __END_DECLS   }
#else
#define __BEGIN_DECLS
#define __END_DECLS
#endif

#define __CDECL

#endif
```

- [ ] **Step 6: Create the empty GEM application**

Create `src/main.c`:

```c
#include <gem.h>

int main(void)
{
    if (appl_init() < 0)
        return 1;
    form_alert(1, "[1][Type Shelf][OK]");
    appl_exit();
    return 0;
}
```

- [ ] **Step 7: Create the Makefile**

Recipe lines start with a tab.

Create `Makefile`:

```make
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
GEM_SRC   = src/main.c
M68K_PLATFORM_SRC =
GENERIC_PLATFORM_SRC =

STD      = -std=gnu99
WARN     = -Wall -Wextra
INCLUDES = -Isrc -I$(GEMLIB) -I$(LIBCMINI)/include

.PHONY: all test check-arm check-x86_64 clean
all: build/m68k/TYPESHLF.PRG

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
```

- [ ] **Step 8: Build the PRG**

The first build also builds libcmini and gemlib; allow a minute. On macOS use `gmake` and put `/opt/cross-mint/bin` on `PATH` (docs/toolchains.md).

Run:

```bash
make
```

Expected: it succeeds and the output matches `TYPESHLF.PRG`.

- [ ] **Step 9: Check that it is a GEMDOS program**

`601a` is the PRG header. The file is also an ELF, which is what lets pTOS load it as well.

Run:

```bash
od -A n -t x1 -N 2 build/m68k/TYPESHLF.PRG | tr -d ' '
```

Expected: it succeeds and the output matches `601a`.

- [ ] **Step 10: Check the ARM and x32 compiles**

On macOS pass `X86_CC=x86_64-elf-gcc`; on Ubuntu the defaults work.

Run:

```bash
make check-arm
make check-x86_64
```

Expected: it succeeds and the output matches `main.c`.

- [ ] **Step 11: Commit**

```bash
git add .gitignore .gitmodules third_party CLAUDE.md docs/toolchains.md src Makefile
git commit -m "Scaffold the build: submodules, Makefile and an empty GEM app"
git push
```

End the commit message with the attribution lines given in the Workflow section.

---

### Task 2: The unit-test harness and splitting a font name into family and style

**Files:**
- Create: `tests/test.h`, `tests/main.c`, `tests/test_split.c`
- Create: `src/fontdb/split.h`, `src/fontdb/split.c`
- Modify: `Makefile` (host test section, `LOGIC_SRC`)

**Interfaces:**
- Consumes: Makefile variables `LOGIC_SRC` and `STD`/`WARN` from Task 1.
- Produces: `make test` builds and runs `build/host/tests`, printing `N checks, M failures` and exiting non-zero on any failure.
- Produces: `tests/test.h` macros `CHECK(cond)`, `CHECK_INT(got, want)`, `CHECK_STR(got, want)`; each test file defines `void test_xxx(void)` and `tests/main.c` calls it.
- Produces: `void font_split_name(const char *full, FontNameParts *out)`: `FontNameParts` has `char family[SPLIT_NAME_MAX]` and `char style[SPLIT_NAME_MAX]`, `SPLIT_NAME_MAX` is 64. A name with no style word gets the style `"Regular"`.
- Produces: `const FontStyleWord *font_style_word(const char *s, size_t len)`: `FontStyleWord` has `const char *word`, `int weight` (0 thin .. 3 regular .. 8 black; -1 for a slope word) and `int slant` (1 for italic and oblique); NULL if `s[0..len)` is not a style word.

- [ ] **Step 1: Create the test macros**

Create `tests/test.h`:

```c
#ifndef TEST_H
#define TEST_H

#include <stdio.h>
#include <string.h>

extern int test_checks;
extern int test_failures;

#define CHECK(cond) do { \
    test_checks++; \
    if (!(cond)) { \
        test_failures++; \
        printf("FAIL %s:%d: %s\n", __FILE__, __LINE__, #cond); \
    } \
} while (0)

#define CHECK_INT(got, want) do { \
    long g_ = (long)(got), w_ = (long)(want); \
    test_checks++; \
    if (g_ != w_) { \
        test_failures++; \
        printf("FAIL %s:%d: %s: got %ld, want %ld\n", \
               __FILE__, __LINE__, #got, g_, w_); \
    } \
} while (0)

#define CHECK_STR(got, want) do { \
    const char *g_ = (got), *w_ = (want); \
    test_checks++; \
    if (strcmp(g_, w_) != 0) { \
        test_failures++; \
        printf("FAIL %s:%d: %s: got \"%s\", want \"%s\"\n", \
               __FILE__, __LINE__, #got, g_, w_); \
    } \
} while (0)

#endif
```

- [ ] **Step 2: Create the test runner**

Create `tests/main.c`:

```c
#include <stdio.h>
#include "test.h"

int test_checks;
int test_failures;

void test_split(void);

int main(void)
{
    test_split();

    printf("%d checks, %d failures\n", test_checks, test_failures);
    return test_failures != 0;
}
```

- [ ] **Step 3: Declare the interface**

Create `src/fontdb/split.h`:

```c
/*
 * split.h - split a full font name into family and style
 *
 * The VDI reports one name per face ("Dutch 801 Bold Italic").  Where
 * vqt_xfntinfo() is not there to say which part is the family and which the
 * style, the style is taken to be the trailing weight and slope words.
 * Width words (Condensed, Narrow, ...) stay in the family: "Arial Narrow" is
 * a family of its own.
 */
#ifndef FONTDB_SPLIT_H
#define FONTDB_SPLIT_H

#include <stddef.h>

#define SPLIT_NAME_MAX 64

typedef struct {
    char family[SPLIT_NAME_MAX];
    char style[SPLIT_NAME_MAX];
} FontNameParts;

/* a word that can be part of a style name */
typedef struct {
    const char *word;
    int weight;     /* 0 thin ... 3 regular ... 8 black; -1 if not a weight */
    int slant;      /* 1 for italic and oblique */
} FontStyleWord;

/* the style word equal to s[0..len), ignoring case, or NULL */
const FontStyleWord *font_style_word(const char *s, size_t len);

/* never fails; a name with no style word gets the style "Regular" */
void font_split_name(const char *full, FontNameParts *out);

#endif
```

- [ ] **Step 4: Write the failing test**

Create `tests/test_split.c`:

```c
#include <string.h>
#include "test.h"
#include "fontdb/split.h"

static void check_split(const char *full, const char *family, const char *style)
{
    FontNameParts parts;

    font_split_name(full, &parts);
    CHECK_STR(parts.family, family);
    CHECK_STR(parts.style, style);
}

void test_split(void)
{
    FontNameParts parts;
    char longname[200];
    int i;

    check_split("Arial", "Arial", "Regular");
    check_split("Arial Bold", "Arial", "Bold");
    check_split("Arial Bold Italic", "Arial", "Bold Italic");
    check_split("DejaVu Sans Mono Bold Oblique", "DejaVu Sans Mono", "Bold Oblique");
    check_split("Arial Rounded MT Bold", "Arial Rounded MT", "Bold");
    check_split("  Dutch 801   Roman ", "Dutch 801", "Roman");
    check_split("arial BOLD", "arial", "BOLD");

    /* width words are part of the family */
    check_split("Arial Narrow", "Arial Narrow", "Regular");
    check_split("Arial Narrow Bold", "Arial Narrow", "Bold");

    /* a name made only of style words is a family */
    check_split("Bold", "Bold", "Regular");
    check_split("Black Italic", "Black", "Italic");

    check_split("", "", "Regular");
    check_split("   ", "", "Regular");

    /* too long: cut, never overrun */
    memset(longname, 'x', sizeof longname - 1);
    longname[sizeof longname - 1] = '\0';
    font_split_name(longname, &parts);
    CHECK(strlen(parts.family) < SPLIT_NAME_MAX);

    /* more words than tokens: still terminates */
    longname[0] = '\0';
    for (i = 0; i < 50; i++)
        strcat(longname, "a ");
    font_split_name(longname, &parts);
    CHECK(strlen(parts.family) < SPLIT_NAME_MAX);

    CHECK(font_style_word("bold", 4) != NULL);
    CHECK(font_style_word("bolder", 6) == NULL);
    CHECK_INT(font_style_word("Heavy", 5)->weight, 8);
    CHECK_INT(font_style_word("Oblique", 7)->slant, 1);
}
```

- [ ] **Step 5: Add the host test section to the Makefile**

Insert the host unit tests section just before the `# ---- the libraries` section, then add the test source:

In `Makefile`, replace:

```
# ---- the libraries -----------------------------------------------------
```

with:

```
# ---- host unit tests ---------------------------------------------------

TEST_SRC   =
CC_HOST    ?= cc
HOST_FLAGS ?= $(STD) $(WARN) -Werror -g -Isrc -Itests

build/host/tests: $(LOGIC_SRC) $(TEST_SRC) $(wildcard src/*/*.h tests/*.h)
	mkdir -p build/host
	$(CC_HOST) $(HOST_FLAGS) $(LOGIC_SRC) $(TEST_SRC) -o $@

test: build/host/tests
	./build/host/tests

# ---- the libraries -----------------------------------------------------
```

- [ ] **Step 6: Register the test file**

In `Makefile`, replace:

```
TEST_SRC   =
```

with:

```
TEST_SRC   =
TEST_SRC  += tests/main.c
TEST_SRC  += tests/test_split.c
```

- [ ] **Step 7: Run the test to see it fail**

It fails to link: `font_split_name` and `font_style_word` do not exist yet.

Run:

```bash
make test
```

Expected: it FAILS (matching `ndefined`).

- [ ] **Step 8: Write the implementation**

Create `src/fontdb/split.c`:

```c
#include <string.h>
#include <ctype.h>
#include "fontdb/split.h"

#define MAX_TOKENS 32

static const FontStyleWord style_words[] = {
    { "Thin",       0, 0 },
    { "ExtraLight", 1, 0 },
    { "Light",      2, 0 },
    { "Regular",    3, 0 },
    { "Roman",      3, 0 },
    { "Book",       3, 0 },
    { "Normal",     3, 0 },
    { "Medium",     4, 0 },
    { "SemiBold",   5, 0 },
    { "DemiBold",   5, 0 },
    { "Bold",       6, 0 },
    { "ExtraBold",  7, 0 },
    { "Heavy",      8, 0 },
    { "Black",      8, 0 },
    { "Italic",    -1, 1 },
    { "Oblique",   -1, 1 },
    { NULL,         0, 0 }
};

const FontStyleWord *font_style_word(const char *s, size_t len)
{
    const FontStyleWord *w;
    size_t i;

    for (w = style_words; w->word != NULL; w++) {
        if (strlen(w->word) != len)
            continue;
        for (i = 0; i < len; i++) {
            if (tolower((unsigned char)s[i]) != tolower((unsigned char)w->word[i]))
                break;
        }
        if (i == len)
            return w;
    }
    return NULL;
}

/* join tokens [from, to) with single spaces into dst, never past its end */
static void join_tokens(char *dst, const char *const *start, const size_t *len,
                        int from, int to)
{
    size_t used = 0;
    int i;

    dst[0] = '\0';
    for (i = from; i < to; i++) {
        size_t need = len[i] + (used > 0 ? 1 : 0);

        if (used + need >= SPLIT_NAME_MAX)
            break;
        if (used > 0)
            dst[used++] = ' ';
        memcpy(dst + used, start[i], len[i]);
        used += len[i];
        dst[used] = '\0';
    }
}

void font_split_name(const char *full, FontNameParts *out)
{
    const char *start[MAX_TOKENS];
    size_t len[MAX_TOKENS];
    int n = 0;
    int first_style;
    const char *p = full;

    while (*p != '\0' && n < MAX_TOKENS) {
        while (*p == ' ')
            p++;
        if (*p == '\0')
            break;
        start[n] = p;
        while (*p != '\0' && *p != ' ')
            p++;
        len[n] = (size_t)(p - start[n]);
        n++;
    }

    /* the family keeps at least one token: the name "Bold" is a family */
    first_style = n;
    while (first_style > 1 &&
           font_style_word(start[first_style - 1], len[first_style - 1]) != NULL)
        first_style--;

    join_tokens(out->family, start, len, 0, first_style);
    if (first_style == n)
        strcpy(out->style, "Regular");
    else
        join_tokens(out->style, start, len, first_style, n);
}
```

- [ ] **Step 9: Add it to the portable sources**

In `Makefile`, replace:

```
LOGIC_SRC =
```

with:

```
LOGIC_SRC =
LOGIC_SRC += src/fontdb/split.c
```

- [ ] **Step 10: Run the test to see it pass**

Run:

```bash
make test
```

Expected: it succeeds and the output matches `d+ checks, 0 failures`.

- [ ] **Step 11: Check that the other builds still work**

Run:

```bash
make
make check-arm
```

Expected: it succeeds and the output matches `split.c`.

- [ ] **Step 12: Commit**

```bash
git add tests src/fontdb Makefile
git commit -m "fontdb: split a font name into family and style, with a host test harness"
git push
```

End the commit message with the attribution lines given in the Workflow section.

---

### Task 3: Group fonts into families and styles

**Files:**
- Create: `src/fontdb/group.h`, `src/fontdb/group.c`, `tests/test_group.c`
- Modify: `tests/main.c`, `Makefile`

**Interfaces:**
- Consumes: `font_split_name()`, `FontNameParts`, `font_style_word()`, `FontStyleWord` from Task 2.
- Produces: `FONT_FORMAT_BITMAP` 0x01, `FONT_FORMAT_SPEEDO` 0x02, `FONT_FORMAT_TRUETYPE` 0x04, `FONT_FORMAT_TYPE1` 0x08, `FONT_FORMAT_VECTOR` 0x10.
- Produces: `FontFace {short id; unsigned short format, flags; const char *full_name, *family, *style;}` (input); `FontStyle {short id; unsigned short format, flags; char *style;}`; `FontFamily {char *name; FontStyle *styles; int style_count; unsigned short formats;}`; `FontList {FontFamily *families; int family_count;}`.
- Produces: `void fontlist_init(FontList *)`, `int fontlist_add(FontList *, const FontFace *)` (0, or -1 if out of memory), `void fontlist_sort(FontList *)`, `const FontFamily *fontlist_find(const FontList *, const char *name)` (ignores case), `void fontlist_free(FontList *)`, `int font_style_rank(const char *style)`.
- Produces: Sort order: families by name ignoring case; styles by `font_style_rank` (weight * 2, +1 if italic or oblique), then name.

- [ ] **Step 1: Declare the interface**

Create `src/fontdb/group.h`:

```c
/*
 * group.h - the installed fonts, grouped into families and their styles
 *
 * Memory is allocated exactly: the list is built on machines with a few
 * hundred kilobytes, so there are no fixed-size arrays of names.
 */
#ifndef FONTDB_GROUP_H
#define FONTDB_GROUP_H

#include "fontdb/split.h"

/* what vqt_ext_name() calls font_format, plus one of ours */
#define FONT_FORMAT_BITMAP    0x01
#define FONT_FORMAT_SPEEDO    0x02
#define FONT_FORMAT_TRUETYPE  0x04
#define FONT_FORMAT_TYPE1     0x08
#define FONT_FORMAT_VECTOR    0x10  /* a vector font, format not reported */

/* one face as the VDI reports it */
typedef struct {
    short id;
    unsigned short format;
    unsigned short flags;
    const char *full_name;      /* always set */
    const char *family;         /* NULL or "": work it out from full_name */
    const char *style;          /* NULL or "": work it out from full_name */
} FontFace;

typedef struct {
    short id;
    unsigned short format;
    unsigned short flags;
    char *style;
} FontStyle;

typedef struct {
    char *name;
    FontStyle *styles;
    int style_count;
    unsigned short formats;     /* the styles' formats, ORed */
} FontFamily;

typedef struct {
    FontFamily *families;
    int family_count;
} FontList;

void fontlist_init(FontList *list);

/* 0 on success, -1 if out of memory (the list stays usable) */
int fontlist_add(FontList *list, const FontFace *face);

/* families by name, ignoring case; styles by weight, upright before slanted */
void fontlist_sort(FontList *list);

/* the family called name, ignoring case, or NULL */
const FontFamily *fontlist_find(const FontList *list, const char *name);

void fontlist_free(FontList *list);

/* the sort key of a style name: weight * 2 + 1 if it is italic or oblique */
int font_style_rank(const char *style);

#endif
```

- [ ] **Step 2: Write the failing test**

Create `tests/test_group.c`:

```c
#include "test.h"
#include "fontdb/group.h"

static FontFace face(short id, unsigned short format, const char *name)
{
    FontFace f;

    f.id = id;
    f.format = format;
    f.flags = 0;
    f.full_name = name;
    f.family = NULL;
    f.style = NULL;
    return f;
}

void test_group(void)
{
    FontList list;
    FontFace f;
    const FontFamily *fam;

    fontlist_init(&list);
    CHECK_INT(list.family_count, 0);

    f = face(11, FONT_FORMAT_BITMAP, "Swiss Bold");
    CHECK_INT(fontlist_add(&list, &f), 0);
    f = face(12, FONT_FORMAT_BITMAP, "Dutch");
    CHECK_INT(fontlist_add(&list, &f), 0);
    f = face(13, FONT_FORMAT_BITMAP, "swiss");
    CHECK_INT(fontlist_add(&list, &f), 0);
    f = face(14, FONT_FORMAT_TRUETYPE, "Swiss Bold Italic");
    CHECK_INT(fontlist_add(&list, &f), 0);
    f = face(15, FONT_FORMAT_TRUETYPE, "Swiss Light");
    CHECK_INT(fontlist_add(&list, &f), 0);

    /* "swiss" and "Swiss" are the same family */
    CHECK_INT(list.family_count, 2);

    fontlist_sort(&list);
    CHECK_STR(list.families[0].name, "Dutch");
    CHECK_STR(list.families[1].name, "Swiss");

    fam = fontlist_find(&list, "SWISS");
    CHECK(fam != NULL);
    if (fam != NULL) {
        CHECK_INT(fam->style_count, 4);
        /* Light, Regular, Bold, Bold Italic */
        CHECK_STR(fam->styles[0].style, "Light");
        CHECK_STR(fam->styles[1].style, "Regular");
        CHECK_STR(fam->styles[2].style, "Bold");
        CHECK_STR(fam->styles[3].style, "Bold Italic");
        CHECK_INT(fam->styles[3].id, 14);
        CHECK_INT(fam->formats, FONT_FORMAT_BITMAP | FONT_FORMAT_TRUETYPE);
    }
    CHECK(fontlist_find(&list, "Nothing") == NULL);

    /* an explicit family and style win over the full name */
    f = face(20, FONT_FORMAT_SPEEDO, "DejaVu Sans Mono");
    f.family = "DejaVu Sans Mono";
    f.style = "Book";
    CHECK_INT(fontlist_add(&list, &f), 0);
    fam = fontlist_find(&list, "DejaVu Sans Mono");
    CHECK(fam != NULL);
    if (fam != NULL)
        CHECK_STR(fam->styles[0].style, "Book");

    /* a family without a style is not trusted: the name is split instead */
    f = face(21, FONT_FORMAT_SPEEDO, "Foo Bold");
    f.family = "Foo Bold";
    f.style = "";
    CHECK_INT(fontlist_add(&list, &f), 0);
    CHECK(fontlist_find(&list, "Foo") != NULL);

    CHECK_INT(font_style_rank("Regular"), 6);
    CHECK_INT(font_style_rank("Italic"), 7);
    CHECK_INT(font_style_rank("Bold Italic"), 13);
    CHECK_INT(font_style_rank("Thin"), 0);
    CHECK_INT(font_style_rank("Whatever"), 6);

    fontlist_free(&list);
    CHECK_INT(list.family_count, 0);
    CHECK(list.families == NULL);
}
```

- [ ] **Step 3: Run the new test from the runner**

In `tests/main.c`, replace:

```
void test_split(void);
```

with:

```
void test_split(void);
void test_group(void);
```

and replace:

```
    test_split();
```

with:

```
    test_split();
    test_group();
```

- [ ] **Step 4: Register the test file**

In `Makefile`, replace:

```
TEST_SRC  += tests/test_split.c
```

with:

```
TEST_SRC  += tests/test_split.c
TEST_SRC  += tests/test_group.c
```

- [ ] **Step 5: Run the test to see it fail**

`fontlist_*` do not exist yet.

Run:

```bash
make test
```

Expected: it FAILS (matching `ndefined`).

- [ ] **Step 6: Write the implementation**

Create `src/fontdb/group.c`:

```c
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "fontdb/group.h"

static char *dup_str(const char *s)
{
    size_t n = strlen(s) + 1;
    char *p = malloc(n);

    if (p != NULL)
        memcpy(p, s, n);
    return p;
}

static int ci_compare(const char *a, const char *b)
{
    while (*a != '\0' && *b != '\0') {
        int ca = tolower((unsigned char)*a);
        int cb = tolower((unsigned char)*b);

        if (ca != cb)
            return ca - cb;
        a++;
        b++;
    }
    return tolower((unsigned char)*a) - tolower((unsigned char)*b);
}

void fontlist_init(FontList *list)
{
    list->families = NULL;
    list->family_count = 0;
}

int fontlist_add(FontList *list, const FontFace *face)
{
    FontNameParts parts;
    const char *family = face->family;
    const char *style = face->style;
    FontFamily *fam = NULL;
    FontStyle *styles;
    int i;

    if (family == NULL || family[0] == '\0' || style == NULL || style[0] == '\0') {
        font_split_name(face->full_name, &parts);
        family = parts.family;
        style = parts.style;
    }

    for (i = 0; i < list->family_count; i++) {
        if (ci_compare(list->families[i].name, family) == 0) {
            fam = &list->families[i];
            break;
        }
    }
    if (fam == NULL) {
        FontFamily *grown = realloc(list->families,
                                    (size_t)(list->family_count + 1) * sizeof(FontFamily));

        if (grown == NULL)
            return -1;
        list->families = grown;
        fam = &list->families[list->family_count];
        fam->name = dup_str(family);
        if (fam->name == NULL)
            return -1;
        fam->styles = NULL;
        fam->style_count = 0;
        fam->formats = 0;
        list->family_count++;
    }

    styles = realloc(fam->styles, (size_t)(fam->style_count + 1) * sizeof(FontStyle));
    if (styles == NULL)
        return -1;
    fam->styles = styles;
    styles[fam->style_count].style = dup_str(style);
    if (styles[fam->style_count].style == NULL)
        return -1;
    styles[fam->style_count].id = face->id;
    styles[fam->style_count].format = face->format;
    styles[fam->style_count].flags = face->flags;
    fam->style_count++;
    fam->formats |= face->format;
    return 0;
}

int font_style_rank(const char *style)
{
    int weight = 3;     /* regular */
    int slant = 0;
    const char *p = style;

    while (*p != '\0') {
        const char *start;
        const FontStyleWord *w;

        while (*p == ' ')
            p++;
        if (*p == '\0')
            break;
        start = p;
        while (*p != '\0' && *p != ' ')
            p++;
        w = font_style_word(start, (size_t)(p - start));
        if (w != NULL) {
            if (w->weight >= 0)
                weight = w->weight;
            if (w->slant)
                slant = 1;
        }
    }
    return weight * 2 + slant;
}

static int cmp_family(const void *a, const void *b)
{
    return ci_compare(((const FontFamily *)a)->name, ((const FontFamily *)b)->name);
}

static int cmp_style(const void *a, const void *b)
{
    const FontStyle *sa = a;
    const FontStyle *sb = b;
    int ra = font_style_rank(sa->style);
    int rb = font_style_rank(sb->style);

    if (ra != rb)
        return ra - rb;
    return ci_compare(sa->style, sb->style);
}

void fontlist_sort(FontList *list)
{
    int i;

    if (list->family_count > 1)
        qsort(list->families, (size_t)list->family_count, sizeof(FontFamily), cmp_family);
    for (i = 0; i < list->family_count; i++) {
        FontFamily *fam = &list->families[i];

        if (fam->style_count > 1)
            qsort(fam->styles, (size_t)fam->style_count, sizeof(FontStyle), cmp_style);
    }
}

const FontFamily *fontlist_find(const FontList *list, const char *name)
{
    int i;

    for (i = 0; i < list->family_count; i++) {
        if (ci_compare(list->families[i].name, name) == 0)
            return &list->families[i];
    }
    return NULL;
}

void fontlist_free(FontList *list)
{
    int i, j;

    for (i = 0; i < list->family_count; i++) {
        FontFamily *fam = &list->families[i];

        for (j = 0; j < fam->style_count; j++)
            free(fam->styles[j].style);
        free(fam->styles);
        free(fam->name);
    }
    free(list->families);
    fontlist_init(list);
}
```

- [ ] **Step 7: Add it to the portable sources**

In `Makefile`, replace:

```
LOGIC_SRC += src/fontdb/split.c
```

with:

```
LOGIC_SRC += src/fontdb/split.c
LOGIC_SRC += src/fontdb/group.c
```

- [ ] **Step 8: Run the tests to see them pass**

Run:

```bash
make test
```

Expected: it succeeds and the output matches `d+ checks, 0 failures`.

- [ ] **Step 9: Run them under the address and undefined-behaviour sanitisers**

The list owns heap memory, so a leak or overrun must fail here. Run `make -B test` afterwards to go back to the normal flags.

Run:

```bash
make -B test HOST_FLAGS="-std=gnu99 -Wall -Wextra -Werror -g -Isrc -Itests -fsanitize=address,undefined -fno-sanitize-recover=all"
```

Expected: it succeeds and the output matches `d+ checks, 0 failures`.

- [ ] **Step 10: Commit**

```bash
git add tests src/fontdb Makefile
git commit -m "fontdb: group fonts into families and styles"
git push
```

End the commit message with the attribution lines given in the Workflow section.

---

### Task 4: Decode GDOS and OS facts into text

**Files:**
- Create: `src/gdosinfo/decode.h`, `src/gdosinfo/decode.c`, `tests/test_decode.c`
- Modify: `tests/main.c`, `Makefile`

**Interfaces:**
- Consumes: The host test harness from Task 2.
- Produces: `GDOS_NONE` is `(-2L)`. `const char *gdos_vgdos_name(long value)` and `const char *gdos_fsmc_name(const char magic[4])` never return NULL.
- Produces: `void gdos_format_version(const char *label, unsigned short bcd, char *out, size_t cap)` gives `"NVDI 4.10"` for label `"NVDI"`, `0x0410`.
- Produces: `void gdos_format_fsmc(const char magic[4], short version, short quality, char *out, size_t cap)` gives `"SpeedoGDOS 4.00, quality -1"`.
- Produces: `void gdos_format_os(unsigned short os_version, int is_etos, char *out, size_t cap)` gives `"TOS 2.06"` or `"EmuTOS or pTOS (TOS 2.06 compatible)"`.

- [ ] **Step 1: Declare the interface**

Create `src/gdosinfo/decode.h`:

```c
/*
 * decode.h - turn the raw GDOS and OS facts into text for the About dialog
 *
 * Pure functions: the values come from vq_vgdos(), the FSMC and NVDI cookies
 * and the OS header, read elsewhere.
 */
#ifndef GDOSINFO_DECODE_H
#define GDOSINFO_DECODE_H

#include <stddef.h>

#define GDOS_NONE   (-2L)       /* what vq_vgdos() returns without a GDOS */

/* the GDOS vq_vgdos() reported; never NULL */
const char *gdos_vgdos_name(long value);

/* the vector-font GDOS an FSMC cookie's magic names; never NULL */
const char *gdos_fsmc_name(const char magic[4]);

/* "NVDI 4.10": bcd 0x0410 with a label; also for EdDI and others */
void gdos_format_version(const char *label, unsigned short bcd, char *out, size_t cap);

/* "SpeedoGDOS 4.00, quality -1" from an FSMC cookie's structure */
void gdos_format_fsmc(const char magic[4], short version, short quality,
                      char *out, size_t cap);

/* "TOS 2.06" or "EmuTOS or pTOS (TOS 2.06 compatible)" */
void gdos_format_os(unsigned short os_version, int is_etos, char *out, size_t cap);

#endif
```

- [ ] **Step 2: Write the failing test**

Create `tests/test_decode.c`:

```c
#include "test.h"
#include "gdosinfo/decode.h"

void test_decode(void)
{
    char buf[80];

    CHECK_STR(gdos_vgdos_name(-2L), "no GDOS");
    CHECK_STR(gdos_vgdos_name(0x5F46534DL), "FSMGDOS");
    CHECK_STR(gdos_vgdos_name(0x5F464E54L), "FONTGDOS");
    CHECK_STR(gdos_vgdos_name(0x66564449L), "fVDI");
    CHECK_STR(gdos_vgdos_name(0x0007E88AL), "Atari GDOS 1.1");
    CHECK_STR(gdos_vgdos_name(0x0007E864L), "AMC-GDOS");
    CHECK_STR(gdos_vgdos_name(0x0007E8BAL), "GEMINI special GDOS");
    CHECK_STR(gdos_vgdos_name(0x3E5D0957L), "ttf-gdos");
    CHECK_STR(gdos_vgdos_name(0x00123456L), "GDOS 1.0 or 1.2");

    CHECK_STR(gdos_fsmc_name("_SPD"), "SpeedoGDOS");
    CHECK_STR(gdos_fsmc_name("_FSM"), "FSMGDOS");
    CHECK_STR(gdos_fsmc_name("_FNT"), "FONTGDOS");
    CHECK_STR(gdos_fsmc_name("TTFG"), "ttf-gdos");
    CHECK_STR(gdos_fsmc_name("????"), "unknown vector GDOS");

    gdos_format_version("NVDI", 0x0410, buf, sizeof buf);
    CHECK_STR(buf, "NVDI 4.10");
    gdos_format_version("NVDI", 0x0500, buf, sizeof buf);
    CHECK_STR(buf, "NVDI 5.00");
    gdos_format_version("EdDI", 0x0110, buf, sizeof buf);
    CHECK_STR(buf, "EdDI 1.10");

    gdos_format_fsmc("_SPD", 0x0400, -1, buf, sizeof buf);
    CHECK_STR(buf, "SpeedoGDOS 4.00, quality -1");
    gdos_format_fsmc("_SPD", 0x0500, 2, buf, sizeof buf);
    CHECK_STR(buf, "SpeedoGDOS 5.00, quality 2");

    gdos_format_os(0x0206, 0, buf, sizeof buf);
    CHECK_STR(buf, "TOS 2.06");
    gdos_format_os(0x0206, 1, buf, sizeof buf);
    CHECK_STR(buf, "EmuTOS or pTOS (TOS 2.06 compatible)");

    /* a buffer that is too small is cut, never overrun */
    gdos_format_version("NVDI", 0x0410, buf, 5);
    CHECK_STR(buf, "NVDI");
}
```

- [ ] **Step 3: Run the new test from the runner**

In `tests/main.c`, replace:

```
void test_group(void);
```

with:

```
void test_group(void);
void test_decode(void);
```

and replace:

```
    test_group();
```

with:

```
    test_group();
    test_decode();
```

- [ ] **Step 4: Register the test file**

In `Makefile`, replace:

```
TEST_SRC  += tests/test_group.c
```

with:

```
TEST_SRC  += tests/test_group.c
TEST_SRC  += tests/test_decode.c
```

- [ ] **Step 5: Run the test to see it fail**

Run:

```bash
make test
```

Expected: it FAILS (matching `ndefined`).

- [ ] **Step 6: Write the implementation**

Create `src/gdosinfo/decode.c`:

```c
#include <stdio.h>
#include <string.h>
#include "gdosinfo/decode.h"

const char *gdos_vgdos_name(long value)
{
    switch (value) {
    case GDOS_NONE:     return "no GDOS";
    case 0x0007E88AL:   return "Atari GDOS 1.1";
    case 0x0007E864L:   return "AMC-GDOS";
    case 0x0007E8BAL:   return "GEMINI special GDOS";
    case 0x3E5D0957L:   return "ttf-gdos";
    case 0x5F464E54L:   return "FONTGDOS";
    case 0x5F46534DL:   return "FSMGDOS";
    case 0x66564449L:   return "fVDI";
    default:            return "GDOS 1.0 or 1.2";
    }
}

const char *gdos_fsmc_name(const char magic[4])
{
    if (memcmp(magic, "_SPD", 4) == 0)
        return "SpeedoGDOS";
    if (memcmp(magic, "_FSM", 4) == 0)
        return "FSMGDOS";
    if (memcmp(magic, "_FNT", 4) == 0)
        return "FONTGDOS";
    if (memcmp(magic, "TTFG", 4) == 0)
        return "ttf-gdos";
    return "unknown vector GDOS";
}

void gdos_format_version(const char *label, unsigned short bcd, char *out, size_t cap)
{
    snprintf(out, cap, "%s %d.%02x", label, (int)(bcd >> 8), (unsigned)(bcd & 0xff));
}

void gdos_format_fsmc(const char magic[4], short version, short quality,
                      char *out, size_t cap)
{
    snprintf(out, cap, "%s %d.%02x, quality %d", gdos_fsmc_name(magic),
             (int)((version >> 8) & 0xff), (unsigned)(version & 0xff), (int)quality);
}

void gdos_format_os(unsigned short os_version, int is_etos, char *out, size_t cap)
{
    if (is_etos)
        snprintf(out, cap, "EmuTOS or pTOS (TOS %d.%02x compatible)",
                 (int)(os_version >> 8), (unsigned)(os_version & 0xff));
    else
        snprintf(out, cap, "TOS %d.%02x",
                 (int)(os_version >> 8), (unsigned)(os_version & 0xff));
}
```

- [ ] **Step 7: Add it to the portable sources**

In `Makefile`, replace:

```
LOGIC_SRC += src/fontdb/group.c
```

with:

```
LOGIC_SRC += src/fontdb/group.c
LOGIC_SRC += src/gdosinfo/decode.c
```

- [ ] **Step 8: Run the tests to see them pass**

Run:

```bash
make test
```

Expected: it succeeds and the output matches `d+ checks, 0 failures`.

- [ ] **Step 9: Commit**

```bash
git add tests src/gdosinfo Makefile
git commit -m "gdosinfo: decode vq_vgdos, FSMC, NVDI and OS values into text"
git push
```

End the commit message with the attribution lines given in the Workflow section.

---

### Task 5: Grid, scroll and zoom arithmetic

**Files:**
- Create: `src/layout/layout.h`, `src/layout/layout.c`, `tests/test_layout.c`
- Modify: `tests/main.c`, `Makefile`

**Interfaces:**
- Consumes: The host test harness from Task 2.
- Produces: `ZOOM_MIN_PT` 8, `ZOOM_MAX_PT` 72, `SLIDER_MAX` 1000; `RowRange {int first; int end;}` (`end` is one past the last row).
- Produces: `int layout_columns(int content_w, int tile_w, int gap)` (at least 1), `int layout_rows(int count, int columns)`, `RowRange layout_visible_rows(int offset_y, int view_h, int row_h, int row_count)`.
- Produces: `int zoom_from_slider(int pos, int track)` and `int zoom_to_slider(int pt, int track)`: `pos` 0..`track` <-> 8..72 points, clamped.
- Produces: `int scroll_slider_size(int content_h, int view_h)`, `int scroll_slider_pos(int offset_y, int content_h, int view_h)`, `int scroll_offset_from_slider(int pos, int content_h, int view_h)`: the AES `WF_VSLSIZE` / `WF_VSLIDE` values, 1..1000.

- [ ] **Step 1: Declare the interface**

Create `src/layout/layout.h`:

```c
/*
 * layout.h - grid, scroll and zoom arithmetic, free of any GEM call
 *
 * Positions and sizes are in pixels; the AES slider values (WF_VSLIDE,
 * WF_VSLSIZE) are 1..1000.
 */
#ifndef LAYOUT_LAYOUT_H
#define LAYOUT_LAYOUT_H

#define ZOOM_MIN_PT   8
#define ZOOM_MAX_PT   72
#define SLIDER_MAX    1000

typedef struct {
    int first;      /* first row to draw */
    int end;        /* one past the last row to draw */
} RowRange;

/* how many tiles of tile_w fit across content_w, with gap between; at least 1 */
int layout_columns(int content_w, int tile_w, int gap);

/* rows needed for count items in columns columns */
int layout_rows(int count, int columns);

/* the rows that show in a view_h tall view scrolled offset_y down */
RowRange layout_visible_rows(int offset_y, int view_h, int row_h, int row_count);

/* zoom slider: pos 0..track <-> a size in points, ZOOM_MIN_PT..ZOOM_MAX_PT */
int zoom_from_slider(int pos, int track);
int zoom_to_slider(int pt, int track);

/* window slider values for a content_h tall document in a view_h tall view */
int scroll_slider_size(int content_h, int view_h);
int scroll_slider_pos(int offset_y, int content_h, int view_h);
int scroll_offset_from_slider(int pos, int content_h, int view_h);

#endif
```

- [ ] **Step 2: Write the failing test**

The `layout_columns(280, 64, 8)` / `(279, 64, 8)` pair is the exact-fit boundary: without it a version that forgets the gap passes.

Create `tests/test_layout.c`:

```c
#include "test.h"
#include "layout/layout.h"

void test_layout(void)
{
    RowRange r;
    int pt, pos, prev;

    /* columns: 320 wide, 64 wide tiles, 8 gap: 4 fit (4*64 + 3*8 = 280) */
    CHECK_INT(layout_columns(320, 64, 8), 4);
    CHECK_INT(layout_columns(280, 64, 8), 4);      /* exactly 4 tiles and 3 gaps */
    CHECK_INT(layout_columns(279, 64, 8), 3);
    CHECK_INT(layout_columns(50, 64, 8), 1);
    CHECK_INT(layout_columns(0, 64, 8), 1);
    CHECK_INT(layout_columns(320, 0, 8), 1);
    CHECK_INT(layout_rows(10, 4), 3);
    CHECK_INT(layout_rows(8, 4), 2);
    CHECK_INT(layout_rows(0, 4), 0);
    CHECK_INT(layout_rows(5, 0), 0);

    /* 10 rows of 20 px in a 50 px view */
    r = layout_visible_rows(0, 50, 20, 10);
    CHECK_INT(r.first, 0);
    CHECK_INT(r.end, 3);
    r = layout_visible_rows(30, 50, 20, 10);       /* rows 1 (partly) to 4 */
    CHECK_INT(r.first, 1);
    CHECK_INT(r.end, 4);
    r = layout_visible_rows(190, 50, 20, 10);      /* the end of the list */
    CHECK_INT(r.first, 9);
    CHECK_INT(r.end, 10);
    r = layout_visible_rows(1000, 50, 20, 10);     /* scrolled past the end */
    CHECK_INT(r.first, 10);
    CHECK_INT(r.end, 10);
    r = layout_visible_rows(0, 50, 20, 0);
    CHECK_INT(r.end, 0);
    r = layout_visible_rows(0, 50, 0, 10);
    CHECK_INT(r.end, 0);

    /* zoom: ends, middle, and a round trip at every size */
    CHECK_INT(zoom_from_slider(0, 200), ZOOM_MIN_PT);
    CHECK_INT(zoom_from_slider(200, 200), ZOOM_MAX_PT);
    CHECK_INT(zoom_from_slider(100, 200), 40);
    CHECK_INT(zoom_from_slider(-5, 200), ZOOM_MIN_PT);
    CHECK_INT(zoom_from_slider(999, 200), ZOOM_MAX_PT);
    CHECK_INT(zoom_from_slider(5, 0), ZOOM_MIN_PT);
    CHECK_INT(zoom_to_slider(ZOOM_MIN_PT, 200), 0);
    CHECK_INT(zoom_to_slider(ZOOM_MAX_PT, 200), 200);
    CHECK_INT(zoom_to_slider(1, 200), 0);
    CHECK_INT(zoom_to_slider(500, 200), 200);
    for (pt = ZOOM_MIN_PT; pt <= ZOOM_MAX_PT; pt++)
        CHECK_INT(zoom_from_slider(zoom_to_slider(pt, 200), 200), pt);

    /* a short track (ST-LOW): not every size is reachable, but it never goes down */
    prev = ZOOM_MIN_PT;
    for (pos = 0; pos <= 48; pos++) {
        int z = zoom_from_slider(pos, 48);

        CHECK(z >= prev);
        prev = z;
    }
    CHECK_INT(prev, ZOOM_MAX_PT);

    /* scroll slider: content 1000, view 250 */
    CHECK_INT(scroll_slider_size(1000, 250), 250);
    CHECK_INT(scroll_slider_size(200, 250), SLIDER_MAX);
    CHECK_INT(scroll_slider_size(100000, 1), 1);
    CHECK_INT(scroll_slider_pos(0, 1000, 250), 1);
    CHECK_INT(scroll_slider_pos(750, 1000, 250), SLIDER_MAX);
    CHECK_INT(scroll_slider_pos(9999, 1000, 250), SLIDER_MAX);
    CHECK_INT(scroll_slider_pos(100, 200, 250), 1);
    CHECK_INT(scroll_offset_from_slider(1, 1000, 250), 0);
    CHECK_INT(scroll_offset_from_slider(SLIDER_MAX, 1000, 250), 750);
    CHECK_INT(scroll_offset_from_slider(500, 200, 250), 0);
    /* the slider position and the offset agree to within a pixel step */
    pos = scroll_slider_pos(375, 1000, 250);
    CHECK(scroll_offset_from_slider(pos, 1000, 250) >= 374 - 1);
    CHECK(scroll_offset_from_slider(pos, 1000, 250) <= 375);
}
```

- [ ] **Step 3: Run the new test from the runner**

In `tests/main.c`, replace:

```
void test_decode(void);
```

with:

```
void test_decode(void);
void test_layout(void);
```

and replace:

```
    test_decode();
```

with:

```
    test_decode();
    test_layout();
```

- [ ] **Step 4: Register the test file**

In `Makefile`, replace:

```
TEST_SRC  += tests/test_decode.c
```

with:

```
TEST_SRC  += tests/test_decode.c
TEST_SRC  += tests/test_layout.c
```

- [ ] **Step 5: Run the test to see it fail**

Run:

```bash
make test
```

Expected: it FAILS (matching `ndefined`).

- [ ] **Step 6: Write the implementation**

Create `src/layout/layout.c`:

```c
#include "layout/layout.h"

int layout_columns(int content_w, int tile_w, int gap)
{
    int n;

    if (tile_w <= 0)
        return 1;
    n = (content_w + gap) / (tile_w + gap);
    return n < 1 ? 1 : n;
}

int layout_rows(int count, int columns)
{
    if (columns <= 0 || count <= 0)
        return 0;
    return (count + columns - 1) / columns;
}

RowRange layout_visible_rows(int offset_y, int view_h, int row_h, int row_count)
{
    RowRange r;

    r.first = 0;
    r.end = 0;
    if (row_h <= 0 || row_count <= 0 || view_h <= 0)
        return r;
    if (offset_y < 0)
        offset_y = 0;
    r.first = offset_y / row_h;
    r.end = (int)(((long)offset_y + view_h + row_h - 1) / row_h);
    if (r.end > row_count)
        r.end = row_count;
    if (r.first > r.end)
        r.first = r.end;
    return r;
}

int zoom_from_slider(int pos, int track)
{
    long range = ZOOM_MAX_PT - ZOOM_MIN_PT;

    if (track <= 0 || pos <= 0)
        return ZOOM_MIN_PT;
    if (pos >= track)
        return ZOOM_MAX_PT;
    return ZOOM_MIN_PT + (int)(((long)pos * range + track / 2) / track);
}

int zoom_to_slider(int pt, int track)
{
    long range = ZOOM_MAX_PT - ZOOM_MIN_PT;

    if (track <= 0 || pt <= ZOOM_MIN_PT)
        return 0;
    if (pt >= ZOOM_MAX_PT)
        return track;
    return (int)(((long)(pt - ZOOM_MIN_PT) * track + range / 2) / range);
}

int scroll_slider_size(int content_h, int view_h)
{
    long size;

    if (content_h <= view_h || content_h <= 0)
        return SLIDER_MAX;
    size = (long)view_h * SLIDER_MAX / content_h;
    return size < 1 ? 1 : (int)size;
}

int scroll_slider_pos(int offset_y, int content_h, int view_h)
{
    long max = (long)content_h - view_h;
    long pos;

    if (max <= 0 || offset_y <= 0)
        return 1;
    if (offset_y >= max)
        return SLIDER_MAX;
    pos = 1 + (long)offset_y * (SLIDER_MAX - 1) / max;
    return (int)pos;
}

int scroll_offset_from_slider(int pos, int content_h, int view_h)
{
    long max = (long)content_h - view_h;

    if (max <= 0 || pos <= 1)
        return 0;
    if (pos >= SLIDER_MAX)
        return (int)max;
    return (int)((long)(pos - 1) * max / (SLIDER_MAX - 1));
}
```

- [ ] **Step 7: Add it to the portable sources**

In `Makefile`, replace:

```
LOGIC_SRC += src/gdosinfo/decode.c
```

with:

```
LOGIC_SRC += src/gdosinfo/decode.c
LOGIC_SRC += src/layout/layout.c
```

- [ ] **Step 8: Run the tests to see them pass**

Run:

```bash
make test
```

Expected: it succeeds and the output matches `d+ checks, 0 failures`.

- [ ] **Step 9: Commit**

```bash
git add tests src/layout Makefile
git commit -m "layout: grid, scroll and zoom arithmetic"
git push
```

End the commit message with the attribution lines given in the Workflow section.

---

### Task 6: The VDI seam and building the font list from a source

**Files:**
- Create: `src/vdi_if/vdi_if.h`, `src/fontdb/build.h`, `src/fontdb/build.c`, `tests/test_build.c`
- Modify: `tests/main.c`, `Makefile`

**Interfaces:**
- Consumes: `FontList`, `FontFace`, `fontlist_init/add/sort/free` from Task 3.
- Produces: `VdiFace {short id; unsigned short format, flags; char name[34]; char family[52]; char style[52];}` (strings always NUL-terminated; `family` and `style` are "" when `vqt_xfntinfo()` is not available).
- Produces: `VdiFontSource {int (*face_count)(void *ctx); int (*face_get)(void *ctx, int index, VdiFace *out); void *ctx;}`: `index` is 1..count; `face_get` returns 0, or -1 for no such face.
- Produces: `int fontdb_build(const VdiFontSource *src, FontList *out)`: reads every face, skips those `face_get` refuses, sorts; returns the number added, or -1 if out of memory (then `out` is empty). Free with `fontlist_free()`.

- [ ] **Step 1: Declare the seam**

Create `src/vdi_if/vdi_if.h`:

```c
/*
 * vdi_if.h - the seam between the font list and the VDI
 *
 * fontdb only ever sees fonts through a VdiFontSource, so it is tested on the
 * host with a fake one; vdi_if_gem.c is the real one, over gemlib.
 */
#ifndef VDI_IF_VDI_IF_H
#define VDI_IF_VDI_IF_H

/* one face, as reported by vqt_name(), vqt_ext_name() and vqt_xfntinfo() */
typedef struct {
    short id;
    unsigned short format;      /* FONT_FORMAT_* (see fontdb/group.h) */
    unsigned short flags;       /* vqt_ext_name(): bit 0 monospaced, bit 4 symbol */
    char name[34];              /* full name, always NUL-terminated */
    char family[52];            /* from vqt_xfntinfo(); "" if not available */
    char style[52];             /* from vqt_xfntinfo(); "" if not available */
} VdiFace;

typedef struct {
    int (*face_count)(void *ctx);

    /* index is 1..face_count; 0 on success, -1 if there is no such face */
    int (*face_get)(void *ctx, int index, VdiFace *out);

    void *ctx;
} VdiFontSource;

#endif
```

- [ ] **Step 2: Declare the build function**

Create `src/fontdb/build.h`:

```c
#ifndef FONTDB_BUILD_H
#define FONTDB_BUILD_H

#include "fontdb/group.h"
#include "vdi_if/vdi_if.h"

/*
 * Read every face from src into out, grouped and sorted.  Faces the source
 * cannot give are skipped.  Returns the number of faces added, or -1 if out of
 * memory (out is then empty).  Free out with fontlist_free().
 */
int fontdb_build(const VdiFontSource *src, FontList *out);

#endif
```

- [ ] **Step 3: Write the failing test, with a fake font source**

Create `tests/test_build.c`:

```c
#include <string.h>
#include "test.h"
#include "fontdb/build.h"

typedef struct {
    short id;
    unsigned short format;
    const char *name;
    const char *family;
    const char *style;
    int missing;                /* face_get fails for this one */
} FakeFace;

static const FakeFace fake_faces[] = {
    { 1,  FONT_FORMAT_BITMAP,   "Arial Bold Italic", "", "", 0 },
    { 2,  FONT_FORMAT_BITMAP,   "Arial",             "", "", 0 },
    { 3,  FONT_FORMAT_BITMAP,   "Broken",            "", "", 1 },
    { 4,  FONT_FORMAT_TRUETYPE, "DejaVu Sans Mono",  "DejaVu Sans Mono", "Book", 0 },
    { 5,  FONT_FORMAT_BITMAP,   "Arial Bold",        "", "", 0 },
    { 6,  FONT_FORMAT_BITMAP,   "Dutch 801 Roman",   "", "", 0 }
};

#define FAKE_COUNT ((int)(sizeof fake_faces / sizeof fake_faces[0]))

static int fake_count(void *ctx)
{
    (void)ctx;
    return FAKE_COUNT;
}

static int fake_get(void *ctx, int index, VdiFace *out)
{
    const FakeFace *f;

    (void)ctx;
    if (index < 1 || index > FAKE_COUNT)
        return -1;
    f = &fake_faces[index - 1];
    if (f->missing)
        return -1;
    memset(out, 0, sizeof *out);
    out->id = f->id;
    out->format = f->format;
    strcpy(out->name, f->name);
    strcpy(out->family, f->family);
    strcpy(out->style, f->style);
    return 0;
}

static int no_faces(void *ctx)
{
    (void)ctx;
    return 0;
}

void test_build(void)
{
    VdiFontSource src;
    FontList list;
    const FontFamily *arial;

    src.face_count = fake_count;
    src.face_get = fake_get;
    src.ctx = NULL;

    CHECK_INT(fontdb_build(&src, &list), 5);        /* "Broken" is skipped */
    CHECK_INT(list.family_count, 3);
    CHECK_STR(list.families[0].name, "Arial");
    CHECK_STR(list.families[1].name, "DejaVu Sans Mono");
    CHECK_STR(list.families[2].name, "Dutch 801");

    arial = fontlist_find(&list, "Arial");
    CHECK(arial != NULL);
    if (arial != NULL) {
        CHECK_INT(arial->style_count, 3);
        CHECK_STR(arial->styles[0].style, "Regular");
        CHECK_STR(arial->styles[1].style, "Bold");
        CHECK_STR(arial->styles[2].style, "Bold Italic");
        CHECK_INT(arial->styles[2].id, 1);
    }
    CHECK_STR(list.families[1].styles[0].style, "Book");
    CHECK_INT(list.families[1].formats, FONT_FORMAT_TRUETYPE);
    fontlist_free(&list);

    src.face_count = no_faces;
    CHECK_INT(fontdb_build(&src, &list), 0);
    CHECK_INT(list.family_count, 0);
    fontlist_free(&list);
}
```

- [ ] **Step 4: Run the new test from the runner**

In `tests/main.c`, replace:

```
void test_layout(void);
```

with:

```
void test_layout(void);
void test_build(void);
```

and replace:

```
    test_layout();
```

with:

```
    test_layout();
    test_build();
```

- [ ] **Step 5: Register the test file**

In `Makefile`, replace:

```
TEST_SRC  += tests/test_layout.c
```

with:

```
TEST_SRC  += tests/test_layout.c
TEST_SRC  += tests/test_build.c
```

- [ ] **Step 6: Run the test to see it fail**

Run:

```bash
make test
```

Expected: it FAILS (matching `ndefined`).

- [ ] **Step 7: Write the implementation**

Create `src/fontdb/build.c`:

```c
#include "fontdb/build.h"

int fontdb_build(const VdiFontSource *src, FontList *out)
{
    int count = src->face_count(src->ctx);
    int i;
    int added = 0;
    VdiFace vf;
    FontFace ff;

    fontlist_init(out);
    for (i = 1; i <= count; i++) {
        if (src->face_get(src->ctx, i, &vf) != 0)
            continue;
        ff.id = vf.id;
        ff.format = vf.format;
        ff.flags = vf.flags;
        ff.full_name = vf.name;
        ff.family = vf.family;
        ff.style = vf.style;
        if (fontlist_add(out, &ff) != 0) {
            fontlist_free(out);
            return -1;
        }
        added++;
    }
    fontlist_sort(out);
    return added;
}
```

- [ ] **Step 8: Add it to the portable sources**

In `Makefile`, replace:

```
LOGIC_SRC += src/layout/layout.c
```

with:

```
LOGIC_SRC += src/layout/layout.c
LOGIC_SRC += src/fontdb/build.c
```

- [ ] **Step 9: Run the tests to see them pass**

Run:

```bash
make test
```

Expected: it succeeds and the output matches `d+ checks, 0 failures`.

- [ ] **Step 10: Commit**

```bash
git add tests src/vdi_if src/fontdb Makefile
git commit -m "fontdb: the VDI seam, and building the font list from a source"
git push
```

End the commit message with the attribution lines given in the Workflow section.

---

### Task 7: The real font source, platform code, and an application that lists the fonts

**Files:**
- Create: `src/vdi_if/gem_source.h`, `src/vdi_if/vdi_if_gem.c`
- Create: `src/platform/platform.h`, `src/platform/m68k/platform.c`, `src/platform/generic/platform.c`
- Modify: `src/main.c` (replace), `Makefile`

**Interfaces:**
- Consumes: `fontdb_build()`, `VdiFontSource`, `VdiFace`, `FontList` (Tasks 3 and 6); `gdos_vgdos_name()`, `gdos_format_os()` (Task 4).
- Consumes: gemlib: `appl_init`, `appl_exit`, `form_alert`, `graf_handle`, `v_opnvwk`, `v_clsvwk`, `vq_gdos`, `vq_vgdos`, `vst_load_fonts`, `vqt_name`, `vqt_ext_name`, `vqt_xfntinfo`, `XFNT_INFO`, from `<gem.h>` and `<gemx.h>`.
- Produces: `int gem_source_open(GemSource *gs, VdiFontSource *out)` (0, or -1 if no workstation) and `void gem_source_close(GemSource *gs)`; `GemSource {short handle, count; int have_ext, have_xfnt;}`.
- Produces: `int platform_get_cookie(long id, long *value)` (0 if found, else -1) and `int platform_get_os(unsigned short *os_version, int *is_etos)` (0, or -1 if unreadable). The generic implementation answers -1 to both.
- Produces: `TYPESHLF.PRG` that opens a workstation, lists the fonts and shows an alert with the family and face counts, the GDOS and the OS.

- [ ] **Step 1: Declare the platform interface**

Create `src/platform/platform.h`:

```c
/*
 * platform.h - the few things that differ from one machine to the next
 *
 * One implementation per directory under src/platform/; the Makefile picks
 * it.  The generic one answers "not available" to everything, which is what a
 * target gets until its own is written.
 */
#ifndef PLATFORM_PLATFORM_H
#define PLATFORM_PLATFORM_H

/* 0 and *value set if the cookie is in the jar, -1 if not (or no jar) */
int platform_get_cookie(long id, long *value);

/*
 * The OS header's version word (0x0206 is TOS 2.06) and whether the 'ETOS'
 * marker EmuTOS and pTOS carry is there.  Returns 0 and sets both, or -1 if
 * they cannot be read.
 */
int platform_get_os(unsigned short *os_version, int *is_etos);

#endif
```

- [ ] **Step 2: Write the m68k platform code**

`Ssystem()` is a MiNT call EmuTOS lacks, so this uses `Setexc()` and `Super()`, which every TOS has.

Create `src/platform/m68k/platform.c`:

```c
/*
 * platform.c (m68k) - cookie jar and OS header, with calls every TOS has
 *
 * Ssystem() is a MiNT call EmuTOS lacks, so it is not used here.
 */
#include <mint/osbind.h>
#include "platform/platform.h"

#define P_COOKIES   0x5A0L          /* system variable: the cookie jar */
#define SYSBASE     0x4F2L          /* system variable: the OS header */

int platform_get_cookie(long id, long *value)
{
    /* Setexc() with a vector number below 0x100 would be an exception vector;
     * 0x5A0 / 4 is the system variable's address as a vector number */
    const long *jar = (const long *)Setexc((short)(P_COOKIES / 4), -1L);

    if (jar == 0 || (long)jar == -1L)
        return -1;
    for (; jar[0] != 0; jar += 2) {
        if (jar[0] == id) {
            if (value != 0)
                *value = jar[1];
            return 0;
        }
    }
    return -1;
}

int platform_get_os(unsigned short *os_version, int *is_etos)
{
    const unsigned char *header;
    long ssp = Super(0L);

    /* the first word of the OS header is a branch; os_beg is at 8, the
     * version at 2 and the EmuTOS marker at 0x2C */
    header = *(const unsigned char *const *)SYSBASE;
    header = *(const unsigned char *const *)(header + 8);
    *os_version = (unsigned short)((header[2] << 8) | header[3]);
    *is_etos = header[0x2C] == 'E' && header[0x2D] == 'T' &&
               header[0x2E] == 'O' && header[0x2F] == 'S';

    Super((void *)ssp);
    return 0;
}
```

- [ ] **Step 3: Write the generic platform code**

Create `src/platform/generic/platform.c`:

```c
/* platform.c (generic) - nothing is available; see platform.h */
#include "platform/platform.h"

int platform_get_cookie(long id, long *value)
{
    (void)id;
    (void)value;
    return -1;
}

int platform_get_os(unsigned short *os_version, int *is_etos)
{
    (void)os_version;
    (void)is_etos;
    return -1;
}
```

- [ ] **Step 4: Declare the real font source**

Create `src/vdi_if/gem_source.h`:

```c
/*
 * gem_source.h - the font source over the real VDI (gemlib)
 *
 * Not built on the host: it needs a screen workstation.
 */
#ifndef VDI_IF_GEM_SOURCE_H
#define VDI_IF_GEM_SOURCE_H

#include "vdi_if/vdi_if.h"

typedef struct {
    short handle;       /* virtual screen workstation */
    short count;        /* number of faces */
    int have_ext;       /* vqt_ext_name() answers (NVDI, SpeedoGDOS) */
    int have_xfnt;      /* vqt_xfntinfo() answers (NVDI 3.02 and later) */
} GemSource;

/*
 * Open a virtual workstation, load the GDOS fonts (once) and describe the
 * faces through out.  0 on success, -1 if no workstation could be opened.
 */
int gem_source_open(GemSource *gs, VdiFontSource *out);

void gem_source_close(GemSource *gs);

#endif
```

- [ ] **Step 5: Write the real font source**

Three things here are decided from documentation and the pTOS source, not yet observed on a running system: the face count `work_out[10] + vst_load_fonts()`, the probes for `vqt_ext_name()` and `vqt_xfntinfo()`, and `vq_gdos()` before loading. Checking them on a real machine is the first task of the plan that adds the Hatari smoke test.

Create `src/vdi_if/vdi_if_gem.c`:

```c
#include <string.h>
#include <gem.h>
#include <gemx.h>
#include "fontdb/group.h"
#include "vdi_if/gem_source.h"

#define XFNT_FLAGS_NAMES  0x07  /* full name, family name, style name */

static int gem_face_count(void *ctx)
{
    return ((GemSource *)ctx)->count;
}

/*
 * vqt_ext_name() and vqt_xfntinfo() are NVDI/SpeedoGDOS calls; a VDI without
 * them returns without filling anything in.  So each is probed on face 1 and
 * trusted only if it agrees with vqt_name().
 */
static int probe_ext(const GemSource *gs)
{
    char name[34];
    short format = 0;
    short flags = 0;
    short id1 = vqt_name(gs->handle, 1, name);
    short id2 = vqt_ext_name(gs->handle, 1, name, &format, &flags);

    return id1 > 0 && id1 == id2 && format != 0;
}

static int probe_xfnt(const GemSource *gs)
{
    char name[34];
    XFNT_INFO info;
    short id = vqt_name(gs->handle, 1, name);

    memset(&info, 0, sizeof info);
    return id > 0 &&
           vqt_xfntinfo(gs->handle, XFNT_FLAGS_NAMES, id, 0, &info) == id &&
           info.family_name[0] != '\0';
}

static int gem_face_get(void *ctx, int index, VdiFace *out)
{
    GemSource *gs = ctx;
    char name[34];
    short format = 0;
    short flags = 0;
    short id;

    memset(out, 0, sizeof *out);
    memset(name, 0, sizeof name);
    if (gs->have_ext)
        id = vqt_ext_name(gs->handle, (short)index, name, &format, &flags);
    else
        id = vqt_name(gs->handle, (short)index, name);
    if (id <= 0)
        return -1;

    if (!gs->have_ext)
        format = name[32] != 0 ? FONT_FORMAT_VECTOR : FONT_FORMAT_BITMAP;
    name[32] = '\0';

    out->id = id;
    out->format = (unsigned short)format;
    out->flags = (unsigned short)flags;
    memcpy(out->name, name, 33);

    if (gs->have_xfnt) {
        XFNT_INFO info;

        memset(&info, 0, sizeof info);
        if (vqt_xfntinfo(gs->handle, XFNT_FLAGS_NAMES, id, 0, &info) == id) {
            strncpy(out->family, info.family_name, sizeof out->family - 1);
            strncpy(out->style, info.style_name, sizeof out->style - 1);
        }
    }
    return 0;
}

int gem_source_open(GemSource *gs, VdiFontSource *out)
{
    short work_in[11];
    short work_out[57];
    short cw, ch, cell_w, cell_h;
    short extra = 0;
    int i;

    memset(gs, 0, sizeof *gs);
    gs->handle = graf_handle(&cw, &ch, &cell_w, &cell_h);
    for (i = 0; i < 10; i++)
        work_in[i] = 1;
    work_in[10] = 2;                    /* raster coordinates */
    v_opnvwk(work_in, &gs->handle, work_out);
    if (gs->handle == 0)
        return -1;

    /*
     * Only with a GDOS, and only once: a second call returns 0 until
     * vst_unload_fonts().  work_out[10] is the number of faces without them.
     */
    if (vq_gdos() != 0)
        extra = vst_load_fonts(gs->handle, 0);
    gs->count = (short)(work_out[10] + extra);
    gs->have_ext = probe_ext(gs);
    gs->have_xfnt = probe_xfnt(gs);

    out->face_count = gem_face_count;
    out->face_get = gem_face_get;
    out->ctx = gs;
    return 0;
}

void gem_source_close(GemSource *gs)
{
    if (gs->handle != 0)
        v_clsvwk(gs->handle);
    gs->handle = 0;
}
```

- [ ] **Step 6: Replace the empty application**

Create `src/main.c`:

```c
/*
 * main.c - Type Shelf
 *
 * For now: list the fonts and say what was found.  The windows come next.
 */
#include <stdio.h>
#include <string.h>
#include <gem.h>
#include "fontdb/build.h"
#include "gdosinfo/decode.h"
#include "platform/platform.h"
#include "vdi_if/gem_source.h"

int main(void)
{
    GemSource gs;
    VdiFontSource source;
    FontList list;
    char os_text[64];
    char alert[256];
    unsigned short os_version;
    int is_etos;
    int faces;

    if (appl_init() < 0)
        return 1;

    if (gem_source_open(&gs, &source) != 0) {
        form_alert(1, "[3][Type Shelf|Cannot open a workstation.][OK]");
        appl_exit();
        return 1;
    }

    faces = fontdb_build(&source, &list);
    if (faces < 0) {
        form_alert(1, "[3][Type Shelf|Out of memory.][OK]");
    } else {
        if (platform_get_os(&os_version, &is_etos) == 0)
            gdos_format_os(os_version, is_etos, os_text, sizeof os_text);
        else
            strcpy(os_text, "unknown OS");
        snprintf(alert, sizeof alert, "[1][Type Shelf|%d families, %d faces|%s|%s][OK]",
                 list.family_count, faces, gdos_vgdos_name(vq_vgdos()), os_text);
        form_alert(1, alert);
        fontlist_free(&list);
    }

    gem_source_close(&gs);
    appl_exit();
    return 0;
}
```

- [ ] **Step 7: Build the new sources**

In `Makefile`, replace:

```
GEM_SRC   = src/main.c
M68K_PLATFORM_SRC =
GENERIC_PLATFORM_SRC =
```

with:

```
GEM_SRC   = src/main.c
GEM_SRC  += src/vdi_if/vdi_if_gem.c
M68K_PLATFORM_SRC = src/platform/m68k/platform.c
GENERIC_PLATFORM_SRC = src/platform/generic/platform.c
```

- [ ] **Step 8: Build the PRG**

No warnings are expected.

Run:

```bash
make
```

Expected: it succeeds and the output matches `TYPESHLF.PRG`.

- [ ] **Step 9: Check that it is still a GEMDOS program**

Run:

```bash
od -A n -t x1 -N 2 build/m68k/TYPESHLF.PRG | tr -d ' '
```

Expected: it succeeds and the output matches `601a`.

- [ ] **Step 10: Check the ARM and x32 compiles**

`-Werror` is on here, so a warning from gemlib's headers or ours on those targets fails the check.

Run:

```bash
make check-arm
make check-x86_64
```

Expected: it succeeds and the output matches `vdi_if_gem.c`.

- [ ] **Step 11: Check that the tests still pass**

Run:

```bash
make test
```

Expected: it succeeds and the output matches `d+ checks, 0 failures`.

- [ ] **Step 12: Commit**

```bash
git add src Makefile
git commit -m "Read the fonts through gemlib and show what was found"
git push
```

End the commit message with the attribution lines given in the Workflow section.

---

### Task 8: Continuous integration

**Files:**
- Create: `.github/workflows/build.yml`

**Interfaces:**
- Consumes: The Makefile targets `test`, `all`, `check-arm`, `check-x86_64` from Tasks 1 to 7.
- Produces: A workflow with four jobs (host tests, m68k build with `TYPESHLF.PRG` as an artifact, ARM compile, x32 compile) that runs on pushes to `main` and on pull requests.

- [ ] **Step 1: Create the workflow**

The m68k job pins `ubuntu-22.04`, as pTOS's CI does: the mintelf PPA does not have packages for every Ubuntu release.

Create `.github/workflows/build.yml`:

```yaml
name: Build

on:
  push:
    branches: [main]
  pull_request:

jobs:
  test:
    name: Host unit tests
    runs-on: ubuntu-latest
    steps:
      - uses: actions/checkout@v4
      - name: Run the unit tests
        run: make test

  m68k:
    name: Build TYPESHLF.PRG (m68k)
    # the mintelf PPA has packages for this release; ubuntu-latest may not
    runs-on: ubuntu-22.04
    steps:
      - uses: actions/checkout@v4
        with:
          submodules: recursive
      - name: Install the m68k toolchain
        run: |
          sudo add-apt-repository -y ppa:vriviere/mintelf
          sudo apt-get update -qq
          sudo apt-get install -y cross-mintelf-essential
          pip3 install kconfiglib
      - name: Build
        run: make
      - name: Check that it is a GEMDOS program
        run: |
          test "$(od -A n -t x1 -N 2 build/m68k/TYPESHLF.PRG | tr -d ' ')" = 601a
      - uses: actions/upload-artifact@v4
        with:
          name: TYPESHLF.PRG
          path: build/m68k/TYPESHLF.PRG

  check-arm:
    name: Compile for ARM
    runs-on: ubuntu-latest
    steps:
      - uses: actions/checkout@v4
        with:
          submodules: recursive
      - name: Install the ARM toolchain
        run: |
          sudo apt-get update -qq
          sudo apt-get install -y gcc-arm-none-eabi
      - name: Compile
        run: make check-arm

  check-x86_64:
    name: Compile for x32
    runs-on: ubuntu-latest
    steps:
      - uses: actions/checkout@v4
        with:
          submodules: recursive
      - name: Install the x32 toolchain
        run: |
          sudo apt-get update -qq
          sudo apt-get install -y gcc gcc-multilib libc6-dev-x32
      - name: Compile
        run: make check-x86_64
```

- [ ] **Step 2: Commit and push**

```bash
git add .github
git commit -m "CI: host tests, the m68k build, and the ARM and x32 compile checks"
git push
```

End the commit message with the attribution lines given in the Workflow section.

- [ ] **Step 3: Watch the first run**

Open the PR and check that all four jobs pass. The workflow cannot be run from a laptop, so this is the first time the `-mx32` job (with `gcc-multilib` and `libc6-dev-x32`) and the PPA install are exercised. If the x32 job fails for a missing header, the fix is in its `apt-get install` line; if the m68k job cannot find the PPA's packages, check the Ubuntu release. Fix and push to the same PR.

---

## Done when

- `make test` passes, and passes under `-fsanitize=address,undefined`.
- `make` builds `build/m68k/TYPESHLF.PRG` with no warnings; it starts with `60 1a`.
- `make check-arm` and `make check-x86_64` pass.
- CI is green on the final PR from the epic branch.
- Not done, and the subject of the next plans: nothing has been run on a TOS, EmuTOS or pTOS yet.

## Addendum: what happened after this plan was written

This plan was executed with one subagent per task and a review after each; the
stack is PR #10 to #24. Three things were added or changed afterwards. Where
this addendum and the task text above differ, the repository is right.

- **Task 9, build and link for ARM (issue #19, PR #20).** gemlib gained ARM
  support (gemlib PR #1), so `make arm` now builds libcmini and gemlib for ARM
  and links `build/arm/TYPESHLF.PRG`; it replaces `make check-arm`, and CI has
  an `arm` job instead of `check-arm`. gemlib is pinned at `d3476b6`. Both
  targets copy libcmini's two outputs to `build/<target>/lib/`, because
  libcmini has one `build/` and one `.config` for every configuration. libcmini
  has no `distclean` target (its `clean` fails on a fresh tree), so the rule
  removes its `build/` directory instead.
- **Task 10, review comments (issue #21, PR #22).** Copilot's comments on #10,
  #11, #13, #14 and #16 were checked and fixed, one commit each: a first word
  longer than the buffer is cut, not dropped (`split.c`); BCD versions are
  printed in hex and the OS line fits a `form_alert()` line, so it is now
  `EmuTOS or pTOS (TOS 2.06)` (`decode.c`); the AES slider position is 0..1000
  and the scroll maths cannot overflow 32 bits (`layout.c`; not with `long
  long`, whose division pulls libgcc's soft-float objects into the hard-float
  ARM link); a vector face keeps its format bit when `vqt_ext_name()` reports 0
  (`vdi_if_gem.c`); and the check-x86_64 wording.
- **Task 11, final review (issue #23, PR #24).** The ext probe no longer
  depends on the format being non-zero; the spec and CLAUDE.md say that ARM
  links; and the font model's deferral (below) is recorded.

The code samples in Tasks 2, 3, 4, 5 and 7 show the code as first written. The
repository has the later versions: `join_tokens()` in `split.c`, the
out-of-memory rollback in `fontlist_add()` (Task 3's fix round), `%x.%02x` and
the shorter OS line in `decode.c`, the scroll functions in `layout.c`, and the
format fallback and probe in `vdi_if_gem.c`. "Done when" above mentions
`make check-arm`; read it as `make arm`.

**Additional differences from the spec:**

- The plan text above treats ARM as a compile check; it builds and links (see Task 9). Only x86_64 stays a compile check.
- The font model carries the name, the styles and the OR of their formats.
  The spec's per-family scalable flag follows from the format bits, and the
  bitmap point sizes are not read yet (`vqt_xfntinfo()` is asked for names
  only): they arrive with the views plan, when drawing needs them and a real
  VDI can be asked.

**Deferred, for the next plans:** nothing has run on a TOS, EmuTOS or pTOS yet;
the three assumptions in `vdi_if_gem.c` (the face count, the two probes, and
`vq_gdos()` before loading) are the first thing to check on a real machine.
