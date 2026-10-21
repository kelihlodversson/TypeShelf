#include <stdio.h>
#include <string.h>
#include "layout/layout.h"
#include "ui/viewmodel.h"

const char sample_pangram[] = "The quick brown fox jumps over the lazy dog";

/* ---- grid ---- */

void grid_make(Grid *g, int content_w, int tile_w, int tile_h, int gap, int count)
{
    g->tile_w = tile_w;
    g->tile_h = tile_h;
    g->gap = gap;
    g->count = count;
    g->cols = layout_columns(content_w - gap, tile_w, gap);
    g->rows = layout_rows(count, g->cols);
}

int grid_height(const Grid *g)
{
    return g->gap + g->rows * (g->tile_h + g->gap);
}

Rect grid_tile(const Grid *g, int index)
{
    Rect r;

    r.x = g->gap + (index % g->cols) * (g->tile_w + g->gap);
    r.y = g->gap + (index / g->cols) * (g->tile_h + g->gap);
    r.w = g->tile_w;
    r.h = g->tile_h;
    return r;
}

int grid_hit(const Grid *g, int x, int y)
{
    int col, row, index;
    Rect r;

    x -= g->gap;
    y -= g->gap;
    if (x < 0 || y < 0)
        return -1;
    col = x / (g->tile_w + g->gap);
    row = y / (g->tile_h + g->gap);
    if (col >= g->cols)
        return -1;
    index = row * g->cols + col;
    if (index >= g->count)
        return -1;
    r = grid_tile(g, index);
    return rect_contains(&r, x + g->gap, y + g->gap) ? index : -1;
}

int grid_move(const Grid *g, int current, int dcol, int drow)
{
    int col, row, next;

    if (g->count <= 0)
        return -1;
    if (current < 0 || current >= g->count)
        return 0;
    col = current % g->cols + dcol;
    row = current / g->cols + drow;
    if (col < 0 || col >= g->cols || row < 0)
        return current;
    next = row * g->cols + col;
    if (next >= g->count)
        return drow > 0 && row < g->rows ? g->count - 1 : current;
    return next;
}

/* ---- vlist ---- */

int vlist_height(int row_h, int gap, int count)
{
    return gap + count * (row_h + gap);
}

int vlist_top(int row_h, int gap, int index)
{
    return gap + index * (row_h + gap);
}

int vlist_hit(int row_h, int gap, int count, int y)
{
    int index;

    y -= gap;
    if (y < 0)
        return -1;
    index = y / (row_h + gap);
    if (index >= count || y % (row_h + gap) >= row_h)
        return -1;
    return index;
}

/* ---- scrolling ---- */

int scroll_to_show(int scroll_y, int view_h, int top, int h)
{
    if (top < scroll_y)
        scroll_y = top;
    else if (top + h > scroll_y + view_h)
        scroll_y = top + h - view_h;
    if (h > view_h)
        scroll_y = top;                 /* too big to show: show its start */
    return scroll_y < 0 ? 0 : scroll_y;
}

int scroll_clamp(int scroll_y, int content_h, int view_h)
{
    int max = content_h - view_h;

    if (max < 0)
        max = 0;
    if (scroll_y > max)
        scroll_y = max;
    return scroll_y < 0 ? 0 : scroll_y;
}

/* ---- list ---- */

static int shown_styles(const FontList *list, const unsigned char *expanded, int family)
{
    return expanded != NULL && expanded[family] ? list->families[family].style_count : 0;
}

int list_row_count(const FontList *list, const unsigned char *expanded)
{
    int i, n = 0;

    for (i = 0; i < list->family_count; i++)
        n += 1 + shown_styles(list, expanded, i);
    return n;
}

ListRow list_row(const FontList *list, const unsigned char *expanded, int n)
{
    ListRow r;
    int i;

    for (i = 0; i < list->family_count; i++) {
        int rows = 1 + shown_styles(list, expanded, i);

        if (n < rows) {
            r.family = i;
            r.style = n - 1;
            return r;
        }
        n -= rows;
    }
    r.family = -1;
    r.style = -1;
    return r;
}

int list_row_of_family(const FontList *list, const unsigned char *expanded, int family)
{
    int i, n = 0;

    for (i = 0; i < family && i < list->family_count; i++)
        n += 1 + shown_styles(list, expanded, i);
    return n;
}

/* ---- text ---- */

int family_default_style(const FontFamily *family)
{
    int i, best = 0, best_d = 1 << 30;

    for (i = 0; i < family->style_count; i++) {
        int d = font_style_rank(family->styles[i].style) - font_style_rank("Regular");

        if (d < 0)
            d = -d;
        if (d < best_d) {
            best_d = d;
            best = i;
        }
    }
    return best;
}

void font_format_text(unsigned short formats, char *out, size_t cap)
{
    static const struct { unsigned short bit; const char *name; } names[] = {
        { FONT_FORMAT_BITMAP,   "Bitmap" },
        { FONT_FORMAT_SPEEDO,   "Speedo" },
        { FONT_FORMAT_TRUETYPE, "TrueType" },
        { FONT_FORMAT_TYPE1,    "Type 1" },
        { FONT_FORMAT_VECTOR,   "Vector" },     /* only when no format is known */
    };
    char text[64];
    size_t i;

    text[0] = '\0';
    for (i = 0; i < sizeof names / sizeof names[0]; i++) {
        if (!(formats & names[i].bit))
            continue;
        if (names[i].bit == FONT_FORMAT_VECTOR && (formats & ~FONT_FORMAT_VECTOR & ~FONT_FORMAT_BITMAP))
            continue;
        if (text[0] != '\0')
            strcat(text, ", ");
        strcat(text, names[i].name);
    }
    snprintf(out, cap, "%s", text[0] != '\0' ? text : "-");
}

void style_count_text(int count, char *out, size_t cap)
{
    snprintf(out, cap, "%d style%s", count, count == 1 ? "" : "s");
}

const char *family_display_name(const FontFamily *family, int sys_font)
{
    int i;

    for (i = 0; i < family->style_count; i++)
        if (family->styles[i].id == sys_font)
            return "System Font";
    return family->name;
}

void window_title_text(const FontList *list, const WinState *ws, int sys_font, char *out, size_t cap)
{
    const FontFamily *f;
    char count[24];

    if (ws->mode != MODE_SINGLE || list == NULL || ws->family < 0 || ws->family >= list->family_count) {
        snprintf(out, cap, "TypeShelf: All Fonts");
        return;
    }
    f = &list->families[ws->family];
    if (ws->view == VIEW_SPECIMEN && ws->style >= 0 && ws->style < f->style_count) {
        snprintf(out, cap, "TypeShelf: %s %s", family_display_name(f, sys_font), f->styles[ws->style].style);
        return;
    }
    style_count_text(f->style_count, count, sizeof count);
    snprintf(out, cap, "TypeShelf: %s, %s", family_display_name(f, sys_font), count);
}

const char *const specimen_preview[PREVIEW_LINES] = {
    "ABCDEFGHIJKLMNOPQRSTUVWXYZ",
    "abcdefghijklmnopqrstuvwxyz",
    "0123456789"
};

int repertoire_count(void)
{
    return (126 - 33 + 1) + (255 - 128 + 1);
}

int repertoire_char(int index)
{
    if (index < 0 || index >= repertoire_count())
        return -1;
    return index < 126 - 33 + 1 ? 33 + index : 128 + (index - (126 - 33 + 1));
}

int style_step(int current, int delta, int count)
{
    int next = current + delta;

    if (count <= 0)
        return 0;
    if (next < 0)
        next = 0;
    if (next > count - 1)
        next = count - 1;
    return next;
}
