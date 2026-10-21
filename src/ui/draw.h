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
void draw_text(short vh, int x, int y, const char *s);

#endif
