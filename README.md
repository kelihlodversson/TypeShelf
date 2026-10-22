# Type Shelf

A Font Book style font viewer for GEM. It lists the fonts the VDI/GDOS has
installed, grouped by family and style, shows them as icons, samples or a list,
installs new scalable fonts, and reports which OS and VDI/GDOS the machine has.

It runs on m68k (TOS, EmuTOS and [pTOS](https://github.com/kelihlodversson/pTOS)) and on pTOS for
ARM (tested under QEMU `virt-arm`), and works from 320x200 (ST low) up.  It is built with
[gemlib](https://github.com/kelihlodversson/gemlib) and
[libcmini](https://github.com/kelihlodversson/libcmini); for x86 the sources are only compile-checked
(x32), until pTOS can run a GEM application there.  The design is in
[docs/superpowers/specs/2026-10-20-typeshelf-design.md](docs/superpowers/specs/2026-10-20-typeshelf-design.md).

Run `TYPESHLF.PRG` with `TYPESHLF.RSC` (the toolbar's icons) in the same folder.

## Using it

**Windows.** File ▸ New Window opens another window (each has its own view and selection); they
redraw, move, resize, go full and close as the AES says.  The toolbar has Back, the three view
buttons, the style pulldown and a zoom slider (8 to 72 points).  Menu shortcuts: Control+N new
window, Control+O Install Font, Control+W close, Control+Q quit, Control+1/2/3 the views.

**Overview** (all fonts), as Icons (a tile per family), Samples (a pangram in each family) or List
(a table of families that expand to their styles).  Click or use the arrow keys to select;
double-click or Return opens a family.

**A family** as Specimen (a preview at the zoom size and the font's characters, in Unicode where
the VDI takes Unicode text) or Samples (a card per style).  The style pulldown, or Left and
Right, picks the style in the Specimen; Back or Esc returns to the overview.

**About** (Desk menu): Type Shelf's version, the OS, the GDOS and its vector-font variant, the
screen, the font counts and the features the VDI has (arbitrary point sizes, Unicode text).  The
dialog has the 3D look where the AES has 3D objects.

**Install Font** (File menu) copies a TrueType or OpenType font (`.TTF`, `.OTF`, `.TTC`) into the
fonts folder and reloads the list:
- the folder is `\GEMSYS\FONTS` on the boot drive for pTOS's built-in GDOS, or the `path=` of
  `ASSIGN.SYS` for a GDOS from disk (which also takes `.SPD`).  A SpeedoGDOS 4.00 from disk has the
  cookie of pTOS's built-in GDOS, so a GDOS with an `ASSIGN.SYS` on the boot drive is taken for the
  disk one.  FONTGDOS has bitmap fonts only and does not count;
- the item is greyed out when the GDOS has no scalable fonts; bitmap fonts are not installed;
- you confirm the destination first, and again before a font is replaced.  A replacement is copied
  to a scratch file (`TSnnn.TMP`) and swapped in, so a failure keeps the old font;
- the list is reloaded with `vst_unload_fonts()` and `vst_load_fonts()`.  pTOS scans the folder
  again then if built with `CONF_VDI_FONT_RESCAN` (the default); otherwise, or with another GDOS,
  the font may show up only after a restart, which Type Shelf then suggests.  Fonts that another
  program installs show up too (the AES message `FNT_CHANGED`).

Not done: search, removing fonts, printing, and installing bitmap fonts.

## Building

    make test             host unit tests (native compiler)
    make                  build/m68k/TYPESHLF.PRG and TYPESHLF.RSC (keep them together)
    make arm              build/arm/TYPESHLF.PRG and TYPESHLF.RSC
    make check-x86_64     compile the sources for x32 (no link)

On macOS use `gmake`.  It needs the submodules (`git submodule update --init`), python3 and rsync,
and the toolchains in [docs/toolchains.md](docs/toolchains.md).  `make VERSION=1.2.3` sets the
version About shows (`dev` otherwise).

## Releases

Download a zip for m68k (`TypeShelf-<version>-m68k.zip`, runs on TOS, EmuTOS and pTOS) or ARM
(`...-arm.zip`, for pTOS on ARM) from the [releases page](../../releases); keep `TYPESHLF.PRG` and
`TYPESHLF.RSC` together.  A release is made by starting the *Release* workflow (Actions > Release >
Run workflow) with a version such as `1.2.3` or `1.2.3-rc.1` (at most 24 characters, so that About shows it whole): it builds both zips with the version in the About
dialog, tags the commit `v1.2.3` and publishes the release, with notes generated from the pull
requests merged since the last one (`.github/release.yml` groups them by label: `enhancement`,
`bug`, `documentation`; `skip-changelog` leaves one out).  A build of your own says `dev`;
`make VERSION=1.2.3` sets another.

## Screenshots

On the left: pTOS (master, with its built-in scalable fonts) under QEMU `virt-arm`, in 32-bit colour, with the system font and four TrueType fonts (Arial with Regular, Italic and Bold).

On the right: a legacy set-up, EmuTOS under Hatari in ST mono (640x400) with the 1987 Atari GDOS and its bitmap fonts (Dutch, Swiss and Typewriter, from the Microsoft Write distribution's GDOS disk).

**Icons**: a tile for each family, with its name and number of styles.

<table><tr>
<td><img src="docs/images/screenshot-icons.png" alt="The Icons view on pTOS"></td>
<td><img src="docs/images/legacy-icons.png" alt="The Icons view on EmuTOS with Atari GDOS"></td>
</tr></table>

**Samples**: a pangram in each family.

<table><tr>
<td><img src="docs/images/screenshot-samples.png" alt="The Samples view on pTOS"></td>
<td><img src="docs/images/legacy-samples.png" alt="The Samples view on EmuTOS with Atari GDOS"></td>
</tr></table>

**List**: a table of families with their number of styles and format; each expands to its styles.

<table><tr>
<td><img src="docs/images/screenshot-list.png" alt="The List view on pTOS"></td>
<td><img src="docs/images/legacy-list.png" alt="The List view on EmuTOS with Atari GDOS"></td>
</tr></table>

**Specimen**: one font, with a preview at the zoom size and the font's characters.

<table><tr>
<td><img src="docs/images/screenshot-specimen.png" alt="The Specimen view on pTOS"></td>
<td><img src="docs/images/legacy-specimen.png" alt="The Specimen view on EmuTOS with Atari GDOS"></td>
</tr></table>

**A family's Samples**: a card for each style.

<table><tr>
<td><img src="docs/images/screenshot-family-samples.png" alt="The Samples of one family on pTOS"></td>
<td><img src="docs/images/legacy-family.png" alt="The Samples of one family on EmuTOS with Atari GDOS"></td>
</tr></table>

**About**: the OS, the GDOS and its vector-font variant, the screen, and the font counts and features.

<table><tr>
<td><img src="docs/images/screenshot-about.png" alt="The About dialog on pTOS"></td>
<td><img src="docs/images/legacy-about.png" alt="The About dialog on EmuTOS with Atari GDOS"></td>
</tr></table>

Licence: GPL-2.0-or-later.
