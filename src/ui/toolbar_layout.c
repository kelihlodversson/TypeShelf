#include <string.h>
#include "layout/layout.h"
#include "ui/toolbar_layout.h"

#define MARGIN      4       /* left and right */
#define PAD         2       /* around a bitmap inside its button */
#define GAP         2       /* between buttons */
#define GROUP_GAP   6       /* between groups of controls */
#define MIN_TRACK   24

static void place(ToolbarLayout *tl, ToolbarItem item, int x, int y, int w, int h)
{
    tl->present[item] = 1;
    tl->rect[item].x = x;
    tl->rect[item].y = y;
    tl->rect[item].w = w;
    tl->rect[item].h = h;
}

void toolbar_layout(ToolbarLayout *tl, int work_w, int char_w, WinMode mode)
{
    int bw, x, track_w;
    ToolbarItem v;

    memset(tl, 0, sizeof *tl);
    tl->icon = work_w >= 240 ? 16 : 8;
    bw = tl->icon + 2 * PAD;
    tl->height = bw + 4;
    x = MARGIN;

    if (mode == MODE_SINGLE) {
        place(tl, TB_BACK, x, 2, bw, bw);
        x += bw + GAP;
        place(tl, TB_STYLE, x, 2, 12 * char_w + 2 * PAD, bw);
        x += tl->rect[TB_STYLE].w + GROUP_GAP;
    }
    for (v = TB_VIEW1; v <= (mode == MODE_SINGLE ? TB_VIEW2 : TB_VIEW3); v++) {
        place(tl, v, x, 2, bw, bw);
        x += bw + GAP;
    }
    x += GROUP_GAP - GAP;

    track_w = work_w - MARGIN - x;
    if (track_w >= MIN_TRACK)
        place(tl, TB_SLIDER, x, 2 + (bw - 10) / 2, track_w, 10);
}

ToolbarItem toolbar_hit(const ToolbarLayout *tl, int x, int y)
{
    int i;

    for (i = 0; i < TB_COUNT; i++)
        if (tl->present[i] && rect_contains(&tl->rect[i], x, y))
            return (ToolbarItem)i;
    return TB_NONE;
}

/* the thumb's travel inside the track's border */
static int travel(const ToolbarLayout *tl)
{
    return tl->rect[TB_SLIDER].w - 2 - TB_THUMB_W;
}

Rect toolbar_thumb(const ToolbarLayout *tl, int pt)
{
    Rect r;
    const Rect *t = &tl->rect[TB_SLIDER];

    r.x = t->x + 1 + zoom_to_slider(pt, travel(tl));
    r.y = t->y + 1;
    r.w = TB_THUMB_W;
    r.h = t->h - 2;
    return r;
}

int toolbar_zoom_at(const ToolbarLayout *tl, int x)
{
    int pos = x - tl->rect[TB_SLIDER].x - 1 - TB_THUMB_W / 2;
    int range = travel(tl);

    if (pos < 0)
        pos = 0;
    if (pos > range)
        pos = range;
    return zoom_from_slider(pos, range);
}
