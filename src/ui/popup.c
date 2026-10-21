#include <string.h>
#include <gem.h>
#include "ui/draw.h"
#include "ui/popup.h"
#include "ui/viewmodel.h"

#define TICK_MS     30
#define SCAN_ESC    0x01
#define SCAN_RETURN 0x1c
#define SCAN_ENTER  0x72
#define SCAN_UP     0x48
#define SCAN_DOWN   0x50

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

static void draw_all(const UiEnv *env, const Rect *pop, int row_h, int rows, int top,
                     const char *(*item)(const void *ctx, int index), const void *ctx, int current, int sel)
{
    int i;

    for (i = 0; i < rows; i++)
        draw_row(env, pop, row_h, i, item(ctx, top + i), top + i == sel, top + i == current);
}

int popup_choose(const UiEnv *env, const Rect *anchor, const char *(*item)(const void *ctx, int index),
                 const void *ctx, int count, int current)
{
    short vh = env->vh;
    int row_h = env->cell_h + 2;
    int i, longest = 0, rows, top, sel = -1, result = -1, prev_down = 1, last_x = -1, last_y = -1, keys = 0;
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
    if (rows > count)
        rows = count;
    top = popup_top(count, rows, current);

    wind_update(BEG_UPDATE);            /* nobody else draws or reads the mouse meanwhile */
    wind_update(BEG_MCTRL);
    form_dial(FMD_START, 0, 0, 0, 0, pop.x, pop.y, pop.w + 2, pop.h + 2);
    v_hide_c(vh);
    vswr_mode(vh, MD_TRANS);
    vst_font(vh, env->sys_font);
    vst_height(vh, env->sys_char_h, &a, &b, &c, &d);
    vst_effects(vh, 0);
    vs_clip(vh, 0, (short[4]){0, 0, 0, 0});
    draw_box(vh, &pop, 0, 1);
    draw_all(env, &pop, row_h, rows, top, item, ctx, current, sel);
    v_show_c(vh, 1);

    memset(&in, 0, sizeof in);
    in.emi_flags = MU_KEYBD | MU_TIMER;
    in.emi_tlow = TICK_MS;
    for (;;) {
        short mx, my, mb;
        int row, down, want = sel, newtop = top, moved;
        short events = evnt_multi_fast(&in, msg, &out);

        if (events & MU_KEYBD) {
            int scan = (out.emo_kreturn >> 8) & 0xff;

            if (scan == SCAN_ESC)
                break;
            if ((scan == SCAN_RETURN || scan == SCAN_ENTER) && sel >= 0) {
                result = sel;
                break;
            }
            if (scan == SCAN_UP || scan == SCAN_DOWN) {
                if (scan == SCAN_UP)
                    want = sel < 0 ? count - 1 : sel - 1;
                else
                    want = sel < 0 ? 0 : sel + 1;
                if (want < 0)
                    want = 0;
                if (want > count - 1)
                    want = count - 1;
                if (want < newtop)
                    newtop = want;
                if (want >= newtop + rows)
                    newtop = want - rows + 1;
                keys = 1;               /* the keyboard leads until the pointer moves */
            }
        }
        vq_mouse(vh, &mb, &mx, &my);
        down = mb & 1;
        row = popup_row(&pop, row_h, mx, my);
        moved = mx != last_x || my != last_y;
        last_x = mx;
        last_y = my;
        /* a press or release is decided on the rows as drawn, before any scrolling */
        if (prev_down && !down) {
            /* released: over a row after a drag chooses it; a plain click's release lands on the control */
            if (row >= 0) {
                result = top + row;
                break;
            }
        } else if (!prev_down && down) {
            result = row >= 0 ? top + row : -1;     /* a press on a row chooses it, anywhere else cancels */
            break;
        }
        prev_down = down;
        if (moved)
            keys = 0;
        if (!keys) {
            /* the pointer leads; resting on the first or last row scrolls the rest into view */
            want = row >= 0 ? top + row : -1;
            if (row == 0 && top > 0)
                newtop = top - 1;
            else if (row == rows - 1 && top + rows < count && rows > 1)
                newtop = top + 1;
            if (newtop != top)
                want = row == 0 ? newtop : newtop + rows - 1;
        }
        if (want != sel || newtop != top) {
            v_hide_c(vh);
            top = newtop;
            sel = want;
            draw_all(env, &pop, row_h, rows, top, item, ctx, current, sel);
            v_show_c(vh, 1);
        }
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
    wind_update(END_MCTRL);
    wind_update(END_UPDATE);
    return result;
}
