/*
 * layout.h - grid, scroll and zoom arithmetic, free of any GEM call
 *
 * Positions and sizes are in pixels; the AES slider values (WF_VSLIDE,
 * WF_VSLSIZE) are 1..1000.
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

/* window slider values for a content_h tall document in a view_h tall view */
int scroll_slider_size(int content_h, int view_h);
int scroll_slider_pos(int offset_y, int content_h, int view_h);
int scroll_offset_from_slider(int pos, int content_h, int view_h);

#endif
