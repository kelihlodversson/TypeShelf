/*
 * window.h - one Type Shelf window: its AES handle, its state, its toolbar
 *
 * The window draws a toolbar below the title bar and, for now, a placeholder
 * in the content area; the views fill that in later.
 */
#ifndef UI_WINDOW_H
#define UI_WINDOW_H

#include <gem.h>
#include "fontdb/group.h"
#include "ui/rect.h"
#include "ui/toolbar_layout.h"
#include "ui/winstate.h"

typedef struct {
    short vh;                   /* the screen workstation to draw with */
    int cell_w, cell_h;         /* the system font's cell */
    Rect desk;                  /* the desktop's work area */
    const FontList *fonts;
} UiEnv;

typedef struct Window {
    short handle;
    WinState state;
    ToolbarLayout tb;
    Rect before_full;           /* where "fulled" returns to */
    int fulled;
    char title[40];
    struct Window *next;
} Window;

/* Open a window, offset from the desktop's corner by cascade steps; NULL if
 * the AES has none left or there is no memory. */
Window *window_open(const UiEnv *env, int cascade);

void window_close(Window *w);

/* the work area, the toolbar and the content below it, in screen pixels */
Rect window_work(const Window *w);
Rect window_toolbar_rect(const Window *w);
Rect window_content_rect(const Window *w);

/* Lay the toolbar out again for the current size and mode, set the title and
 * the sliders. */
void window_refresh(const UiEnv *env, Window *w);

/* Draw the part of the window inside area (screen pixels), through the AES
 * rectangle list. */
void window_draw(const UiEnv *env, Window *w, const Rect *area);

/* a click at a screen point: the toolbar item under it, or TB_NONE */
ToolbarItem window_toolbar_hit(const Window *w, int x, int y);

/* Follow the mouse over the zoom slider until the button goes up. */
void window_drag_zoom(const UiEnv *env, Window *w, int x);

/* move or resize to r; keeps the window at least a usable size */
void window_set_rect(const UiEnv *env, Window *w, const Rect *r);

#endif
