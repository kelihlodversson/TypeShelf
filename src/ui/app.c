#include <stdlib.h>
#include <string.h>
#include "layout/layout.h"
#include "ui/app.h"
#include "ui/command.h"
#include "ui/menu.h"
#include "ui/window.h"

typedef struct {
    UiEnv env;
    OBJECT *menu;
    Window *windows;            /* the front window first isn't kept: ask the AES */
    int opened;                 /* windows opened so far, for the cascade */
    int quit;
    const char *about;
} App;

static Window *find_window(App *app, int handle)
{
    Window *w;

    for (w = app->windows; w != NULL; w = w->next)
        if (w->handle == handle)
            return w;
    return NULL;
}

static Window *front_window(App *app)
{
    short top, dummy;

    wind_get(0, WF_TOP, &top, &dummy, &dummy, &dummy);
    return find_window(app, top);
}

static void refresh_menu(App *app)
{
    Window *w = front_window(app);

    menu_update(app->menu, w != NULL ? &w->state : NULL);
}

static void redraw_all(App *app, Window *w)
{
    Rect work = window_work(w);

    window_refresh(&app->env, w);
    window_draw(&app->env, w, &work);
    refresh_menu(app);
}

static void new_window(App *app)
{
    Window *w = window_open(&app->env, app->opened++);

    if (w == NULL) {
        form_alert(1, "[3][Type Shelf|No more windows.][OK]");
        return;
    }
    w->next = app->windows;
    app->windows = w;
    refresh_menu(app);
}

static void close_window(App *app, Window *w)
{
    Window **p;

    for (p = &app->windows; *p != NULL; p = &(*p)->next) {
        if (*p == w) {
            *p = w->next;
            break;
        }
    }
    window_close(w);
    refresh_menu(app);
}

static void execute(App *app, Command cmd)
{
    Window *w = front_window(app);

    switch (cmd) {
    case CMD_ABOUT:
        form_alert(1, (char *)app->about);
        break;
    case CMD_NEW_WINDOW:
        new_window(app);
        break;
    case CMD_INSTALL:
        form_alert(1, "[1][Install Font|is not built yet.][OK]");
        break;
    case CMD_CLOSE:
        if (w != NULL)
            close_window(app, w);
        break;
    case CMD_QUIT:
        app->quit = 1;
        break;
    case CMD_VIEW_1:
    case CMD_VIEW_2:
    case CMD_VIEW_3:
    case CMD_BACK:
    case CMD_OPEN:
        if (w != NULL && winstate_apply(&w->state, cmd))
            redraw_all(app, w);
        break;
    default:
        break;
    }
}

static void on_message(App *app, const short msg[8])
{
    Window *w = msg[0] == MN_SELECTED ? NULL : find_window(app, msg[3]);
    Rect r;

    switch (msg[0]) {
    case MN_SELECTED:
        execute(app, menu_command(msg[4]));
        menu_tnormal(app->menu, msg[3], 1);
        break;
    case WM_REDRAW:
        if (w != NULL) {
            r.x = msg[4];
            r.y = msg[5];
            r.w = msg[6];
            r.h = msg[7];
            window_draw(&app->env, w, &r);
        }
        break;
    case WM_TOPPED:
    case WM_NEWTOP:
        if (w != NULL) {
            wind_set_int(w->handle, WF_TOP, 0);
            refresh_menu(app);
        }
        break;
    case WM_CLOSED:
        if (w != NULL)
            close_window(app, w);
        break;
    case WM_FULLED:
        if (w != NULL) {
            GRECT g;

            if (w->fulled) {
                r = w->before_full;
            } else {
                wind_get_grect(w->handle, WF_CURRXYWH, &g);
                w->before_full.x = g.g_x;
                w->before_full.y = g.g_y;
                w->before_full.w = g.g_w;
                w->before_full.h = g.g_h;
                wind_get_grect(w->handle, WF_FULLXYWH, &g);
                r.x = g.g_x;
                r.y = g.g_y;
                r.w = g.g_w;
                r.h = g.g_h;
            }
            w->fulled = !w->fulled;
            window_set_rect(&app->env, w, &r);
        }
        break;
    case WM_SIZED:
    case WM_MOVED:
        if (w != NULL) {
            r.x = msg[4];
            r.y = msg[5];
            r.w = msg[6];
            r.h = msg[7];
            w->fulled = 0;
            window_set_rect(&app->env, w, &r);
        }
        break;
    default:
        break;
    }
}

static void on_click(App *app, int x, int y)
{
    Window *w = find_window(app, wind_find(x, y));
    ToolbarItem item;

    if (w == NULL)
        return;
    if (w != front_window(app)) {
        wind_set_int(w->handle, WF_TOP, 0);
        refresh_menu(app);
        return;
    }
    item = window_toolbar_hit(w, x, y);
    switch (item) {
    case TB_BACK:       execute(app, CMD_BACK); break;
    case TB_VIEW1:      execute(app, CMD_VIEW_1); break;
    case TB_VIEW2:      execute(app, CMD_VIEW_2); break;
    case TB_VIEW3:      execute(app, CMD_VIEW_3); break;
    case TB_SLIDER:     window_drag_zoom(&app->env, w, x); break;
    default:            break;      /* the style pulldown comes with the single-font view */
    }
}

int app_run(short vh, const FontList *fonts, const char *about)
{
    App app;
    short cw, ch, box_w, box_h;
    GRECT desk;
    EVMULT_IN in;
    EVMULT_OUT out;
    short msg[8];

    memset(&app, 0, sizeof app);
    app.about = about;
    app.env.vh = vh;
    app.env.fonts = fonts;
    graf_handle(&cw, &ch, &box_w, &box_h);        /* the system font's cell, and its box */
    app.env.cell_w = cw;
    app.env.cell_h = ch;
    wind_get_grect(0, WF_WORKXYWH, &desk);
    app.env.desk.x = desk.g_x;
    app.env.desk.y = desk.g_y;
    app.env.desk.w = desk.g_w;
    app.env.desk.h = desk.g_h;

    app.menu = menu_create(desk.g_w / cw, (desk.g_y + desk.g_h) / ch);
    if (app.menu == NULL)
        return -1;
    graf_mouse(ARROW, NULL);
    menu_bar(app.menu, 1);
    new_window(&app);
    if (app.windows == NULL) {
        menu_bar(app.menu, 0);
        menu_destroy(app.menu);
        return -1;
    }

    memset(&in, 0, sizeof in);
    in.emi_flags = MU_MESAG | MU_BUTTON | MU_KEYBD;
    in.emi_bclicks = 1;
    in.emi_bmask = 1;
    in.emi_bstate = 1;
    while (!app.quit) {
        short events = evnt_multi_fast(&in, msg, &out);

        if (events & MU_MESAG)
            on_message(&app, msg);
        if (events & MU_KEYBD)
            execute(&app, command_from_key((unsigned short)out.emo_kreturn,
                                           (unsigned short)out.emo_kmeta));
        if (events & MU_BUTTON)
            on_click(&app, out.emo_mouse.p_x, out.emo_mouse.p_y);
    }

    while (app.windows != NULL)
        close_window(&app, app.windows);
    menu_bar(app.menu, 0);
    menu_destroy(app.menu);
    return 0;
}
