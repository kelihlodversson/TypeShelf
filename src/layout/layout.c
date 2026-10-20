#include "layout/layout.h"

int layout_columns(int content_w, int tile_w, int gap)
{
    int n;

    if (tile_w <= 0)
        return 1;
    n = (content_w + gap) / (tile_w + gap);
    return n < 1 ? 1 : n;
}

int layout_rows(int count, int columns)
{
    if (columns <= 0 || count <= 0)
        return 0;
    return (count + columns - 1) / columns;
}

RowRange layout_visible_rows(int offset_y, int view_h, int row_h, int row_count)
{
    RowRange r;

    r.first = 0;
    r.end = 0;
    if (row_h <= 0 || row_count <= 0 || view_h <= 0)
        return r;
    if (offset_y < 0)
        offset_y = 0;
    r.first = offset_y / row_h;
    r.end = (int)(((long)offset_y + view_h + row_h - 1) / row_h);
    if (r.end > row_count)
        r.end = row_count;
    if (r.first > r.end)
        r.first = r.end;
    return r;
}

int zoom_from_slider(int pos, int track)
{
    long range = ZOOM_MAX_PT - ZOOM_MIN_PT;

    if (track <= 0 || pos <= 0)
        return ZOOM_MIN_PT;
    if (pos >= track)
        return ZOOM_MAX_PT;
    return ZOOM_MIN_PT + (int)(((long)pos * range + track / 2) / track);
}

int zoom_to_slider(int pt, int track)
{
    long range = ZOOM_MAX_PT - ZOOM_MIN_PT;

    if (track <= 0 || pt <= ZOOM_MIN_PT)
        return 0;
    if (pt >= ZOOM_MAX_PT)
        return track;
    return (int)(((long)(pt - ZOOM_MIN_PT) * track + range / 2) / range);
}

int scroll_slider_size(int content_h, int view_h)
{
    long size;

    if (content_h <= view_h || content_h <= 0)
        return SLIDER_MAX;
    size = (long)view_h * SLIDER_MAX / content_h;
    return size < 1 ? 1 : (int)size;
}

/*
 * value * SLIDER_MAX / max for 0 <= value <= max without a product that can
 * overflow 32 bits: halve both until the product fits.  The error is at most
 * one slider step, and only for documents over two million pixels tall.
 */
static long scaled_up(long value, long max)
{
    while (max > 2000000L) {
        value >>= 1;
        max >>= 1;
    }
    return value * SLIDER_MAX / max;
}

int scroll_slider_pos(int offset_y, int content_h, int view_h)
{
    long max = (long)content_h - view_h;

    if (max <= 0 || offset_y <= 0)
        return 0;
    if (offset_y >= max)
        return SLIDER_MAX;
    return (int)scaled_up(offset_y, max);
}

/* pos * max / SLIDER_MAX, exactly, split so that no product exceeds max */
int scroll_offset_from_slider(int pos, int content_h, int view_h)
{
    long max = (long)content_h - view_h;

    if (max <= 0 || pos <= 0)
        return 0;
    if (pos >= SLIDER_MAX)
        return (int)max;
    return (int)((max / SLIDER_MAX) * pos + (max % SLIDER_MAX) * pos / SLIDER_MAX);
}
