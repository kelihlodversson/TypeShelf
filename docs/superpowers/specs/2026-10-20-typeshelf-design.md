# Type Shelf — design

A Font Book style font viewer for GEM. It lists the fonts the VDI/GDOS has
installed, grouped by family and style, shows them in several views, installs
new fonts, and reports which VDI/GDOS the machine has.

Status: approved design, 2026-10-20. Nothing is implemented yet.

## Goals

- Browse every installed font, grouped into families with their styles.
- Three overview views (Icons, Samples, List) and two single-font views
  (Specimen, Samples), with a zoom slider.
- Install a font file into the system fonts folder and reload.
- An About dialog that shows the OS, the GDOS/VDI variant and version, the
  screen, and the font counts.
- A native GEM application: m68k (runs on TOS, EmuTOS and pTOS), with ARM and
  x86 compile-checked.
- Usable down to ST-LOW (320x200).

## Non-goals (for v1)

- Search (a later version).
- Removing or disabling fonts.
- Printing, and any printer/GDOS driver handling.
- Running on ARM or x86. See "Targets".

## Targets

| Target | v1 | Why |
| --- | --- | --- |
| m68k | Built, runs, smoke-tested under Hatari | gemlib's trap layer exists for m68k |
| ARM | Compile-check only | gemlib has no ARM trap layer; pTOS program entry (#387, #388) is open and libcmini ARM is untested end to end |
| x86_64 (x32) | Compile-check only | gemlib has no x32 layer; pTOS refuses trap class 2 (VDI, AES) from ring 3 (`doc/x86_64-syscall.txt`), the AES is not in ring 3, and the native VDI call contract (#427) is open |

When a gemlib port and the pTOS side exist, the compile check becomes a link
and a run. The source must stay portable until then (see "Portability").

## Scale and screens

- Minimum: ST-LOW (320x200). The window opens full-screen; the toolbar and the
  grids take their sizes from the screen width.
- The look is black and white with fill patterns. Nothing depends on colour.
- Chrome (toolbar, labels) uses the system font; only the previews use the
  font being shown.
- Toolbar buttons are monochrome bitmaps compiled in, in 16x16 and 8x8 sizes.

## Application structure

| Module | Job | Depends on |
| --- | --- | --- |
| `app` | startup, menu bar, `evnt_multi` loop, About dialog | all below |
| `window` | per-window state: mode, view, zoom, scroll, selection | `toolbar`, `views`, `fontdb` |
| `toolbar` | AES `OBJECT` tree built in C, drawn clipped below the title bar | gemlib |
| `views/*` | icons, samples, list, specimen, family_samples; draw only what is visible | `fontdb` |
| `fontdb` | families, styles, per-font metrics cache | `vdi_if` |
| `gdosinfo` | OS, GDOS and screen facts for About | `platform` |
| `install` | Install Font | `fontdb`, `platform` |
| `platform/*` | cookies, file copy, process; the only arch-specific code | OS |
| `vdi_if` | the seam between pure logic and VDI calls | gemlib |

The toolbar trees are built in code, not loaded from an `.rsc` file: RSC files
are big-endian m68k data and would need byte-swapping on ARM and x86, and
building them lets layout depend on the screen size.

`File ▸ New Window` creates another window; each has its own state.

## Windows and views

The toolbar is a strip below the title bar, inside the window's work area. The
content area below it is redrawn through the AES rectangle list and scrolls
with the window's slider.

### Overview (all families)

| View | Menu | Shows |
| --- | --- | --- |
| Icons | `View ▸ Show as Icons ^1` | a tile per family: "Aa" at the zoom size, style count, family name |
| Samples | `View ▸ Show as Samples ^2` | a card per family: name, style count, a pangram at the zoom size, style label |
| List | `View ▸ Show as List ^3` | a tree: family rows expand to styles; columns Typeface, Styles, Format |

Toolbar: three view buttons and the zoom slider.

### Single font (one family)

Entered by double-click or Return on a tile, card or row. The Back button (and
Esc) returns to the overview.

| View | Menu | Shows |
| --- | --- | --- |
| Specimen | `View ▸ Show as Specimen ^1` | a Preview (alphabet, lower case, digits) at the zoom size, then a Repertoire grid of characters |
| Samples | `View ▸ Show as Samples ^2` | a card per style: a pangram at the zoom size, style label |

`Show as List` is greyed out. Toolbar: Back, the style pulldown, two view
buttons, and the zoom slider.

The style pulldown is enabled in Specimen (it picks the style shown) and
disabled in Samples (all styles are shown). This follows the reference
screenshots.

### Menus

- Desk: About Type Shelf…
- File: New Window ^N, Install Font… ^O, Close, Quit ^Q
- View: as above

### Zoom

A custom slider (a box in a box, like a scroll bar). It maps to a font size in
points, 8 to 72, so a window shows the same sizes on every screen.
Bitmap fonts show the nearest smaller size that exists, as `vst_point` does.

## Font model

A `FontFamily` has a name, a list of styles, format flags, whether it is
scalable, and, for bitmap fonts, the available point sizes. Each style has a
font id and its name.

### Building the list

At startup, and again after Install:

1. Open a virtual workstation. Call `vst_load_fonts()` once; pTOS's built-in
   GDOS returns 0 on a second call. The number of faces is `work_out[10]` plus
   the number loaded.
2. For each index call `vqt_ext_name()` where it exists, otherwise
   `vqt_name()`. This gives the id, the name, the format bits (1 bitmap, 2
   Speedo, 4 TrueType, 8 Type-1) and the monospaced, symbol and vector flags.
3. For family and style, call `vqt_xfntinfo()` where it answers (NVDI 3.02 and
   later). It gives the family name, the style name, the file names and the
   point sizes.
4. Without it, split the name at known style words (Bold, Italic, Oblique,
   Light, Medium, Black, Condensed, and so on). A face with no style word
   becomes a family with one style, Regular.
5. Never call `vqt_fontheader()`: it can cause disk accesses on every call.
   Per-font metrics are fetched lazily for what is visible and cached.

### Drawing at a size

- Scalable fonts: `vst_arbpt()` or `vst_arbpt32()` (FSM and Speedo 4.00 and
  later, NVDI 3 and later).
- Bitmap fonts: `vst_point()`.
- Repertoire: the first and last character from `vqt_fontinfo()`;
  `vst_map_mode()` and `vqt_char_index()` for Unicode mapping where available.

## GDOS and system information (About dialog)

| Fact | Source |
| --- | --- |
| OS | TOS version from the OS header; `'ETOS'` at `os_beg + 0x2C` for EmuTOS or pTOS |
| GDOS variant | `vq_vgdos()`: -2 none, `'_FSM'` FSMGDOS, `'_FNT'` FONTGDOS, `0x0007E88A` Atari GDOS 1.1, `'fVDI'`, others |
| Vector-font GDOS | `FSMC` cookie: `{magic, version, quality}`; magic `'_SPD'` SpeedoGDOS, `'_FSM'`, `'_FNT'`, `'TTFG'` ttf-gdos |
| NVDI | `NVDI` cookie, BCD version (`0x0410` is 4.10) |
| EdDI | `EdDI` cookie, version from its dispatcher |
| Screen | planes and resolution from `vq_extnd()` |
| Fonts | bitmap and scalable counts from `fontdb` |

pTOS's built-in GDOS (#591) answers `vq_gdos()` with `'_FSM'` and sets `FSMC`
to `'_SPD'`, 4.00, quality -1. Scalable font ids start at 10000, and
`vqt_name()` byte 33 flags outline fonts.

Open item: an explicit "pTOS built-in GDOS" label needs a reliable
identifier. None has been found yet; until one is, About shows the raw facts
and the decoded name.

## Install Font

1. A file selector picks the font file.
2. The destination folder: `path=` from `ASSIGN.SYS` when a disk GDOS is
   present, otherwise pTOS's default `\GEMSYS\FONTS` (`CONF_VDI_FONT_DIR`).
3. Show the destination and ask for confirmation.
4. Copy the file.
5. Reload with `vst_unload_fonts()` then `vst_load_fonts()`, and rebuild the
   list.

Risk: whether that unload and load actually rescans the directory in pTOS's
built-in GDOS depends on pTOS #576 and #578. If it does not, Install copies the
file and tells the user to restart.

## Repository and build

```
TypeShelf/
  Makefile                 make TARGET=m68k|arm|x86_64
  CLAUDE.md                workflow and conventions
  third_party/gemlib/      submodule: kelihlodversson/gemlib, pinned
  third_party/libcmini/    submodule: kelihlodversson/libcmini, pinned
  src/                     app, window, toolbar, fontdb, gdosinfo, install
  src/views/
  src/platform/{m68k,arm,x86_64}/
  tests/                   host unit tests
  docs/superpowers/specs/
  .github/workflows/build.yml
```

- **m68k:** `make TARGET=m68k` builds libcmini and gemlib from the submodules,
  then the app: 68000, 32-bit `int`, `-nostdlib`, linked against `crt0.o`,
  `-lcmini -lgem -lgcc -lcmini`. Output `TYPESHLF.PRG`. No MiNT features.
- **ARM, x86_64:** CI compiles `src/` with `arm-none-eabi-` and with `-mx32`
  and does not link, until gemlib has their trap layers.
- **Toolchains:** installed as in pTOS's `toolchain-install` skill; CI uses the
  same packages as pTOS CI.

Open build question, a spike before anything depends on it: libcmini's
`COMPILE_ELF` builds ELF with the mintelf toolchain, and real TOS cannot load
ELF. Confirm that the toolchain can produce a GEMDOS PRG, or fall back to the
a.out `m68k-atari-mint-` toolchain. Verify by booting the result in Hatari.
libcmini, gemlib and the app must all use the same format.

### Portability

- Fixed-width types for anything that crosses the AES and VDI arrays.
- No m68k assumptions outside `src/platform/m68k/`.
- `int` is 32 bits on every target; pointers are 32 bits on m68k, ARM and x32.

## Testing

- **Host unit tests** (native gcc, in CI): splitting names into family and
  style, grouping and sorting, tile-grid and scroll maths, zoom mapping, and
  decoding raw `vq_vgdos`, `FSMC` and `NVDI` values into names. A fake font
  source feeds `fontdb` through `vdi_if`.
- **Integration:** the PRG under Hatari, on the EmuTOS image and on a pTOS m68k
  image, in ST-LOW, ST-MEDIUM and ST-HIGH (see the pTOS `ptos-smoketest`
  skill). It checks that the app launches, opens a window, switches views and
  quits cleanly, and uploads a screenshot of each view as a CI artifact.
- Pixel-exact screenshot comparison is left for later, after the UI settles.

## Workflow

As in pTOS. Every change starts from a GitHub issue, then a branch
`{type}/{issue}-{title}`, an empty "Start work on #N" commit, and a draft PR
before real work. The owner merges.

Planned issues, in order:

1. Scaffold, build the libraries, and the PRG spike.
2. `fontdb` and `gdosinfo`, with host tests.
3. Window and toolbar framework.
4. Overview views, then single-font views.
5. Install Font, then About.
6. CI and Hatari smoke tests.

## Dependencies on pTOS

| Needed for | pTOS issue |
| --- | --- |
| Scalable font calls (`vst_arbpt`, `vqt_xfntinfo`, …) | #577 |
| Selecting and drawing scalable fonts | #576 |
| Font folder and default font set | #578 |
| Epic | #573, #585 |
| ARM program entry and libcmini startup | #387, #388 |
| Native x32 VDI call contract | #427 |

## Licence

GPL-2.0-or-later, as pTOS and EmuTOS.
