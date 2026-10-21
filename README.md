# Type Shelf

A Font Book style font viewer for GEM. It lists the fonts the VDI/GDOS has
installed, grouped by family and style, shows them as icons, samples or a list,
installs new fonts, and reports which VDI/GDOS the machine has.

It targets m68k first (TOS, EmuTOS and [pTOS](https://github.com/kelihlodversson/pTOS)),
built with [gemlib](https://github.com/kelihlodversson/gemlib) and
[libcmini](https://github.com/kelihlodversson/libcmini). ARM builds and links,
and x86 is compile-checked, until gemlib and pTOS can run a GEM application
there.

![The Icons view: a tile for each installed family, the system font and four TrueType fonts](docs/images/screenshot-icons.png)

![The Specimen view of Arial: a preview at the zoom size, then the font's Unicode characters](docs/images/screenshot-specimen.png)

Both on pTOS (the scalable-fonts branch) under QEMU `virt-arm`, in 32-bit colour.

Run `TYPESHLF.PRG` with `TYPESHLF.RSC` (the toolbar's icons) in the same folder.

The font list, the GDOS and OS decoding, the layout maths, a real font source
over gemlib and CI are in place, and so are the menu bar, windows that follow
the AES (redraw, move, resize, full, close), the toolbar with its zoom slider,
and the three overview views: Icons (a tile per family), Samples (a pangram in
each family) and List (a tree of families that expand to their styles).  Click
or use the arrow keys to select, double-click or Return to open a family: its
Specimen shows a preview at the zoom size and the font's characters, with a
style pulldown (or Left and Right) to pick the style, and its Samples show a
card per style.  Back or Esc returns, and the window's slider scrolls.  Install
Font and the full About dialog are still to come.  The design is in
[docs/superpowers/specs/2026-10-20-typeshelf-design.md](docs/superpowers/specs/2026-10-20-typeshelf-design.md).

Licence: GPL-2.0-or-later.
