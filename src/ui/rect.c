#include "ui/rect.h"

int rect_intersect(const Rect *a, const Rect *b, Rect *out)
{
    int x0 = a->x > b->x ? a->x : b->x;
    int y0 = a->y > b->y ? a->y : b->y;
    int x1 = a->x + a->w < b->x + b->w ? a->x + a->w : b->x + b->w;
    int y1 = a->y + a->h < b->y + b->h ? a->y + a->h : b->y + b->h;

    if (x1 <= x0 || y1 <= y0)
        return 0;
    out->x = x0;
    out->y = y0;
    out->w = x1 - x0;
    out->h = y1 - y0;
    return 1;
}

int rect_contains(const Rect *r, int x, int y)
{
    return x >= r->x && y >= r->y && x < r->x + r->w && y < r->y + r->h;
}

Rect rect_initial(const Rect *desk, int cascade, int max_w, int max_h, int step)
{
    Rect r;
    int shift = step * (cascade % 6);

    r.w = desk->w < max_w ? desk->w : max_w;
    r.h = desk->h < max_h ? desk->h : max_h;
    r.x = desk->x + (desk->w - r.w) / 2 + shift;
    r.y = desk->y + (desk->h - r.h) / 2 + shift;
    if (r.x + r.w > desk->x + desk->w)
        r.w = desk->x + desk->w - r.x;
    if (r.y + r.h > desk->y + desk->h)
        r.h = desk->y + desk->h - r.y;
    return r;
}
