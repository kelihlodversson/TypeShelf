#include "layout/layout.h"
#include "ui/winstate.h"

void winstate_init(WinState *ws)
{
    ws->mode = MODE_OVERVIEW;
    ws->view = VIEW_ICONS;
    ws->overview_view = VIEW_ICONS;
    ws->zoom_pt = ZOOM_DEFAULT_PT;
    ws->scroll_y = 0;
    ws->selected = -1;
    ws->family = -1;
    ws->style = 0;
}

void winstate_enter(WinState *ws, int family)
{
    if (ws->mode == MODE_OVERVIEW)
        ws->overview_view = ws->view;
    ws->mode = MODE_SINGLE;
    ws->view = VIEW_SPECIMEN;
    ws->family = family;
    ws->selected = family;
    ws->style = 0;
    ws->scroll_y = 0;
}

static int set_view(WinState *ws, View view)
{
    if (ws->view == view)
        return 0;
    ws->view = view;
    ws->scroll_y = 0;
    return 1;
}

int winstate_apply(WinState *ws, Command cmd)
{
    switch (cmd) {
    case CMD_VIEW_1:
        return set_view(ws, ws->mode == MODE_OVERVIEW ? VIEW_ICONS : VIEW_SPECIMEN);
    case CMD_VIEW_2:
        return set_view(ws, ws->mode == MODE_OVERVIEW ? VIEW_SAMPLES : VIEW_FAMILY_SAMPLES);
    case CMD_VIEW_3:
        if (ws->mode != MODE_OVERVIEW)
            return 0;
        return set_view(ws, VIEW_LIST);
    case CMD_BACK:
        if (ws->mode != MODE_SINGLE)
            return 0;
        ws->mode = MODE_OVERVIEW;
        ws->view = ws->overview_view;
        ws->family = -1;
        ws->scroll_y = 0;
        return 1;
    case CMD_OPEN:
        if (ws->mode != MODE_OVERVIEW || ws->selected < 0)
            return 0;
        winstate_enter(ws, ws->selected);
        return 1;
    default:
        return 0;
    }
}

void winstate_set_zoom(WinState *ws, int pt)
{
    if (pt < ZOOM_MIN_PT)
        pt = ZOOM_MIN_PT;
    if (pt > ZOOM_MAX_PT)
        pt = ZOOM_MAX_PT;
    ws->zoom_pt = pt;
}

int winstate_list_enabled(const WinState *ws)
{
    return ws->mode == MODE_OVERVIEW;
}

int winstate_style_enabled(const WinState *ws)
{
    return ws->mode == MODE_SINGLE && ws->view == VIEW_SPECIMEN;
}
