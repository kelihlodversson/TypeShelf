/*
 * layout.h - grid, scroll and zoom arithmetic, free of any GEM call
 *
 * Positions and sizes are in pixels.  The AES slider position (WF_VSLIDE,
 * WM_VSLID) is 0 (very top) to 1000 (very bottom); the slider size
 * (WF_VSLSIZE) is 1 to 1000.
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

/* window slider values for a content_h tall document in a view_h tall view:
 * the size is 1..1000 (1000 when it all fits), the position 0..1000; the
 * arithmetic avoids 32 bit overflow for very tall documents */
int scroll_slider_size(int content_h, int view_h);
int scroll_slider_pos(int offset_y, int content_h, int view_h);
int scroll_offset_from_slider(int pos, int content_h, int view_h);

#endif
