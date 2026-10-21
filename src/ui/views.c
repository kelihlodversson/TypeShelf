#include <stdio.h>
#include <string.h>
#include <gem.h>
#include "layout/layout.h"
#include "ui/draw.h"
#include "ui/viewmodel.h"
#include "ui/views.h"
#include "vdi_if/vdi_extra.h"


#define PAD     4       /* inside a tile, a card or a row */
#define GAP     8       /* between tiles and cards */
#define ICON_GAP 3      /* between an icon's frame and its labels */

typedef struct {
    int cw, ch;                 /* the system font's cell: what the labels use */
    int glyph_w, glyph_h;       /* the largest cell of the zoom size over the families' fonts */
    int block_h;                /* a card's height */
    int icon_h;                 /* an icon tile's: the frame and the labels below */
    int tile_w;
    int row_h;                  /* a List row */
    int card_h;                 /* a style's card in a family's Samples */
} Metrics;

/* the labels' font: the system font, black; the Icons view's tiles use its
 * smallest size (6x6 on the Atari fonts) to fit their text.  metrics() sets
 * which, and every entry point calls it first. */
#define SMALL_PT    8

static int small_labels;

static void chrome(const UiEnv *env)
{
    short a, b, c, d;

    vst_font(env->vh, env->sys_font);
    if (small_labels)
        vst_point(env->vh, SMALL_PT, &a, &b, &c, &d);
    else
        vst_height(env->vh, env->sys_char_h, &a, &b, &c, &d);
    vst_effects(env->vh, 0);
    vst_color(env->vh, 1);
}

/* the label font's cell */
static void label_cell(const UiEnv *env, int *cw, int *ch)
{
    short a, b, c, d;

    *cw = env->cell_w;
    *ch = env->cell_h;
    if (small_labels) {
        vst_font(env->vh, env->sys_font);
        vst_point(env->vh, SMALL_PT, &a, &b, &c, &d);
        if (c > 0 && d > 0) {
            *cw = c;
            *ch = d;
        }
    }
}

/* the baseline for a line of previews whose top is y: the cells are as tall
 * as the largest font's, and a baseline sits about 4/5 of the way down one */
static int base_y(const Metrics *m, int top)
{
    return top + m->glyph_h * 4 / 5;
}

/* a family's default style at pt points; the cell it gives */
static void use_font(const UiEnv *env, const FontFamily *f, int style, int pt, int *cellw, int *cellh)
{
    const FontStyle *st = &f->styles[style];
    short cw = 0, chh = 0, w = 0, h = 0;

    vst_font(env->vh, st->id);
    if (env->have_arbpt && (st->format & ~FONT_FORMAT_BITMAP))
        vst_arbpt(env->vh, (short)pt, &cw, &chh, &w, &h);
    if (h <= 0)
        vst_point(env->vh, (short)pt, &cw, &chh, &w, &h);
    *cellw = w;
    *cellh = h;
}

/* the largest preview cell over the families, remembered per font list and
 * zoom: measuring selects every family's font in the VDI */
static struct {
    const FontList *list;
    int count, pt, have_arbpt;
    int w, h;
    int valid;
} cell_cache;

static void preview_cell(const UiEnv *env, const FontList *list, int pt, int *gw, int *gh)
{
    int i, sized = 0;

    if (cell_cache.valid && cell_cache.list == list && cell_cache.count == list->family_count
        && cell_cache.pt == pt && cell_cache.have_arbpt == env->have_arbpt) {
        *gw = cell_cache.w;
        *gh = cell_cache.h;
        return;
    }
    /* every family previews in its own font, so size for the largest cell */
    for (i = 0; i < list->family_count; i++) {
        const FontFamily *f = &list->families[i];
        int cw, chh;

        if (f->style_count <= 0)
            continue;
        use_font(env, f, family_default_style(f), pt, &cw, &chh);
        if (cw > *gw || !sized)
            *gw = cw;
        if (chh > *gh || !sized)
            *gh = chh;
        sized = 1;
    }
    chrome(env);
    cell_cache.list = list;
    cell_cache.count = list->family_count;
    cell_cache.pt = pt;
    cell_cache.have_arbpt = env->have_arbpt;
    cell_cache.w = *gw;
    cell_cache.h = *gh;
    cell_cache.valid = 1;
}

/* the largest cell over one family's styles */
static void family_cell(const UiEnv *env, const FontFamily *f, int pt, int *gw, int *gh)
{
    int i, cw, chh;

    for (i = 0; i < f->style_count; i++) {
        use_font(env, f, i, pt, &cw, &chh);
        if (cw > *gw || i == 0)
            *gw = cw;
        if (chh > *gh || i == 0)
            *gh = chh;
    }
}

static void metrics(const UiEnv *env, const Window *w, Metrics *m)
{
    const FontList *list = env->fonts;

    small_labels = w->state.view == VIEW_ICONS;
    label_cell(env, &m->cw, &m->ch);
    m->glyph_w = m->cw;
    m->glyph_h = m->ch;
    /* the List only uses the system font */
    if (list != NULL && w->state.mode == MODE_SINGLE) {
        if (w->state.family >= 0 && w->state.family < list->family_count)
            family_cell(env, &list->families[w->state.family], w->state.zoom_pt, &m->glyph_w, &m->glyph_h);
        chrome(env);
    } else if (list != NULL && w->state.view != VIEW_LIST) {
        preview_cell(env, list, w->state.zoom_pt, &m->glyph_w, &m->glyph_h);
    }
    m->block_h = m->glyph_h + 2 * PAD + 2 * (m->ch + 2);
    m->icon_h = m->glyph_h + 2 * PAD + ICON_GAP + 2 * (m->ch + 2);
    m->tile_w = (small_labels ? 16 : 12) * m->cw + 2 * PAD;
    if (m->tile_w < 2 * m->glyph_w + 4 * PAD)
        m->tile_w = 2 * m->glyph_w + 4 * PAD;   /* the icon frame, with room around it */
    m->row_h = m->ch + 2;
    m->card_h = m->glyph_h + 2 * PAD + m->ch + 2;
}

static int family_count(const UiEnv *env)
{
    return env->fonts != NULL ? env->fonts->family_count : 0;
}

static void icons_grid(const UiEnv *env, const Window *w, const Metrics *m, int content_w, Grid *g)
{
    (void)w;
    grid_make(g, content_w, m->tile_w, m->icon_h, GAP, family_count(env));
}

/* rows in the List: the header, then the families and the styles shown */
static int list_rows(const UiEnv *env, Window *w)
{
    return 1 + list_row_count(env->fonts, window_expanded(w, family_count(env)));
}

/* ---- drawing ---- */

/* the text, cut to chars characters */
static void label(short vh, int x, int y, const char *text, int chars)
{
    char buf[80];

    if (chars < 0)
        chars = 0;
    snprintf(buf, sizeof buf, "%.*s", chars, text);
    draw_text(vh, x, y, buf);
}

/* the family in its default style at the zoom size, inside r (clipped to it
 * and to clip); the text's baseline is at y, so that families line up */
static void sample_text(const UiEnv *env, const Window *w, const FontFamily *f, int style, const Rect *r,
                        const Rect *clip, int x, int y, const char *text, int color)
{
    Rect inner, vis;
    int cw, chh;

    inner.x = r->x + 1;
    inner.y = r->y + 1;
    inner.w = r->w - 2;
    inner.h = r->h - 2;
    if (!rect_intersect(&inner, clip, &vis))
        return;
    draw_clip(env->vh, &vis);
    use_font(env, f, style, w->state.zoom_pt, &cw, &chh);
    vst_color(env->vh, (short)color);
    {
        /* the text is aligned to the top, so move it up by the font's top
         * line to put the baseline at y */
        short min, max, dist[5], width, eff[3];

        vqt_fontinfo(env->vh, &min, &max, dist, &width, eff);
        draw_text(env->vh, x, y - dist[4], text);
    }
    draw_clip(env->vh, clip);
    chrome(env);
}

/* a line of text centred under an icon; selected, it is white on black like
 * the Atari Desktop's icon labels */
static void tile_label(const UiEnv *env, const Metrics *m, const Rect *r, int y, const char *text, int selected)
{
    int chars = r->w / m->cw;
    int n = (int)strlen(text);
    Rect bar;

    if (n > chars)
        n = chars;
    bar.x = r->x + (r->w - n * m->cw) / 2 - 1;
    bar.y = y;
    bar.w = n * m->cw + 2;
    bar.h = m->ch + 2;
    if (selected)
        draw_box(env->vh, &bar, 1, 1);
    vst_color(env->vh, selected ? 0 : 1);
    label(env->vh, bar.x + 1, y + 1, text, n);
    vst_color(env->vh, 1);
}

/* like the Atari Desktop's icon view: the icon on a white rectangle (black
 * when selected), with its labels below */
static void draw_tile(const UiEnv *env, const Window *w, const Metrics *m, const FontFamily *f,
                      const Rect *r, const Rect *clip, int selected)
{
    short ext[8];
    char count[24];
    int width, dw, dh;
    Rect icon;
    int ly;

    icon.w = 2 * m->glyph_w + 2 * PAD;
    icon.h = m->glyph_h + 2 * PAD;
    icon.x = r->x + (r->w - icon.w) / 2;
    icon.y = r->y;
    draw_box(env->vh, &icon, selected ? 1 : 0, 0);
    use_font(env, f, family_default_style(f), w->state.zoom_pt, &dw, &dh);
    vqt_extent(env->vh, "Aa", ext);
    width = ext[2] - ext[0];
    chrome(env);
    sample_text(env, w, f, family_default_style(f), &icon, clip, icon.x + (icon.w - width) / 2, base_y(m, icon.y + PAD), "Aa", selected ? 0 : 1);
    ly = icon.y + icon.h + ICON_GAP;
    tile_label(env, m, r, ly, family_display_name(f, env->sys_font), selected);
    style_count_text(f->style_count, count, sizeof count);
    tile_label(env, m, r, ly + m->ch + 2, count, selected);
}

static void draw_card(const UiEnv *env, const Window *w, const Metrics *m, const FontFamily *f,
                      const Rect *r, const Rect *clip, int selected)
{
    char count[24];
    int color = selected ? 0 : 1;
    int n;

    draw_box(env->vh, r, selected ? 1 : 0, 1);
    chrome(env);
    vst_color(env->vh, (short)color);
    style_count_text(f->style_count, count, sizeof count);
    n = (int)strlen(count);
    label(env->vh, r->x + PAD, r->y + PAD, family_display_name(f, env->sys_font), (r->w - 3 * PAD - n * m->cw) / m->cw);
    label(env->vh, r->x + r->w - PAD - n * m->cw, r->y + PAD, count, n);
    sample_text(env, w, f, family_default_style(f), r, clip, r->x + PAD, base_y(m, r->y + PAD + m->ch + 2), sample_pangram, color);
    vst_color(env->vh, (short)color);
    label(env->vh, r->x + PAD, r->y + r->h - PAD - m->ch, f->styles[family_default_style(f)].style,
          (r->w - 2 * PAD) / m->cw);
    vst_color(env->vh, 1);
}

static unsigned short formats_of(const FontFamily *f)
{
    return f->formats;
}

/* ---- single font: Specimen and the family's Samples ---- */

#define REP_PT      16      /* the repertoire's size, whatever the zoom */

typedef struct {
    int preview_y;          /* the first preview line */
    int line_h;
    int rows;               /* preview rows after wrapping */
    int rep_label_y;
    int rep_y;              /* the repertoire grid */
    Grid rep;
    int rep_cw, rep_ch;     /* a glyph cell at REP_PT */
    int total;
} Specimen;

static const FontFamily *single_family(const UiEnv *env, const Window *w)
{
    const FontList *list = env->fonts;

    if (list == NULL || w->state.family < 0 || w->state.family >= list->family_count)
        return NULL;
    return &list->families[w->state.family];
}

static int single_style(const FontFamily *f, const Window *w)
{
    return w->state.style >= 0 && w->state.style < f->style_count ? w->state.style : 0;
}

/* how many leading characters of text fit in maxw pixels, in the font
 * selected now; at least one */
static int fit_chars(short vh, const char *text, int maxw)
{
    char buf[64];
    short ext[8];
    int k;

    for (k = (int)strlen(text); k > 1; k--) {
        snprintf(buf, sizeof buf, "%.*s", k, text);
        vqt_extent(vh, buf, ext);
        if (ext[2] - ext[0] <= maxw)
            return k;
    }
    return 1;
}

/* the rows the preview wraps to at the zoom size; with draw, draws them from x, y */
static int preview_rows(const UiEnv *env, const Window *w, const FontFamily *f, int maxw, int x, int y, int line_h,
                        int draw)
{
    int rows = 0, i;
    int dw, dh;

    use_font(env, f, single_style(f, w), w->state.zoom_pt, &dw, &dh);
    for (i = 0; i < PREVIEW_LINES; i++) {
        const char *rest = specimen_preview[i];

        while (*rest != '\0') {
            int k = fit_chars(env->vh, rest, maxw);

            if (draw)
                label(env->vh, x, y + rows * line_h, rest, k);
            rest += k;
            rows++;
        }
    }
    chrome(env);
    return rows;
}

static void specimen_layout(const UiEnv *env, const Window *w, const Metrics *m, int content_w, Specimen *sp)
{
    const FontFamily *f = single_family(env, w);
    int y = GAP;

    memset(sp, 0, sizeof *sp);
    if (f == NULL)
        return;
    y += m->ch + 2 + PAD;                       /* the "Preview" label */
    sp->preview_y = y;
    sp->line_h = m->glyph_h + 4;
    sp->rows = preview_rows(env, w, f, content_w - 2 * GAP, 0, 0, sp->line_h, 0);
    y += sp->rows * sp->line_h + GAP;
    sp->rep_label_y = y;
    y += m->ch + 2 + PAD;
    use_font(env, f, single_style(f, w), REP_PT, &sp->rep_cw, &sp->rep_ch);
    chrome(env);
    sp->rep_y = y;
    grid_make(&sp->rep, content_w, sp->rep_cw + 8, sp->rep_ch + 6, 2, repertoire_count());
    sp->total = y + grid_height(&sp->rep) + GAP;
}

static void draw_specimen(const UiEnv *env, const Window *w, const Metrics *m, const Rect *content, const Rect *clip)
{
    const FontFamily *f = single_family(env, w);
    Specimen sp;
    int oy = content->y - w->state.scroll_y;
    int dw, dh, i;
    Rect r, vis;

    if (f == NULL)
        return;
    specimen_layout(env, w, m, content->w, &sp);
    draw_text(env->vh, content->x + GAP, oy + GAP, "Preview");
    preview_rows(env, w, f, content->w - 2 * GAP, content->x + GAP, oy + sp.preview_y, sp.line_h, 1);
    draw_text(env->vh, content->x + GAP, oy + sp.rep_label_y, "Repertoire");
    use_font(env, f, single_style(f, w), REP_PT, &dw, &dh);
    for (i = 0; i < repertoire_count(); i++) {
        Rect t = grid_tile(&sp.rep, i);
        char ch[2];
        short ext[8];

        r.x = content->x + t.x;
        r.y = oy + sp.rep_y + t.y;
        r.w = t.w;
        r.h = t.h;
        if (!rect_intersect(&r, clip, &vis))
            continue;
        draw_box(env->vh, &r, 0, 1);
        ch[0] = (char)repertoire_char(i);
        ch[1] = '\0';
        vqt_extent(env->vh, ch, ext);
        draw_text(env->vh, r.x + (r.w - (ext[2] - ext[0])) / 2, r.y + (r.h - sp.rep_ch) / 2, ch);
    }
    chrome(env);
}

/* a style's card: its label and format, then the pangram in that style */
static void draw_style_card(const UiEnv *env, const Window *w, const Metrics *m, const FontFamily *f, int style,
                            const Rect *r, const Rect *clip)
{
    char text[32];
    int n;

    draw_box(env->vh, r, 0, 1);
    chrome(env);
    font_format_text(f->styles[style].format, text, sizeof text);
    n = (int)strlen(text);
    label(env->vh, r->x + PAD, r->y + PAD, f->styles[style].style, (r->w - 3 * PAD - n * m->cw) / m->cw);
    label(env->vh, r->x + r->w - PAD - n * m->cw, r->y + PAD, text, n);
    sample_text(env, w, f, style, r, clip, r->x + PAD, base_y(m, r->y + PAD + m->ch + 2), sample_pangram, 1);
}

static void draw_single(const UiEnv *env, const Window *w, const Metrics *m, const Rect *content, const Rect *clip)
{
    const FontFamily *f = single_family(env, w);
    int oy = content->y - w->state.scroll_y;
    int i;
    Rect r, vis;

    if (f == NULL)
        return;
    if (w->state.view == VIEW_SPECIMEN) {
        draw_specimen(env, w, m, content, clip);
        return;
    }
    for (i = 0; i < f->style_count; i++) {
        r.x = content->x + GAP;
        r.y = oy + vlist_top(m->card_h, GAP, i);
        r.w = content->w - 2 * GAP;
        r.h = m->card_h;
        if (rect_intersect(&r, clip, &vis))
            draw_style_card(env, w, m, f, i, &r, clip);
    }
}

int view_content_height(const UiEnv *env, Window *w)
{
    Metrics m;
    Rect c = window_content_rect(w);
    Grid g;

    metrics(env, w, &m);
    if (w->state.mode == MODE_SINGLE) {
        const FontFamily *f = single_family(env, w);
        Specimen sp;

        if (f == NULL)
            return 0;
        if (w->state.view == VIEW_SPECIMEN) {
            specimen_layout(env, w, &m, c.w, &sp);
            return sp.total;
        }
        return vlist_height(m.card_h, GAP, f->style_count);
    }
    switch (w->state.view) {
    case VIEW_ICONS:
        icons_grid(env, w, &m, c.w, &g);
        return grid_height(&g);
    case VIEW_SAMPLES:
        return vlist_height(m.block_h, GAP, family_count(env));
    case VIEW_LIST:
        return vlist_height(m.row_h, 0, list_rows(env, w));
    default:
        return 0;
    }
}

static void draw_list_row(const UiEnv *env, const Metrics *m, const FontList *list, const unsigned char *expanded,
                          int row_index, const Rect *r, int selected_family)
{
    short vh = env->vh;
    int x_name = r->x + 3 * m->cw;
    int x_styles = x_name + (r->w - 3 * m->cw) * 45 / 100;
    int x_format;
    int chars_name;
    char text[64];
    ListRow lr;
    short line[4];

    if (x_styles < x_name + 16 * m->cw)
        x_styles = x_name + 16 * m->cw;         /* room for a name, "9 styles" and "Bitmap" at least */
    x_format = x_styles + 10 * m->cw;
    chars_name = (x_styles - x_name - m->cw) / m->cw;
    if (row_index == 0) {
        draw_box(vh, r, 0, 0);
        draw_text(vh, x_name, r->y + 1, "Typeface");
        draw_text(vh, x_styles, r->y + 1, "Styles");
        draw_text(vh, x_format, r->y + 1, "Format");
        line[0] = (short)r->x;
        line[1] = (short)(r->y + r->h - 1);
        line[2] = (short)(r->x + r->w - 1);
        line[3] = line[1];
        vsl_color(vh, 1);
        v_pline(vh, 2, line);
        return;
    }
    lr = list_row(list, expanded, row_index - 1);
    if (lr.family < 0)
        return;
    {
        const FontFamily *f = &list->families[lr.family];
        int selected = lr.family == selected_family;

        draw_box(vh, r, selected && lr.style < 0 ? 1 : 0, 0);
        vst_color(vh, (short)(selected && lr.style < 0 ? 0 : 1));
        if (lr.style < 0) {
            draw_text(vh, r->x + m->cw, r->y + 1, expanded != NULL && expanded[lr.family] ? "-" : "+");
            label(vh, x_name, r->y + 1, family_display_name(f, env->sys_font), chars_name);
            style_count_text(f->style_count, text, sizeof text);
            draw_text(vh, x_styles, r->y + 1, text);
            font_format_text(formats_of(f), text, sizeof text);
            label(vh, x_format, r->y + 1, text, (r->x + r->w - x_format) / m->cw);
        } else {
            label(vh, x_name + 2 * m->cw, r->y + 1, f->styles[lr.style].style, chars_name - 2);
            font_format_text(f->styles[lr.style].format, text, sizeof text);
            label(vh, x_format, r->y + 1, text, (r->x + r->w - x_format) / m->cw);
        }
        vst_color(vh, 1);
    }
}

void view_draw(const UiEnv *env, Window *w, const Rect *content, const Rect *clip)
{
    Metrics m;
    const FontList *list = env->fonts;
    int count = family_count(env);
    int oy = content->y - w->state.scroll_y;
    int i;
    Rect r, vis;
    Grid g;

    draw_box(env->vh, content, 0, 0);
    chrome(env);
    vswr_mode(env->vh, MD_TRANS);       /* the text goes over a background drawn first, black or white */
    if (count == 0) {
        draw_text(env->vh, content->x + 8, content->y + 8, "No fonts found.");
        vswr_mode(env->vh, MD_REPLACE);
        return;
    }
    metrics(env, w, &m);
    chrome(env);

    if (w->state.mode == MODE_SINGLE) {
        draw_single(env, w, &m, content, clip);
        vswr_mode(env->vh, MD_REPLACE);
        return;
    }
    switch (w->state.view) {
    case VIEW_ICONS:
        icons_grid(env, w, &m, content->w, &g);
        for (i = 0; i < count; i++) {
            Rect t = grid_tile(&g, i);

            r.x = content->x + t.x;
            r.y = oy + t.y;
            r.w = t.w;
            r.h = t.h;
            if (rect_intersect(&r, clip, &vis))
                draw_tile(env, w, &m, &list->families[i], &r, clip, i == w->state.selected);
        }
        break;
    case VIEW_SAMPLES:
        for (i = 0; i < count; i++) {
            r.x = content->x + GAP;
            r.y = oy + vlist_top(m.block_h, GAP, i);
            r.w = content->w - 2 * GAP;
            r.h = m.block_h;
            if (rect_intersect(&r, clip, &vis))
                draw_card(env, w, &m, &list->families[i], &r, clip, i == w->state.selected);
        }
        break;
    case VIEW_LIST: {
        const unsigned char *expanded = window_expanded(w, count);
        int rows = list_rows(env, w);

        for (i = 0; i < rows; i++) {
            r.x = content->x;
            r.y = oy + vlist_top(m.row_h, 0, i);
            r.w = content->w;
            r.h = m.row_h;
            if (rect_intersect(&r, clip, &vis))
                draw_list_row(env, &m, list, expanded, i, &r, w->state.selected);
        }
        break;
    }
    default:
        break;
    }
    chrome(env);
    vswr_mode(env->vh, MD_REPLACE);
}

/* ---- selection ---- */

/* the selected item's span in content coordinates */
static int selected_span(const UiEnv *env, Window *w, const Metrics *m, int content_w, int *top, int *h)
{
    int sel = w->state.selected;
    Grid g;

    if (sel < 0 || sel >= family_count(env))
        return 0;
    switch (w->state.view) {
    case VIEW_ICONS: {
        Rect t;

        icons_grid(env, w, m, content_w, &g);
        t = grid_tile(&g, sel);
        *top = t.y;
        *h = t.h;
        return 1;
    }
    case VIEW_SAMPLES:
        *top = vlist_top(m->block_h, GAP, sel);
        *h = m->block_h;
        return 1;
    case VIEW_LIST:
        *top = vlist_top(m->row_h, 0, 1 + list_row_of_family(env->fonts, window_expanded(w, family_count(env)), sel));
        *h = m->row_h;
        return 1;
    default:
        return 0;
    }
}

static void show_selection(const UiEnv *env, Window *w)
{
    Metrics m;
    Rect c = window_content_rect(w);
    int top, h;

    metrics(env, w, &m);
    if (selected_span(env, w, &m, c.w, &top, &h))
        w->state.scroll_y = scroll_to_show(w->state.scroll_y, c.h, top, h);
}

static int select_family(const UiEnv *env, Window *w, int family)
{
    if (w->state.selected == family)
        return 0;
    w->state.selected = family;
    if (family >= 0)
        show_selection(env, w);
    return 1;
}

int view_click(const UiEnv *env, Window *w, int x, int y, int clicks)
{
    Metrics m;
    Rect c = window_content_rect(w);
    int cx = x - c.x, cy = y - c.y + w->state.scroll_y;
    int hit = -1;
    int changed = 0;
    int count = family_count(env);

    if (w->state.mode != MODE_OVERVIEW)
        return 0;
    metrics(env, w, &m);
    switch (w->state.view) {
    case VIEW_ICONS: {
        Grid g;

        icons_grid(env, w, &m, c.w, &g);
        hit = grid_hit(&g, cx, cy);
        break;
    }
    case VIEW_SAMPLES:
        hit = (cx >= GAP && cx < c.w - GAP) ? vlist_hit(m.block_h, GAP, count, cy) : -1;
        break;
    case VIEW_LIST: {
        unsigned char *expanded = window_expanded(w, count);
        int row = vlist_hit(m.row_h, 0, list_rows(env, w), cy);

        if (row > 0) {
            ListRow lr = list_row(env->fonts, expanded, row - 1);

            hit = lr.family;
            if (lr.style < 0 && cx < 3 * m.cw && expanded != NULL) {
                expanded[lr.family] = !expanded[lr.family];
                changed = 1;
            }
        }
        break;
    }
    default:
        return 0;
    }
    if (hit < 0 && clicks == 1)
        return select_family(env, w, -1) || changed;
    if (hit < 0)
        return changed;
    changed |= select_family(env, w, hit);
    if (changed)
        show_selection(env, w);
    if (clicks >= 2 && winstate_apply(&w->state, CMD_OPEN))
        changed = 1;
    return changed;
}

int view_navigate(const UiEnv *env, Window *w, Command cmd)
{
    Metrics m;
    Rect c = window_content_rect(w);
    int count = family_count(env);
    int sel = w->state.selected;
    int next = sel;

    if (w->state.mode == MODE_SINGLE) {
        const FontFamily *f = single_family(env, w);

        /* Left and Right pick the style in Specimen, Up and Down scroll */
        if ((cmd == CMD_LEFT || cmd == CMD_RIGHT) && f != NULL && winstate_style_enabled(&w->state)) {
            int style = style_step(w->state.style, cmd == CMD_LEFT ? -1 : 1, f->style_count);

            if (style == w->state.style)
                return 0;
            w->state.style = style;
            return 1;
        }
        if (cmd == CMD_UP || cmd == CMD_DOWN) {
            w->state.scroll_y += (cmd == CMD_UP ? -2 : 2) * env->cell_h;
            return 1;
        }
        return 0;
    }
    if (w->state.mode != MODE_OVERVIEW || count == 0)
        return 0;
    metrics(env, w, &m);
    switch (w->state.view) {
    case VIEW_ICONS: {
        Grid g;

        icons_grid(env, w, &m, c.w, &g);
        next = grid_move(&g, sel, cmd == CMD_LEFT ? -1 : cmd == CMD_RIGHT ? 1 : 0,
                         cmd == CMD_UP ? -1 : cmd == CMD_DOWN ? 1 : 0);
        break;
    }
    case VIEW_SAMPLES:
    case VIEW_LIST:
        if (w->state.view == VIEW_LIST && (cmd == CMD_LEFT || cmd == CMD_RIGHT)) {
            unsigned char *expanded = window_expanded(w, count);

            if (sel < 0 || expanded == NULL || expanded[sel] == (cmd == CMD_RIGHT))
                return 0;
            expanded[sel] = (unsigned char)(cmd == CMD_RIGHT);
            show_selection(env, w);
            return 1;
        }
        if (sel < 0)
            next = 0;
        else if (cmd == CMD_UP || cmd == CMD_LEFT)
            next = sel > 0 ? sel - 1 : 0;
        else
            next = sel < count - 1 ? sel + 1 : count - 1;
        break;
    default:
        return 0;
    }
    return select_family(env, w, next);
}
