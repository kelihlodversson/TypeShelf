#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "layout/layout.h"
#include "ui/icons.h"
#include "ui/window.h"

#define WINDOW_PARTS    (NAME | CLOSER | FULLER | MOVER | SIZER | UPARROW | DNARROW | VSLIDE)
#define INITIAL_W       800     /* a window opens at most this big; the user can enlarge it */
#define INITIAL_H       600
#define CASCADE_STEP    12
#define MIN_W           140
#define MIN_H           80

static void to_grect(const Rect *r, GRECT *g)
{
    g->g_x = (short)r->x;
    g->g_y = (short)r->y;
    g->g_w = (short)r->w;
    g->g_h = (short)r->h;
}

static Rect from_grect(const GRECT *g)
{
    Rect r;

    r.x = g->g_x;
    r.y = g->g_y;
    r.w = g->g_w;
    r.h = g->g_h;
    return r;
}

Window *window_open(const UiEnv *env, int cascade)
{
    Window *w = calloc(1, sizeof *w);
    GRECT g;
    Rect r;

    if (w == NULL)
        return NULL;
    winstate_init(&w->state);
    /* the desktop's rectangle is what "full" returns to, the cascaded one is where it opens */
    to_grect(&env->desk, &g);
    w->handle = wind_create_grect(WINDOW_PARTS, &g);
    if (w->handle < 0) {
        free(w);
        return NULL;
    }
    r = rect_initial(&env->desk, cascade, INITIAL_W, INITIAL_H, CASCADE_STEP);
    to_grect(&r, &g);
    window_refresh(env, w);
    wind_open_grect(w->handle, &g);
    window_refresh(env, w);
    return w;
}

void window_close(Window *w)
{
    wind_close(w->handle);
    wind_delete(w->handle);
    free(w);
}

Rect window_work(const Window *w)
{
    GRECT g;

    wind_get_grect(w->handle, WF_WORKXYWH, &g);
    return from_grect(&g);
}

Rect window_toolbar_rect(const Window *w)
{
    Rect r = window_work(w);

    r.h = w->tb.height;
    return r;
}

Rect window_content_rect(const Window *w)
{
    Rect r = window_work(w);

    r.y += w->tb.height;
    r.h -= w->tb.height;
    return r;
}

void window_refresh(const UiEnv *env, Window *w)
{
    Rect work = window_work(w);
    Rect content;
    const FontList *list = env->fonts;

    toolbar_layout(&w->tb, work.w, env->cell_w, w->state.mode);

    if (w->state.mode == MODE_SINGLE && list != NULL &&
        w->state.family >= 0 && w->state.family < list->family_count)
        snprintf(w->title, sizeof w->title, " %s ", list->families[w->state.family].name);
    else
        snprintf(w->title, sizeof w->title, " Type Shelf ");
    wind_set_str(w->handle, WF_NAME, w->title);

    /* nothing to scroll until a view has content: the slider shows it all */
    content = window_content_rect(w);
    wind_set_int(w->handle, WF_VSLSIZE, (short)scroll_slider_size(0, content.h));
    wind_set_int(w->handle, WF_VSLIDE, (short)scroll_slider_pos(w->state.scroll_y, 0, content.h));
}

/* ---- drawing ---------------------------------------------------------- */

static void clip_to(short vh, const Rect *r)
{
    short pxy[4];

    pxy[0] = (short)r->x;
    pxy[1] = (short)r->y;
    pxy[2] = (short)(r->x + r->w - 1);
    pxy[3] = (short)(r->y + r->h - 1);
    vs_clip(vh, 1, pxy);
}

/* a filled box, with a black outline if outline is set (the VDI draws a
 * perimeter in the fill color, so it is drawn here) */
static void box(short vh, const Rect *r, int fill_color, int outline)
{
    short bar[4], line[10];

    bar[0] = (short)r->x;
    bar[1] = (short)r->y;
    bar[2] = (short)(r->x + r->w - 1);
    bar[3] = (short)(r->y + r->h - 1);
    vsf_interior(vh, FIS_SOLID);
    vsf_color(vh, (short)fill_color);
    vsf_perimeter(vh, 0);
    v_bar(vh, bar);
    if (!outline)
        return;
    line[0] = bar[0]; line[1] = bar[1];
    line[2] = bar[2]; line[3] = bar[1];
    line[4] = bar[2]; line[5] = bar[3];
    line[6] = bar[0]; line[7] = bar[3];
    line[8] = bar[0]; line[9] = bar[1];
    vsl_color(vh, 1);
    vsl_type(vh, 1);
    v_pline(vh, 5, line);
}

/* the part of the window being drawn: the AES draws the icons on its own workstation */
static Rect drawing_clip;

static void text_at(short vh, int x, int y, const char *s)
{
    vst_alignment(vh, 0, 5, &(short){0}, &(short){0});
    v_gtext(vh, (short)x, (short)y, (char *)s);
}

/* a button, drawn inverted when it is the view in use */
static void draw_button(const UiEnv *env, const Window *w, ToolbarItem item, IconId icon, int on)
{
    Rect r = w->tb.rect[item];
    Rect work = window_work(w);
    int size = w->tb.icon;

    r.x += work.x;
    r.y += work.y;
    box(env->vh, &r, 0, 1);
    icons_draw(icon, size, r.x + (r.w - size) / 2, r.y + (r.h - size) / 2, on, &drawing_clip);
}

static void draw_style_pulldown(const UiEnv *env, const Window *w)
{
    Rect r = w->tb.rect[TB_STYLE];
    Rect work = window_work(w);
    const FontList *list = env->fonts;
    const char *label = "Regular";
    char text[13];
    int chars = toolbar_style_chars(&w->tb, env->cell_w);

    r.x += work.x;
    r.y += work.y;
    box(env->vh, &r, 0, 1);
    if (list != NULL && w->state.family >= 0 && w->state.family < list->family_count) {
        const FontFamily *f = &list->families[w->state.family];

        if (w->state.style >= 0 && w->state.style < f->style_count)
            label = f->styles[w->state.style].style;
    }
    snprintf(text, sizeof text, "%.*s", chars, label);
    vst_effects(env->vh, winstate_style_enabled(&w->state) ? 0 : 2);   /* light when disabled */
    text_at(env->vh, r.x + 3, r.y + (r.h - env->cell_h) / 2, text);
    text_at(env->vh, r.x + r.w - env->cell_w - 2, r.y + (r.h - env->cell_h) / 2, "\x1f");
    vst_effects(env->vh, 0);
}

static void draw_slider(const UiEnv *env, const Window *w)
{
    Rect track = w->tb.rect[TB_SLIDER];
    Rect work = window_work(w);
    Rect thumb = toolbar_thumb(&w->tb, w->state.zoom_pt);

    track.x += work.x;
    track.y += work.y;
    thumb.x += work.x;
    thumb.y += work.y;
    box(env->vh, &track, 0, 1);
    box(env->vh, &thumb, 1, 1);
}

static void draw_toolbar(const UiEnv *env, const Window *w)
{
    const ToolbarLayout *tl = &w->tb;
    const WinState *ws = &w->state;
    Rect bar = window_toolbar_rect(w);
    short line[4];

    box(env->vh, &bar, 0, 0);
    line[0] = (short)bar.x;
    line[1] = (short)(bar.y + bar.h - 1);
    line[2] = (short)(bar.x + bar.w - 1);
    line[3] = line[1];
    vsl_color(env->vh, 1);
    v_pline(env->vh, 2, line);

    if (tl->present[TB_BACK])
        draw_button(env, w, TB_BACK, ICON_BACK, 0);
    if (tl->present[TB_STYLE])
        draw_style_pulldown(env, w);
    if (tl->present[TB_VIEW1])
        draw_button(env, w, TB_VIEW1,
                    ws->mode == MODE_SINGLE ? ICON_SPECIMEN : ICON_ICONS,
                    ws->view == VIEW_ICONS || ws->view == VIEW_SPECIMEN);
    if (tl->present[TB_VIEW2])
        draw_button(env, w, TB_VIEW2, ICON_SAMPLES,
                    ws->view == VIEW_SAMPLES || ws->view == VIEW_FAMILY_SAMPLES);
    if (tl->present[TB_VIEW3])
        draw_button(env, w, TB_VIEW3, ICON_LIST, ws->view == VIEW_LIST);
    if (tl->present[TB_SLIDER])
        draw_slider(env, w);
}

static const char *view_name(const WinState *ws)
{
    switch (ws->view) {
    case VIEW_ICONS:            return "Icons";
    case VIEW_SAMPLES:          return "Samples";
    case VIEW_LIST:             return "List";
    case VIEW_SPECIMEN:         return "Specimen";
    default:                    return "Samples";
    }
}

/* what the views will draw: for now, the view and the zoom, in words */
static void draw_content(const UiEnv *env, const Window *w)
{
    Rect c = window_content_rect(w);
    char line[48];
    int n = env->fonts != NULL ? env->fonts->family_count : 0;

    box(env->vh, &c, 0, 0);
    snprintf(line, sizeof line, "%s view, %d pt", view_name(&w->state), w->state.zoom_pt);
    text_at(env->vh, c.x + 8, c.y + 8, line);
    snprintf(line, sizeof line, "%d font famil%s", n, n == 1 ? "y" : "ies");
    text_at(env->vh, c.x + 8, c.y + 8 + env->cell_h + 2, line);
}

void window_draw(const UiEnv *env, Window *w, const Rect *area)
{
    GRECT g;
    Rect r, clip;
    short vh = env->vh;

    wind_update(BEG_UPDATE);
    v_hide_c(vh);
    vswr_mode(vh, MD_REPLACE);
    vst_color(vh, 1);
    wind_get_grect(w->handle, WF_FIRSTXYWH, &g);
    while (g.g_w > 0 && g.g_h > 0) {
        r = from_grect(&g);
        if (rect_intersect(&r, area, &clip)) {
            clip_to(vh, &clip);
            drawing_clip = clip;
            draw_toolbar(env, w);
            draw_content(env, w);
        }
        wind_get_grect(w->handle, WF_NEXTXYWH, &g);
    }
    vs_clip(vh, 0, (short[4]){0, 0, 0, 0});
    v_show_c(vh, 1);
    wind_update(END_UPDATE);
}

ToolbarItem window_toolbar_hit(const Window *w, int x, int y)
{
    Rect bar = window_toolbar_rect(w);

    if (!rect_contains(&bar, x, y))
        return TB_NONE;
    return toolbar_hit(&w->tb, x - bar.x, y - bar.y);
}

/* follow the mouse to x: set the zoom and redraw the toolbar if it changed */
static void zoom_to_x(const UiEnv *env, Window *w, const Rect *bar, int x)
{
    int pt = toolbar_zoom_at(&w->tb, x - bar->x);

    if (pt != w->state.zoom_pt) {
        winstate_set_zoom(&w->state, pt);
        window_draw(env, w, bar);
    }
}

void window_drag_zoom(const UiEnv *env, Window *w, int x)
{
    Rect bar = window_toolbar_rect(w);
    short mx, my, buttons, kmeta;

    for (;;) {
        zoom_to_x(env, w, &bar, x);
        graf_mkstate(&mx, &my, &buttons, &kmeta);
        x = mx;
        if (!(buttons & 1)) {
            zoom_to_x(env, w, &bar, x);         /* where the button went up counts too */
            break;
        }
    }
    window_draw(env, w, &(Rect){ bar.x, bar.y + bar.h, bar.w, window_work(w).h - bar.h });
}

void window_set_rect(const UiEnv *env, Window *w, const Rect *r)
{
    GRECT g;
    Rect fixed = *r;

    if (fixed.w < MIN_W)
        fixed.w = MIN_W;
    if (fixed.h < MIN_H)
        fixed.h = MIN_H;
    to_grect(&fixed, &g);
    wind_set_grect(w->handle, WF_CURRXYWH, &g);
    window_refresh(env, w);
    /* the AES redraws what a resize exposes, not what stays: the toolbar may
     * have been laid out differently */
    {
        Rect work = window_work(w);

        window_draw(env, w, &work);
    }
}
