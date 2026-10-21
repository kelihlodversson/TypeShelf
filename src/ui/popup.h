/*
 * popup.h - a list of choices that pops up under a control
 */
#ifndef UI_POPUP_H
#define UI_POPUP_H

#include "ui/window.h"

/* Pop up the items under anchor (screen pixels) and follow the mouse until one
 * is chosen: by releasing the button over it after a drag, or by clicking it.
 * A click elsewhere or Esc cancels.  Returns the item's index, or -1.  The
 * current item is marked.  The caller's windows are redrawn through the AES
 * when the popup closes. */
int popup_choose(const UiEnv *env, const Rect *anchor, const char *(*item)(const void *ctx, int index),
                 const void *ctx, int count, int current);

#endif
