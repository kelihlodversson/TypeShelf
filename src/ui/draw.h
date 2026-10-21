/*
 * draw.h - the few VDI drawing helpers the toolbar and the views share
 */
#ifndef UI_DRAW_H
#define UI_DRAW_H

#include <gem.h>
#include "ui/rect.h"

/* clip drawing to r */
void draw_clip(short vh, const Rect *r);

/* a filled box, with a black outline if outline is set (the VDI draws a
 * perimeter in the fill color, so the outline is drawn separately) */
void draw_box(short vh, const Rect *r, int fill_color, int outline);

/* text with its top left at x, y, in the current font and color */
/* a small black triangle pointing down, w wide (odd), its top left at x, y;
 * drawn with lines, so it does not depend on a font's glyphs */
void draw_down_arrow(short vh, int x, int y, int w);

void draw_text(short vh, int x, int y, const char *s);

/* the same for one UCS-2 character, with the VDI in Unicode mode */
void draw_char16(short vh, int x, int y, int code);

/* the width in pixels of one UCS-2 character, in the current font */
int char16_width(short vh, int code);

#endif
