/*
 * viewmodel.h - what the overview views lay out, free of any GEM call
 *
 * All positions are pixels in the view's own coordinates: 0, 0 is the top
 * left of the content before it is scrolled.
 */
#ifndef UI_VIEWMODEL_H
#define UI_VIEWMODEL_H

#include <stddef.h>
#include "fontdb/group.h"
#include "ui/rect.h"
#include "ui/winstate.h"

/* ---- Icons: a grid of tiles ---- */

typedef struct {
    int tile_w, tile_h;
    int gap;                /* between tiles, and round the edge */
    int cols, rows;
    int count;
} Grid;

void grid_make(Grid *g, int content_w, int tile_w, int tile_h, int gap, int count);

/* the height of the whole grid */
int grid_height(const Grid *g);

/* the tile's rectangle; index 0 .. count - 1 */
Rect grid_tile(const Grid *g, int index);

/* the tile at a point, or -1 (in a gap, or past the last tile) */
int grid_hit(const Grid *g, int x, int y);

/* move the selection by columns and rows, staying on the grid; from -1 it
 * goes to the first tile.  A move that would leave the grid stays put, except
 * that down from the last full row goes to the last tile. */
int grid_move(const Grid *g, int current, int dcol, int drow);

/* ---- Samples and List: a column of equal rows ---- */

/* the height of count rows of row_h with gap between and round them */
int vlist_height(int row_h, int gap, int count);

/* the top of row index */
int vlist_top(int row_h, int gap, int index);

/* the row at y, or -1 */
int vlist_hit(int row_h, int gap, int count, int y);

/* ---- scrolling ---- */

/* the scroll offset that brings the span top .. top + h into a view_h tall
 * view, moving as little as possible from scroll_y; never below 0 */
int scroll_to_show(int scroll_y, int view_h, int top, int h);

/* scroll_y kept inside 0 .. max(0, content_h - view_h) */
int scroll_clamp(int scroll_y, int content_h, int view_h);

/* ---- List: a tree of families that expand to styles ---- */

typedef struct {
    int family;
    int style;              /* -1 for the family's own row */
} ListRow;

/* expanded[i] is nonzero when family i shows its styles; NULL means none do */
int list_row_count(const FontList *list, const unsigned char *expanded);
ListRow list_row(const FontList *list, const unsigned char *expanded, int n);
int list_row_of_family(const FontList *list, const unsigned char *expanded, int family);

/* ---- text ---- */

/* the style whose weight is nearest Regular, the one a family is shown in */
int family_default_style(const FontFamily *family);

/* "Bitmap", "TrueType, Type 1"; "-" for none */
void font_format_text(unsigned short formats, char *out, size_t cap);

/* "1 style", "3 styles" */
void style_count_text(int count, char *out, size_t cap);

/* the name to show: the system font's faces are named for one size ("6x6
 * system font"), but all its sizes belong to it */
const char *family_display_name(const FontFamily *family, int sys_font);

/* the window title for what the window shows: "TypeShelf: All Fonts",
 * "TypeShelf: Andale Mono, 4 styles" or "TypeShelf: Andale Mono Italic" */
void window_title_text(const FontList *list, const WinState *ws, int sys_font, char *out, size_t cap);

/* the line the Samples view shows at the zoom size */
extern const char sample_pangram[];

/* the Specimen's preview: three lines, each wrapped to the window's width */
#define PREVIEW_LINES 3
extern const char *const specimen_preview[PREVIEW_LINES];

/* ---- Specimen: the repertoire ---- */

/* the characters of the repertoire grid: 33 to 126 and 128 to 255 (127 is
 * DEL, 0 to 32 are controls and the space) */
int repertoire_count(void);
int repertoire_char(int index);        /* the character code, or -1 out of range */

/* the style after moving by delta in a family of count styles, staying inside */
int style_step(int current, int delta, int count);

#endif
