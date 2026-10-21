#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "layout/layout.h"
#include "ui/viewmodel.h"
#include "ui/draw.h"
#include "ui/icons.h"
#include "ui/views.h"
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
    free(w->expanded);
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
    int content_h;
    const FontList *list = env->fonts;

    toolbar_layout(&w->tb, work.w, env->cell_w, w->state.mode);

    window_title_text(list, &w->state, env->sys_font, w->title, sizeof w->title);
    wind_set_str(w->handle, WF_NAME, w->title);

    content = window_content_rect(w);
    content_h = view_content_height(env, w);
    w->state.scroll_y = scroll_clamp(w->state.scroll_y, content_h, content.h);
    wind_set_int(w->handle, WF_VSLSIZE, (short)scroll_slider_size(content_h, content.h));
    wind_set_int(w->handle, WF_VSLIDE, (short)scroll_slider_pos(w->state.scroll_y, content_h, content.h));
}

/* ---- drawing ---------------------------------------------------------- */

/* the part of the window being drawn: the AES draws the icons on its own workstation */
static Rect drawing_clip;

/* a button, drawn inverted when it is the view in use */
static void draw_button(const UiEnv *env, const Window *w, ToolbarItem item, IconId icon, int on)
{
    Rect r = w->tb.rect[item];
    Rect work = window_work(w);
    int size = w->tb.icon;

    r.x += work.x;
    r.y += work.y;
    draw_box(env->vh, &r, 0, 1);
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
    draw_box(env->vh, &r, 0, 1);
    if (list != NULL && w->state.family >= 0 && w->state.family < list->family_count) {
        const FontFamily *f = &list->families[w->state.family];

        if (w->state.style >= 0 && w->state.style < f->style_count)
            label = f->styles[w->state.style].style;
    }
    snprintf(text, sizeof text, "%.*s", chars, label);
    draw_text(env->vh, r.x + 3, r.y + (r.h - env->cell_h) / 2, text);
    draw_down_arrow(env->vh, r.x + r.w - 9, r.y + (r.h - 5) / 2, 7);
    if (!winstate_style_enabled(&w->state)) {
        Rect in = { r.x + 1, r.y + 1, r.w - 2, r.h - 2 };

        draw_stipple(env->vh, &in);             /* the label and the arrow turn light */
    }
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
    draw_box(env->vh, &track, 0, 1);
    draw_box(env->vh, &thumb, 1, 1);
}

static void draw_toolbar(const UiEnv *env, const Window *w)
{
    const ToolbarLayout *tl = &w->tb;
    const WinState *ws = &w->state;
    Rect bar = window_toolbar_rect(w);
    short line[4];

    draw_box(env->vh, &bar, 0, 0);
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

/* the views leave the workstation in their own fonts: the toolbar and other
 * windows draw in the system font */
static void system_text(const UiEnv *env)
{
    short a, b, c, d;

    vst_font(env->vh, env->sys_font);
    vst_height(env->vh, env->sys_char_h, &a, &b, &c, &d);
    vst_effects(env->vh, 0);
    vst_color(env->vh, 1);
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
            draw_clip(vh, &clip);
            drawing_clip = clip;
            system_text(env);
            draw_toolbar(env, w);
            {
                Rect content = window_content_rect(w);
                Rect vclip;

                /* scrolled rows must not paint over the toolbar */
                if (rect_intersect(&content, &clip, &vclip)) {
                    draw_clip(vh, &vclip);
                    drawing_clip = vclip;
                    view_draw(env, w, &content, &vclip);
                }
            }
        }
        wind_get_grect(w->handle, WF_NEXTXYWH, &g);
    }
    system_text(env);
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
    window_refresh(env, w);                     /* the zoom changed what the views lay out */
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

unsigned char *window_expanded(Window *w, int family_count)
{
    if (w->expanded_n != family_count) {
        free(w->expanded);
        w->expanded = family_count > 0 ? calloc((size_t)family_count, 1) : NULL;
        w->expanded_n = w->expanded != NULL ? family_count : 0;
    }
    return w->expanded;
}

void window_scroll_to(const UiEnv *env, Window *w, int scroll_y)
{
    Rect content = window_content_rect(w);
    int before = w->state.scroll_y;

    w->state.scroll_y = scroll_clamp(scroll_y, view_content_height(env, w), content.h);
    if (w->state.scroll_y == before)
        return;
    window_refresh(env, w);
    window_draw(env, w, &content);
}
