#include <string.h>
#include <gem.h>
#include "ui/draw.h"
#include "ui/popup.h"
#include "ui/viewmodel.h"

#define TICK_MS     30
#define SCAN_ESC    0x01

static void draw_row(const UiEnv *env, const Rect *pop, int row_h, int row, const char *text, int on, int current)
{
    Rect r;

    r.x = pop->x + 1;
    r.y = pop->y + 1 + row * row_h;
    r.w = pop->w - 2;
    r.h = row_h;
    draw_box(env->vh, &r, on ? 1 : 0, 0);
    vst_color(env->vh, on ? 0 : 1);
    if (current)
        draw_text(env->vh, r.x + 1, r.y + 1, "\x08");           /* the check mark */
    draw_text(env->vh, r.x + env->cell_w + 2, r.y + 1, text);
    vst_color(env->vh, 1);
}

int popup_choose(const UiEnv *env, const Rect *anchor, const char *(*item)(const void *ctx, int index),
                 const void *ctx, int count, int current)
{
    short vh = env->vh;
    int row_h = env->cell_h + 2;
    int i, longest = 0, rows, hover = -1, result = -1, prev_down = 1;
    Rect pop;
    EVMULT_IN in;
    EVMULT_OUT out;
    short msg[8];
    short a, b, c, d;

    if (count <= 0)
        return -1;
    for (i = 0; i < count; i++) {
        int n = (int)strlen(item(ctx, i));

        if (n > longest)
            longest = n;
    }
    pop = popup_rect(anchor, (longest + 2) * env->cell_w + 6, row_h, count, &env->desk);
    rows = (pop.h - 2) / row_h;

    form_dial(FMD_START, 0, 0, 0, 0, pop.x, pop.y, pop.w + 2, pop.h + 2);
    v_hide_c(vh);
    vswr_mode(vh, MD_TRANS);
    vst_font(vh, env->sys_font);
    vst_height(vh, env->sys_char_h, &a, &b, &c, &d);
    vst_effects(vh, 0);
    vs_clip(vh, 0, (short[4]){0, 0, 0, 0});
    draw_box(vh, &pop, 0, 1);
    for (i = 0; i < rows; i++)
        draw_row(env, &pop, row_h, i, item(ctx, i), 0, i == current);
    v_show_c(vh, 1);

    memset(&in, 0, sizeof in);
    in.emi_flags = MU_KEYBD | MU_TIMER;
    in.emi_tlow = TICK_MS;
    for (;;) {
        short mx, my, mb;
        int row, down;
        short events = evnt_multi_fast(&in, msg, &out);

        if ((events & MU_KEYBD) && ((out.emo_kreturn >> 8) & 0xff) == SCAN_ESC)
            break;
        vq_mouse(vh, &mb, &mx, &my);
        down = mb & 1;
        row = popup_row(&pop, row_h, mx, my);
        if (row != hover) {
            v_hide_c(vh);
            if (hover >= 0)
                draw_row(env, &pop, row_h, hover, item(ctx, hover), 0, hover == current);
            if (row >= 0)
                draw_row(env, &pop, row_h, row, item(ctx, row), 1, row == current);
            v_show_c(vh, 1);
            hover = row;
        }
        if (prev_down && !down) {
            /* released: over a row after a drag chooses it; a plain click's release lands on the control */
            if (row >= 0) {
                result = row;
                break;
            }
        } else if (!prev_down && down) {
            result = row;               /* a press on a row chooses it, anywhere else cancels */
            break;
        }
        prev_down = down;
    }
    for (;;) {                          /* let go before the main loop looks at the button */
        short mx, my, mb;

        vq_mouse(vh, &mb, &mx, &my);
        if (!(mb & 1))
            break;
        evnt_timer(TICK_MS);
    }
    vswr_mode(vh, MD_REPLACE);
    form_dial(FMD_FINISH, 0, 0, 0, 0, pop.x, pop.y, pop.w + 2, pop.h + 2);
    return result;
}
