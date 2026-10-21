/*
 * views.h - the overview views: Icons, Samples and List
 *
 * They draw into a window's content area, scrolled by its state.  The model
 * they lay out with is in viewmodel.h.
 */
#ifndef UI_VIEWS_H
#define UI_VIEWS_H

#include "ui/window.h"

/* the height of what the window's overview view shows, at its zoom and width;
 * 0 for the views that are not built yet */
int view_content_height(const UiEnv *env, Window *w);

/* Draw the view in content (screen pixels), inside clip. */
void view_draw(const UiEnv *env, Window *w, const Rect *content, const Rect *clip);

/* A click at a screen point inside the content area; clicks is 1 or 2.  The
 * click selects, a double click opens, a click on a List arrow expands.  1 if
 * the window needs redrawing. */
int view_click(const UiEnv *env, Window *w, int x, int y, int clicks);

/* An arrow key.  1 if the window needs redrawing. */
int view_navigate(const UiEnv *env, Window *w, Command cmd);

#endif
