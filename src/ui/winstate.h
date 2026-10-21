/*
 * winstate.h - what a window shows, free of any GEM call
 *
 * A window is either an overview of all families (Icons, Samples or List) or
 * one family (Specimen or Samples).  The menu and the toolbar both turn into
 * Commands and come here.
 */
#ifndef UI_WINSTATE_H
#define UI_WINSTATE_H

#include "ui/command.h"

typedef enum {
    MODE_OVERVIEW,
    MODE_SINGLE
} WinMode;

typedef enum {
    VIEW_ICONS,
    VIEW_SAMPLES,
    VIEW_LIST,
    VIEW_SPECIMEN,
    VIEW_FAMILY_SAMPLES
} View;

#define ZOOM_DEFAULT_PT 24

typedef struct {
    WinMode mode;
    View view;
    View overview_view;     /* what Back returns to */
    int zoom_pt;            /* ZOOM_MIN_PT .. ZOOM_MAX_PT */
    int scroll_y;           /* pixels scrolled down */
    int selected;           /* family index in the overview, or -1 */
    int family;             /* the family shown when MODE_SINGLE */
    int style;              /* the style shown in Specimen */
} WinState;

void winstate_init(WinState *ws);

/* Apply a view, Back or Open command.  1 if the window changed, else 0. */
int winstate_apply(WinState *ws, Command cmd);

/* show one family */
void winstate_enter(WinState *ws, int family);

/* the zoom, kept within ZOOM_MIN_PT .. ZOOM_MAX_PT */
void winstate_set_zoom(WinState *ws, int pt);

/* "Show as List" is greyed out in a single font */
int winstate_list_enabled(const WinState *ws);

/* the style pulldown works in Specimen only */
int winstate_style_enabled(const WinState *ws);

#endif
