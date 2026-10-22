#include <stdlib.h>
#include <string.h>
#include "layout/layout.h"
#include "ui/about.h"
#include "ui/app.h"
#include "ui/command.h"
#include "fontdb/build.h"
#include "ui/icons.h"
#include "ui/install.h"
#include "ui/menu.h"
#include "ui/popup.h"
#include "ui/views.h"
#include "layout/layout.h"
#include "ui/window.h"

typedef struct {
    UiEnv env;
    OBJECT *menu;
    Window *windows;            /* the front window first isn't kept: ask the AES */
    int opened;                 /* windows opened so far, for the cascade */
    int quit;
    int button_down;            /* a press was handled; wait for the release */
    AboutFacts about;
    GemSource *gs;
    const VdiFontSource *src;
    FontList *fonts;            /* rebuilt in place, so env.fonts stays valid */
    InstallKind install;        /* INSTALL_NONE: Install Font is greyed out */
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

/*
 * Read the fonts again (the GDOS unloads and loads them) and point the windows
 * at the same families in the new list; one whose family is gone goes back to
 * the overview.  Returns how many faces were added, which is negative if
 * fewer are listed now.
 */
static int reload_fonts(App *app)
{
    char (*names)[64] = NULL;
    Window *w;
    int n = 0, i, before = app->gs->count;

    for (w = app->windows; w != NULL; w = w->next)
        n++;
    if (n > 0)
        names = calloc((size_t)n, sizeof *names);
    for (w = app->windows, i = 0; w != NULL; w = w->next, i++) {
        const FontList *l = app->fonts;

        if (names != NULL && w->state.family >= 0 && w->state.family < l->family_count)
            strncpy(names[i], l->families[w->state.family].name, sizeof names[i] - 1);
    }

    gem_source_reload(app->gs);
    fontlist_free(app->fonts);
    if (fontdb_build(app->src, app->fonts) < 0)
        form_alert(1, "[3][Type Shelf|Out of memory.][OK]");
    views_forget();

    for (w = app->windows, i = 0; w != NULL; w = w->next, i++) {
        const FontFamily *f = names != NULL && names[i][0] != '\0' ?
                              fontlist_find(app->fonts, names[i]) : NULL;

        free(w->expanded);
        w->expanded = NULL;
        w->expanded_n = 0;
        w->state.selected = -1;
        w->state.scroll_y = 0;
        if (f != NULL) {
            w->state.family = (int)(f - app->fonts->families);
            if (w->state.style >= f->style_count)
                w->state.style = 0;
        } else if (w->state.mode == MODE_SINGLE) {
            w->state.mode = MODE_OVERVIEW;
            w->state.view = w->state.overview_view;
            w->state.family = -1;
        }
    }
    free(names);
    about_collect(&app->about, app->env.vh, app->fonts, app->gs->have_arbpt, app->gs->have_ucs2);
    for (w = app->windows; w != NULL; w = w->next)
        redraw_all(app, w);
    return app->gs->count - before;
}

static void install_font(App *app)
{
    InstallDone done;

    if (app->install == INSTALL_NONE || !install_ask_and_copy(app->install, &done))
        return;
    install_report(&done, reload_fonts(app) > 0);
}

static void execute(App *app, Command cmd)
{
    Window *w = front_window(app);

    switch (cmd) {
    case CMD_ABOUT:
        about_show(&app->about);
        break;
    case CMD_NEW_WINDOW:
        new_window(app);
        break;
    case CMD_INSTALL:
        install_font(app);
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
    case CMD_LEFT:
    case CMD_RIGHT:
    case CMD_UP:
    case CMD_DOWN:
        if (w != NULL && view_navigate(&app->env, w, cmd))
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
    case FNT_CHANGED:
        reload_fonts(app);
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
    case WM_ARROWED:
        if (w != NULL) {
            Rect c = window_content_rect(w);
            int line = 2 * app->env.cell_h;
            int page = c.h > line ? c.h - line : c.h;
            int y = w->state.scroll_y;

            switch (msg[4]) {
            case WA_UPLINE: y -= line; break;
            case WA_DNLINE: y += line; break;
            case WA_UPPAGE: y -= page; break;
            case WA_DNPAGE: y += page; break;
            default: break;
            }
            window_scroll_to(&app->env, w, y);
        }
        break;
    case WM_VSLID:
        if (w != NULL) {
            Rect c = window_content_rect(w);

            window_scroll_to(&app->env, w, scroll_offset_from_slider(msg[4], view_content_height(&app->env, w), c.h));
        }
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

static const char *style_name(const void *ctx, int index)
{
    return ((const FontFamily *)ctx)->styles[index].style;
}

/* the style pulldown: pick which style the Specimen shows */
static void choose_style(App *app, Window *w)
{
    const FontList *list = app->env.fonts;
    const FontFamily *f;
    Rect anchor = w->tb.rect[TB_STYLE];
    Rect work = window_work(w);
    int style;

    if (!winstate_style_enabled(&w->state) || list == NULL || w->state.family < 0 ||
        w->state.family >= list->family_count)
        return;
    f = &list->families[w->state.family];
    anchor.x += work.x;
    anchor.y += work.y;
    style = popup_choose(&app->env, &anchor, style_name, f, f->style_count, w->state.style);
    if (style >= 0 && style != w->state.style) {
        w->state.style = style;
        w->state.scroll_y = 0;
        redraw_all(app, w);
    }
}

static void on_click(App *app, int x, int y, int clicks)
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
    if (item == TB_NONE) {
        Rect content = window_content_rect(w);

        if (rect_contains(&content, x, y) && view_click(&app->env, w, x, y, clicks))
            redraw_all(app, w);
        return;
    }
    switch (item) {
    case TB_BACK:       execute(app, CMD_BACK); break;
    case TB_VIEW1:      execute(app, CMD_VIEW_1); break;
    case TB_VIEW2:      execute(app, CMD_VIEW_2); break;
    case TB_VIEW3:      execute(app, CMD_VIEW_3); break;
    case TB_SLIDER:     window_drag_zoom(&app->env, w, x); break;
    case TB_STYLE:      choose_style(app, w); break;
    default:            break;
    }
}

int app_run(GemSource *gs, const VdiFontSource *src, FontList *fonts)
{
    short vh = gs->handle;
    int have_arbpt = gs->have_arbpt;
    int have_ucs2 = gs->have_ucs2;
    App app;
    short cw, ch, box_w, box_h;
    GRECT desk;
    EVMULT_IN in;
    EVMULT_OUT out;
    short msg[8];

    memset(&app, 0, sizeof app);
    app.gs = gs;
    app.src = src;
    app.fonts = fonts;
    about_collect(&app.about, vh, fonts, have_arbpt, have_ucs2);
    app.install = install_kind(&app.about);
    app.env.vh = vh;
    app.env.fonts = fonts;
    app.env.have_arbpt = have_arbpt;
    app.env.have_ucs2 = have_ucs2;
    graf_handle(&cw, &ch, &box_w, &box_h);        /* the system font's cell, and its box */
    app.env.cell_w = cw;
    app.env.cell_h = ch;
    {
        short attrib[10];

        vqt_attributes(vh, attrib);
        app.env.sys_font = attrib[0];
        app.env.sys_char_h = attrib[7];
    }
    wind_get_grect(0, WF_WORKXYWH, &desk);
    app.env.desk.x = desk.g_x;
    app.env.desk.y = desk.g_y;
    app.env.desk.w = desk.g_w;
    app.env.desk.h = desk.g_h;

    if (icons_load() != 0) {
        form_alert(1, "[3][Type Shelf|Cannot find TYPESHLF.RSC.][OK]");
        return -1;
    }
    app.menu = menu_create(desk.g_w / cw, (desk.g_y + desk.g_h) / ch);
    if (app.menu == NULL) {
        icons_free();
        return -1;
    }
    menu_set_install_enabled(app.menu, app.install != INSTALL_NONE);
    graf_mouse(ARROW, NULL);
    menu_bar(app.menu, 1);
    new_window(&app);
    if (app.windows == NULL) {
        menu_bar(app.menu, 0);
        menu_destroy(app.menu);
        icons_free();
        return -1;
    }

    memset(&in, 0, sizeof in);
    in.emi_flags = MU_MESAG | MU_BUTTON | MU_KEYBD;
    in.emi_bclicks = 2;                  /* a double click comes as one event with emo_mclicks 2 */
    in.emi_bmask = 1;
    in.emi_bstate = 1;
    while (!app.quit) {
        short events = evnt_multi_fast(&in, msg, &out);

        if (events & MU_MESAG)
            on_message(&app, msg);
        if (events & MU_KEYBD)
            execute(&app, command_from_key((unsigned short)out.emo_kreturn,
                                           (unsigned short)out.emo_kmeta));
        if (events & MU_BUTTON) {
            /* ask for the press and the release in turn, so a held button is one click */
            if (!app.button_down)
                on_click(&app, out.emo_mouse.p_x, out.emo_mouse.p_y, out.emo_mclicks);
            app.button_down = !app.button_down;
            in.emi_bstate = app.button_down ? 0 : 1;
        }
    }

    while (app.windows != NULL)
        close_window(&app, app.windows);
    menu_bar(app.menu, 0);
    menu_destroy(app.menu);
    icons_free();
    return 0;
}
