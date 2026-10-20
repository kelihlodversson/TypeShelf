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
    make arm              build/arm/TYPESHLF.elf (links; not yet run on pTOS)
    make check-x86_64     compile the portable sources for x32 (no link)
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
