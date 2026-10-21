/*
 * rect.h - rectangles in pixels, free of any GEM call
 */
#ifndef UI_RECT_H
#define UI_RECT_H

typedef struct {
    int x, y, w, h;
} Rect;

/* the part both share; 1 and *out set if it is not empty, otherwise 0 */
int rect_intersect(const Rect *a, const Rect *b, Rect *out);

/* 1 if the point is inside the rectangle */
int rect_contains(const Rect *r, int x, int y);

/*
 * Where a new window opens: centred on the desktop, at most max_w by max_h (a
 * big screen does not need a full-screen window), moved down and right by step
 * pixels for each of the first six windows (cascade 0 to 5) and cut back to
 * stay inside the desktop.
 */
Rect rect_initial(const Rect *desk, int cascade, int max_w, int max_h, int step);

#endif
