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

#endif
