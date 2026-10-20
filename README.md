# Type Shelf

A Font Book style font viewer for GEM. It lists the fonts the VDI/GDOS has
installed, grouped by family and style, shows them as icons, samples or a list,
installs new fonts, and reports which VDI/GDOS the machine has.

It targets m68k first (TOS, EmuTOS and [pTOS](https://github.com/kelihlodversson/pTOS)),
built with [gemlib](https://github.com/kelihlodversson/gemlib) and
[libcmini](https://github.com/kelihlodversson/libcmini). ARM builds and links,
and x86 is compile-checked, until gemlib and pTOS can run a GEM application
there.

![Type Shelf's window on EmuTOS: the menu bar, the toolbar with the three view buttons and the zoom slider, and a placeholder where the views will go](docs/images/screenshot-emutos-low.png)

*Today's build under Hatari on pTOS (m68k, ST low, 320x200): the menu bar, a
window with its toolbar (Icons, Samples and List buttons and the zoom slider),
and a placeholder where the views will go.*

Run `TYPESHLF.PRG` with `TYPESHLF.RSC` (the toolbar's icons) in the same folder.

The foundation is in place: the font list, the GDOS and OS decoding, the layout
maths, a real font source over gemlib, and CI.  On top of it are the menu bar
(New Window, Close, Quit, the View entries, About), windows that follow the
AES (redraw, move, resize, full, close) and the toolbar with its zoom slider.
The views, Install Font and the full About dialog are still to come.  The
design is in
[docs/superpowers/specs/2026-10-20-typeshelf-design.md](docs/superpowers/specs/2026-10-20-typeshelf-design.md).

Licence: GPL-2.0-or-later.
