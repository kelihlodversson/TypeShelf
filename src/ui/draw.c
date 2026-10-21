#include "ui/draw.h"

void draw_clip(short vh, const Rect *r)
{
    short pxy[4];

    pxy[0] = (short)r->x;
    pxy[1] = (short)r->y;
    pxy[2] = (short)(r->x + r->w - 1);
    pxy[3] = (short)(r->y + r->h - 1);
    vs_clip(vh, 1, pxy);
}

void draw_box(short vh, const Rect *r, int fill_color, int outline)
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

void draw_down_arrow(short vh, int x, int y, int w)
{
    short line[4];
    int i;

    vsl_color(vh, 1);
    vsl_type(vh, 1);
    for (i = 0; i <= w / 2; i++) {
        line[0] = (short)(x + i);
        line[1] = (short)(y + i);
        line[2] = (short)(x + w - 1 - i);
        line[3] = (short)(y + i);
        v_pline(vh, 2, line);
    }
}

void draw_stipple(short vh, const Rect *r)
{
    short pattern[16], bar[4];
    int i;

    for (i = 0; i < 16; i++)
        pattern[i] = (short)((i & 1) ? 0x5555 : 0xAAAA);
    bar[0] = (short)r->x;
    bar[1] = (short)r->y;
    bar[2] = (short)(r->x + r->w - 1);
    bar[3] = (short)(r->y + r->h - 1);
    vswr_mode(vh, MD_TRANS);            /* only the pattern's set bits are painted */
    vsf_udpat(vh, pattern, 1);
    vsf_interior(vh, FIS_USER);
    vsf_color(vh, 0);
    vsf_perimeter(vh, 0);
    v_bar(vh, bar);
    vswr_mode(vh, MD_REPLACE);
}

void draw_text(short vh, int x, int y, const char *s)
{
    short dummy_h, dummy_v;

    vst_alignment(vh, 0, 5, &dummy_h, &dummy_v);
    v_gtext(vh, (short)x, (short)y, (char *)s);
}

void draw_char16(short vh, int x, int y, int code)
{
    short dummy_h, dummy_v;
    short wc = (short)code;

    vst_alignment(vh, 0, 5, &dummy_h, &dummy_v);
    v_gtext16n(vh, (short)x, (short)y, &wc, 1);
}

int char16_width(short vh, int code)
{
    short ext[8];
    short wc = (short)code;

    vqt_extent16n(vh, &wc, 1, ext);
    return ext[2] - ext[0];
}
