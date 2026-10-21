/*
 * toolbar_layout.h - where the toolbar's parts are, free of any GEM call
 *
 * Positions are in pixels, relative to the toolbar's top left corner.
 */
#ifndef UI_TOOLBAR_LAYOUT_H
#define UI_TOOLBAR_LAYOUT_H

#include "ui/rect.h"
#include "ui/winstate.h"

typedef enum {
    TB_NONE = -1,
    TB_BACK,
    TB_STYLE,       /* the style pulldown */
    TB_VIEW1,
    TB_VIEW2,
    TB_VIEW3,
    TB_SLIDER,      /* the zoom slider's track */
    TB_COUNT
} ToolbarItem;

typedef struct {
    int height;             /* the whole toolbar, in pixels */
    int icon;               /* 8 or 16: the size of the bitmaps */
    int present[TB_COUNT];
    Rect rect[TB_COUNT];
} ToolbarLayout;

#define TB_THUMB_W  8

/* Lay the toolbar out for a work area work_w wide; the zoom slider is at
 * most 160 pixels wide and right aligned.  char_w is the width of
 * the system font's cell; the style pulldown is 12 cells wide. */
void toolbar_layout(ToolbarLayout *tl, int work_w, int char_w, WinMode mode);

/* How many characters of a style name fit in the pulldown (at most 10, in the full 12 cell pulldown): its text
 * starts 3 pixels in and the arrow, a cell wide, sits 2 pixels from the right
 * edge with 2 pixels clear of it. */
int toolbar_style_chars(const ToolbarLayout *tl, int char_w);

/* the item at the point, or TB_NONE */
ToolbarItem toolbar_hit(const ToolbarLayout *tl, int x, int y);

/* the slider's thumb for a zoom in points */
Rect toolbar_thumb(const ToolbarLayout *tl, int pt);

/* the zoom in points for an x position in the slider's track; clamps */
int toolbar_zoom_at(const ToolbarLayout *tl, int x);

#endif
