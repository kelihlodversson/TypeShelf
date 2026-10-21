#include "test.h"
#include "layout/layout.h"
#include "ui/command.h"
#include "ui/rect.h"
#include "ui/toolbar_layout.h"
#include "ui/winstate.h"

#define CTRL(scan) command_from_key((unsigned short)((scan) << 8), KMETA_CTRL)
#define PLAIN(scan) command_from_key((unsigned short)((scan) << 8), 0)

static void test_commands(void)
{
    CHECK_INT(CTRL(0x31), CMD_NEW_WINDOW);
    CHECK_INT(CTRL(0x18), CMD_INSTALL);
    CHECK_INT(CTRL(0x11), CMD_CLOSE);
    CHECK_INT(CTRL(0x10), CMD_QUIT);
    CHECK_INT(CTRL(0x02), CMD_VIEW_1);
    CHECK_INT(CTRL(0x03), CMD_VIEW_2);
    CHECK_INT(CTRL(0x04), CMD_VIEW_3);
    CHECK_INT(CTRL(0x05), CMD_NONE);
    CHECK_INT(PLAIN(0x01), CMD_BACK);
    CHECK_INT(PLAIN(0x1C), CMD_OPEN);
    CHECK_INT(PLAIN(0x72), CMD_OPEN);
    CHECK_INT(PLAIN(0x31), CMD_NONE);
    /* the character does not matter, the scan code does */
    CHECK_INT(command_from_key(0x310E, KMETA_CTRL), CMD_NEW_WINDOW);
}

static void test_winstate(void)
{
    WinState ws;

    winstate_init(&ws);
    CHECK_INT(ws.mode, MODE_OVERVIEW);
    CHECK_INT(ws.view, VIEW_ICONS);
    CHECK_INT(ws.zoom_pt, ZOOM_DEFAULT_PT);
    CHECK(winstate_list_enabled(&ws));
    CHECK(!winstate_style_enabled(&ws));

    CHECK(winstate_apply(&ws, CMD_VIEW_3));
    CHECK_INT(ws.view, VIEW_LIST);
    CHECK(!winstate_apply(&ws, CMD_VIEW_3));            /* already there */
    CHECK(winstate_apply(&ws, CMD_VIEW_2));
    CHECK_INT(ws.view, VIEW_SAMPLES);

    ws.scroll_y = 90;
    CHECK(winstate_apply(&ws, CMD_VIEW_1));
    CHECK_INT(ws.scroll_y, 0);                          /* a new view starts at the top */

    CHECK(!winstate_apply(&ws, CMD_OPEN));              /* nothing selected */
    CHECK(!winstate_apply(&ws, CMD_BACK));              /* not in a single font */

    winstate_apply(&ws, CMD_VIEW_2);                    /* Samples, then open a family */
    ws.selected = 3;
    CHECK(winstate_apply(&ws, CMD_OPEN));
    CHECK_INT(ws.mode, MODE_SINGLE);
    CHECK_INT(ws.view, VIEW_SPECIMEN);
    CHECK_INT(ws.family, 3);
    CHECK(!winstate_list_enabled(&ws));
    CHECK(winstate_style_enabled(&ws));

    CHECK(!winstate_apply(&ws, CMD_VIEW_3));            /* no list in a single font */
    CHECK_INT(ws.view, VIEW_SPECIMEN);
    CHECK(winstate_apply(&ws, CMD_VIEW_2));
    CHECK_INT(ws.view, VIEW_FAMILY_SAMPLES);
    CHECK(!winstate_style_enabled(&ws));                /* only Specimen picks a style */
    CHECK(winstate_apply(&ws, CMD_VIEW_1));
    CHECK_INT(ws.view, VIEW_SPECIMEN);

    CHECK(winstate_apply(&ws, CMD_BACK));
    CHECK_INT(ws.mode, MODE_OVERVIEW);
    CHECK_INT(ws.view, VIEW_SAMPLES);                   /* back where it came from */
    CHECK_INT(ws.selected, 3);

    winstate_set_zoom(&ws, 3);
    CHECK_INT(ws.zoom_pt, ZOOM_MIN_PT);
    winstate_set_zoom(&ws, 500);
    CHECK_INT(ws.zoom_pt, ZOOM_MAX_PT);
    winstate_set_zoom(&ws, 30);
    CHECK_INT(ws.zoom_pt, 30);
}

static void test_rect(void)
{
    Rect a = { 0, 0, 10, 10 }, b = { 5, 5, 10, 10 }, c = { 10, 0, 5, 5 }, r;

    CHECK(rect_intersect(&a, &b, &r));
    CHECK_INT(r.x, 5);
    CHECK_INT(r.y, 5);
    CHECK_INT(r.w, 5);
    CHECK_INT(r.h, 5);
    CHECK(!rect_intersect(&a, &c, &r));                 /* they only touch */
    CHECK(rect_contains(&a, 0, 0));
    CHECK(rect_contains(&a, 9, 9));
    CHECK(!rect_contains(&a, 10, 5));
    CHECK(!rect_contains(&a, -1, 5));
}

static void test_initial(void)
{
    Rect big = { 0, 19, 1280, 781 }, small = { 0, 11, 320, 189 }, r;

    /* a big screen: 800x600, centred */
    r = rect_initial(&big, 0, 800, 600, 12);
    CHECK_INT(r.w, 800);
    CHECK_INT(r.h, 600);
    CHECK_INT(r.x, 240);
    CHECK_INT(r.y, 19 + 90);
    /* the cascade moves the next ones down and right, six deep, then starts again */
    r = rect_initial(&big, 1, 800, 600, 12);
    CHECK_INT(r.x, 252);
    CHECK_INT(r.y, 19 + 90 + 12);
    r = rect_initial(&big, 6, 800, 600, 12);
    CHECK_INT(r.x, 240);
    /* a small screen: the whole desktop, shrinking with the cascade to stay on it */
    r = rect_initial(&small, 0, 800, 600, 12);
    CHECK(r.x == 0 && r.y == 11 && r.w == 320 && r.h == 189);
    r = rect_initial(&small, 2, 800, 600, 12);
    CHECK(r.x == 24 && r.y == 35 && r.w == 296 && r.h == 165);
    /* always inside the desktop */
    {
        int c;

        for (c = 0; c < 12; c++) {
            r = rect_initial(&big, c, 800, 600, 12);
            CHECK(r.x >= big.x && r.y >= big.y && r.x + r.w <= big.x + big.w && r.y + r.h <= big.y + big.h);
        }
    }
}

static void test_toolbar(void)
{
    ToolbarLayout tl;
    Rect th;
    int i, pt;

    /* an overview on a 320 wide screen: three view buttons and a slider */
    toolbar_layout(&tl, 300, 8, MODE_OVERVIEW);
    CHECK_INT(tl.icon, 16);
    CHECK_INT(tl.height, 24);
    CHECK(!tl.present[TB_BACK]);
    CHECK(!tl.present[TB_STYLE]);
    CHECK(tl.present[TB_VIEW1] && tl.present[TB_VIEW2] && tl.present[TB_VIEW3]);
    CHECK(tl.present[TB_SLIDER]);
    CHECK(tl.rect[TB_VIEW2].x > tl.rect[TB_VIEW1].x + tl.rect[TB_VIEW1].w - 1);
    CHECK(tl.rect[TB_SLIDER].x > tl.rect[TB_VIEW3].x + tl.rect[TB_VIEW3].w);
    CHECK(tl.rect[TB_SLIDER].x + tl.rect[TB_SLIDER].w <= 300 - 4 + 0);
    CHECK_INT(toolbar_hit(&tl, tl.rect[TB_VIEW2].x + 1, 4), TB_VIEW2);
    CHECK_INT(toolbar_hit(&tl, 0, 0), TB_NONE);

    /* a single font: Back, the style pulldown, two views, and no List */
    toolbar_layout(&tl, 300, 8, MODE_SINGLE);
    CHECK(tl.present[TB_BACK] && tl.present[TB_STYLE]);
    CHECK(tl.present[TB_VIEW1] && tl.present[TB_VIEW2] && !tl.present[TB_VIEW3]);
    CHECK_INT(tl.rect[TB_STYLE].w, 12 * 8 + 4);
    CHECK(tl.present[TB_SLIDER]);
    CHECK_INT(toolbar_hit(&tl, tl.rect[TB_BACK].x, 2), TB_BACK);

    /* nothing overlaps, and everything is inside the toolbar */
    for (i = 0; i < TB_COUNT; i++) {
        int j;
        if (!tl.present[i])
            continue;
        CHECK(tl.rect[i].x >= 0 && tl.rect[i].x + tl.rect[i].w <= 300);
        CHECK(tl.rect[i].y >= 0 && tl.rect[i].y + tl.rect[i].h <= tl.height);
        for (j = i + 1; j < TB_COUNT; j++) {
            Rect x;
            if (tl.present[j])
                CHECK(!rect_intersect(&tl.rect[i], &tl.rect[j], &x));
        }
    }

    /* the slider: the ends of the track are the ends of the zoom range */
    CHECK_INT(toolbar_zoom_at(&tl, 0), ZOOM_MIN_PT);
    CHECK_INT(toolbar_zoom_at(&tl, 10000), ZOOM_MAX_PT);
    th = toolbar_thumb(&tl, ZOOM_MIN_PT);
    CHECK_INT(th.x, tl.rect[TB_SLIDER].x + 1);
    th = toolbar_thumb(&tl, ZOOM_MAX_PT);
    CHECK_INT(th.x + th.w, tl.rect[TB_SLIDER].x + tl.rect[TB_SLIDER].w - 1);
    for (pt = ZOOM_MIN_PT; pt <= ZOOM_MAX_PT; pt += 8) {
        th = toolbar_thumb(&tl, pt);
        CHECK_INT(toolbar_zoom_at(&tl, th.x + th.w / 2), pt);   /* a round trip */
    }

    /* every window width the AES can give: what is present fits, in both modes */
    {
        int w, m;

        for (w = 104; w <= 640; w += 4) {
            for (m = MODE_OVERVIEW; m <= MODE_SINGLE; m++) {
                toolbar_layout(&tl, w, 8, (WinMode)m);
                for (i = 0; i < TB_COUNT; i++) {
                    if (!tl.present[i])
                        continue;
                    CHECK(tl.rect[i].x >= 0);
                    CHECK(tl.rect[i].x + tl.rect[i].w <= w);
                }
                CHECK(tl.present[TB_VIEW1] && tl.present[TB_VIEW2]);
                CHECK(m == MODE_OVERVIEW ? tl.present[TB_VIEW3] : tl.present[TB_BACK] && tl.present[TB_STYLE]);
            }
        }
    }

    /* a narrow window gets the small bitmaps; one too narrow loses the slider */
    toolbar_layout(&tl, 200, 8, MODE_OVERVIEW);
    CHECK_INT(tl.icon, 8);
    CHECK_INT(tl.height, 16);
    toolbar_layout(&tl, 60, 8, MODE_OVERVIEW);
    CHECK(!tl.present[TB_SLIDER]);
}

void test_ui(void)
{
    test_commands();
    test_winstate();
    test_rect();
    test_initial();
    test_toolbar();
}
