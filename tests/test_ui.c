#include "test.h"
#include "layout/layout.h"
#include "ui/command.h"
#include "ui/rect.h"
#include "ui/toolbar_layout.h"
#include "ui/viewmodel.h"
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
    CHECK_INT(tl.rect[TB_SLIDER].x + tl.rect[TB_SLIDER].w, 300 - 4);       /* right aligned */
    toolbar_layout(&tl, 900, 8, MODE_OVERVIEW);
    CHECK_INT(tl.rect[TB_SLIDER].w, 160);                                  /* and not wider */
    CHECK_INT(tl.rect[TB_SLIDER].x + tl.rect[TB_SLIDER].w, 900 - 4);
    toolbar_layout(&tl, 300, 8, MODE_OVERVIEW);
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

    /* the style label never crosses the arrow, at any width */
    {
        int w, m = MODE_SINGLE;

        for (w = 104; w <= 640; w += 4) {
            int n;

            toolbar_layout(&tl, w, 8, (WinMode)m);
            n = toolbar_style_chars(&tl, 8);
            CHECK(n >= 0 && n <= 10);
            CHECK(3 + n * 8 + 2 <= tl.rect[TB_STYLE].w - 8 - 2);
        }
        toolbar_layout(&tl, 104, 8, MODE_SINGLE);
        CHECK_INT(toolbar_style_chars(&tl, 8), 3);              /* "Reg", not "Regular" */
        toolbar_layout(&tl, 400, 8, MODE_SINGLE);
        CHECK_INT(toolbar_style_chars(&tl, 8), 10);
        toolbar_layout(&tl, 400, 8, MODE_OVERVIEW);
        CHECK_INT(toolbar_style_chars(&tl, 8), 0);              /* no pulldown there */
    }

    /* a narrow window gets the small bitmaps; one too narrow loses the slider */
    toolbar_layout(&tl, 200, 8, MODE_OVERVIEW);
    CHECK_INT(tl.icon, 8);
    CHECK_INT(tl.height, 16);
    toolbar_layout(&tl, 60, 8, MODE_OVERVIEW);
    CHECK(!tl.present[TB_SLIDER]);
}


/* ---- the view model ---- */

static FontList fake_list(void)
{
    FontList list;
    const char *families[3] = { "Alpha", "Beta", "Gamma" };
    const char *styles[3] = { "Light", "Regular", "Bold" };
    int f, s;

    fontlist_init(&list);
    for (f = 0; f < 3; f++) {
        for (s = 0; s < (f == 1 ? 3 : 1); s++) {
            FontFace face;
            char name[64];

            snprintf(name, sizeof name, "%s %s", families[f], styles[f == 1 ? s : 1]);
            face.id = (short)(f * 10 + s);
            face.format = f == 2 ? FONT_FORMAT_TRUETYPE : FONT_FORMAT_BITMAP;
            face.flags = 0;
            face.full_name = name;
            face.family = families[f];
            face.style = styles[f == 1 ? s : 1];
            fontlist_add(&list, &face);
        }
    }
    fontlist_sort(&list);
    return list;
}

static void test_viewmodel(void)
{
    Grid g;
    Rect r;
    FontList list = fake_list();
    unsigned char expanded[3] = { 0, 0, 0 };
    ListRow row;
    char buf[64];
    WinState ws;

    /* the Unicode candidates run through the blocks in order */
    /* the repertoire walks the font's range, skipping what cannot be shown */
    CHECK_INT(repertoire_next(0, 255, -1, 0), 1);
    CHECK_INT(repertoire_next(0, 255, 31, 0), 33);          /* not the space */
    CHECK_INT(repertoire_next(33, 126, -1, 0), 33);
    CHECK_INT(repertoire_next(33, 126, 126, 0), -1);
    CHECK_INT(repertoire_next(8, 8, -1, 0), 8);             /* a font of one check mark */
    CHECK_INT(repertoire_next(8, 8, 8, 0), -1);
    CHECK_INT(repertoire_next(0x7E, 0xA0, 0x7E, 1), 0xA0);
    CHECK_INT(repertoire_next(0x7E, 0xA1, 0x7E, 1), 0xA0);  /* no C1 controls in Unicode */
    CHECK_INT(repertoire_next(0xD7FF, 0xE000, 0xD7FF, 1), 0xE000);
    CHECK_INT(repertoire_next(0xFFFE, 0x10010, 0xFFFE, 1), 0xFFFF);
    CHECK_INT(repertoire_next(0xFFFE, 0x10010, 0xFFFF, 1), -1);
    CHECK_INT(repertoire_next(10, 5, -1, 0), -1);
    CHECK_INT(repertoire_next(0, 255, -1, 1), 0x21);        /* Unicode has no C0 controls; the Atari set keeps its symbols */

    /* the popup sits under its anchor and stays on the screen */
    {
        Rect anchor = { 10, 20, 60, 16 };
        Rect screen = { 0, 0, 320, 200 };
        Rect pop = popup_rect(&anchor, 40, 10, 3, &screen);

        CHECK_INT(pop.x, 10);
        CHECK_INT(pop.y, 36);
        CHECK_INT(pop.w, 60);
        CHECK_INT(pop.h, 32);
        CHECK_INT(popup_row(&pop, 10, 12, 37), 0);
        CHECK_INT(popup_row(&pop, 10, 12, 47), 1);
        CHECK_INT(popup_row(&pop, 10, 12, 57), 2);
        CHECK_INT(popup_row(&pop, 10, 12, 36), -1);     /* the border */
        CHECK_INT(popup_row(&pop, 10, 5, 40), -1);
        CHECK_INT(popup_row(&pop, 10, 12, 80), -1);
        pop = popup_rect(&anchor, 100, 10, 3, &screen);
        CHECK_INT(pop.w, 100);
        anchor.x = 300;
        pop = popup_rect(&anchor, 100, 10, 3, &screen);
        CHECK_INT(pop.x, 220);                          /* pushed back inside */
        anchor.x = 10;
        anchor.y = 170;
        pop = popup_rect(&anchor, 40, 10, 30, &screen);
        CHECK_INT(pop.h, 2 + 10 * ((170 - 2) / 10));   /* more room above: it opens upwards */
        CHECK_INT(pop.y + pop.h, 170);
        anchor.y = 20;
        pop = popup_rect(&anchor, 40, 10, 30, &screen);
        CHECK_INT(pop.h, 2 + 10 * ((200 - 36 - 2) / 10));   /* only the rows that fit */
        anchor.y = 180;
        pop = popup_rect(&anchor, 40, 10, 1, &screen);
        CHECK(pop.y >= 0 && pop.y + pop.h <= 200);          /* inside the desktop */
        anchor.y = 195;
        pop = popup_rect(&anchor, 40, 10, 5, &screen);
        CHECK(pop.y >= 0 && pop.y + pop.h <= 200);
        CHECK(pop.h >= 12);                             /* at least one */
        CHECK_INT(popup_top(5, 8, 2), 0);               /* all fit */
        CHECK_INT(popup_top(30, 8, 0), 0);
        CHECK_INT(popup_top(30, 8, 15), 11);            /* the current one stays visible */
        CHECK_INT(popup_top(30, 8, 29), 22);
    }

    CHECK_INT(style_step(0, -1, 3), 0);
    CHECK_INT(style_step(1, 1, 3), 2);
    CHECK_INT(style_step(2, 1, 3), 2);
    CHECK_INT(style_step(0, 1, 0), 0);

    /* the title says what the window shows */
    winstate_init(&ws);
    window_title_text(&list, &ws, -1, buf, sizeof buf);
    CHECK_STR(buf, "TypeShelf: All Fonts");
    winstate_enter(&ws, 1);
    ws.view = VIEW_FAMILY_SAMPLES;
    window_title_text(&list, &ws, -1, buf, sizeof buf);
    CHECK_STR(buf, "TypeShelf: Beta, 3 styles");
    ws.view = VIEW_SPECIMEN;
    ws.style = 0;
    window_title_text(&list, &ws, -1, buf, sizeof buf);
    {
        char want[64];

        snprintf(want, sizeof want, "TypeShelf: Beta %s", list.families[1].styles[0].style);
        CHECK_STR(buf, want);
        window_title_text(&list, &ws, list.families[1].styles[0].id, buf, sizeof buf);
        snprintf(want, sizeof want, "TypeShelf: System Font %s", list.families[1].styles[0].style);
        CHECK_STR(buf, want);
    }

    /* a grid of 5 tiles, 50 x 40 with a gap of 10, in a 200 wide view: 3 columns */
    grid_make(&g, 200, 50, 40, 10, 5);
    CHECK_INT(g.cols, 3);
    CHECK_INT(g.rows, 2);
    CHECK_INT(grid_height(&g), 10 + 2 * 50);
    r = grid_tile(&g, 4);
    CHECK_INT(r.x, 10 + 1 * 60);
    CHECK_INT(r.y, 10 + 1 * 50);
    CHECK_INT(grid_hit(&g, 15, 15), 0);
    CHECK_INT(grid_hit(&g, 5, 15), -1);                 /* the left margin */
    CHECK_INT(grid_hit(&g, 62, 15), -1);                /* the gap between 0 and 1 */
    CHECK_INT(grid_hit(&g, 75, 15), 1);
    CHECK_INT(grid_hit(&g, 135, 15), 2);
    CHECK_INT(grid_hit(&g, 135, 75), -1);               /* below tile 2 there is none: 5 tiles */
    CHECK_INT(grid_hit(&g, 75, 75), 4);
    CHECK_INT(grid_hit(&g, 75, 300), -1);
    CHECK_INT(grid_hit(&g, 500, 15), -1);
    /* every tile is hit at its own corners */
    {
        int i;

        for (i = 0; i < 5; i++) {
            r = grid_tile(&g, i);
            CHECK_INT(grid_hit(&g, r.x, r.y), i);
            CHECK_INT(grid_hit(&g, r.x + r.w - 1, r.y + r.h - 1), i);
        }
    }

    /* keyboard moves */
    CHECK_INT(grid_move(&g, -1, 1, 0), 0);
    CHECK_INT(grid_move(&g, 0, 1, 0), 1);
    CHECK_INT(grid_move(&g, 2, 1, 0), 2);               /* the right edge */
    CHECK_INT(grid_move(&g, 0, -1, 0), 0);
    CHECK_INT(grid_move(&g, 1, 0, 1), 4);
    CHECK_INT(grid_move(&g, 2, 0, 1), 4);               /* nothing below 2: the last tile */
    CHECK_INT(grid_move(&g, 4, 0, -1), 1);
    CHECK_INT(grid_move(&g, 1, 0, -1), 1);              /* the top edge */
    CHECK_INT(grid_move(&g, 4, 1, 0), 4);               /* 5 is not a tile */
    grid_make(&g, 200, 50, 40, 10, 0);
    CHECK_INT(grid_move(&g, -1, 1, 0), -1);
    CHECK_INT(grid_height(&g), 10);
    grid_make(&g, 20, 50, 40, 10, 3);                   /* a view narrower than a tile: one column */
    CHECK_INT(g.cols, 1);

    /* rows */
    CHECK_INT(vlist_height(20, 4, 3), 4 + 3 * 24);
    CHECK_INT(vlist_top(20, 4, 2), 4 + 2 * 24);
    CHECK_INT(vlist_hit(20, 4, 3, 4), 0);
    CHECK_INT(vlist_hit(20, 4, 3, 23), 0);
    CHECK_INT(vlist_hit(20, 4, 3, 24), -1);             /* in the gap */
    CHECK_INT(vlist_hit(20, 4, 3, 28), 1);
    CHECK_INT(vlist_hit(20, 4, 3, 76), -1);             /* past the last: 4 + 72 is the end */
    CHECK_INT(vlist_hit(20, 4, 3, -5), -1);

    /* scrolling */
    CHECK_INT(scroll_to_show(0, 100, 20, 30), 0);       /* already in view */
    CHECK_INT(scroll_to_show(0, 100, 90, 30), 20);      /* below: just enough */
    CHECK_INT(scroll_to_show(80, 100, 10, 30), 10);     /* above */
    CHECK_INT(scroll_to_show(0, 100, 0, 300), 0);       /* too big: its start */
    CHECK_INT(scroll_clamp(500, 300, 100), 200);
    CHECK_INT(scroll_clamp(-5, 300, 100), 0);
    CHECK_INT(scroll_clamp(50, 60, 100), 0);            /* it all fits */

    /* the list: Alpha (1 style), Beta (3), Gamma (1) */
    CHECK_INT(list_row_count(&list, NULL), 3);
    CHECK_INT(list_row_count(&list, expanded), 3);
    expanded[1] = 1;
    CHECK_INT(list_row_count(&list, expanded), 6);
    row = list_row(&list, expanded, 0);
    CHECK(row.family == 0 && row.style == -1);
    row = list_row(&list, expanded, 1);
    CHECK(row.family == 1 && row.style == -1);
    row = list_row(&list, expanded, 2);
    CHECK(row.family == 1 && row.style == 0);
    row = list_row(&list, expanded, 4);
    CHECK(row.family == 1 && row.style == 2);
    row = list_row(&list, expanded, 5);
    CHECK(row.family == 2 && row.style == -1);
    row = list_row(&list, expanded, 6);
    CHECK_INT(row.family, -1);
    CHECK_INT(list_row_of_family(&list, expanded, 0), 0);
    CHECK_INT(list_row_of_family(&list, expanded, 2), 5);
    CHECK_INT(list_row_of_family(&list, NULL, 2), 2);

    /* the style a family is shown in: Regular, or the one nearest it */
    CHECK_INT(family_default_style(&list.families[1]), 1);      /* Light, Regular, Bold */
    CHECK_INT(family_default_style(&list.families[0]), 0);      /* its only style */

    font_format_text(FONT_FORMAT_BITMAP, buf, sizeof buf);
    CHECK_STR(buf, "Bitmap");
    font_format_text(FONT_FORMAT_TRUETYPE | FONT_FORMAT_TYPE1, buf, sizeof buf);
    CHECK_STR(buf, "TrueType, Type 1");
    font_format_text(FONT_FORMAT_VECTOR, buf, sizeof buf);
    CHECK_STR(buf, "Vector");
    font_format_text(FONT_FORMAT_VECTOR | FONT_FORMAT_SPEEDO, buf, sizeof buf);
    CHECK_STR(buf, "Speedo");
    font_format_text(0, buf, sizeof buf);
    CHECK_STR(buf, "-");
    font_format_text(FONT_FORMAT_BITMAP | FONT_FORMAT_SPEEDO | FONT_FORMAT_TRUETYPE | FONT_FORMAT_TYPE1, buf, 10);
    CHECK_INT(strlen(buf), 9);                          /* cut to fit, still a string */
    style_count_text(1, buf, sizeof buf);
    CHECK_STR(buf, "1 style");
    style_count_text(3, buf, sizeof buf);
    CHECK_STR(buf, "3 styles");

    CHECK_INT(command_from_key(0x4800, 0), CMD_UP);
    CHECK_INT(command_from_key(0x5000, 0), CMD_DOWN);
    CHECK_INT(command_from_key(0x4B00, 0), CMD_LEFT);
    CHECK_INT(command_from_key(0x4D00, 0), CMD_RIGHT);

    fontlist_free(&list);
}

void test_ui(void)
{
    test_commands();
    test_winstate();
    test_rect();
    test_initial();
    test_toolbar();
    test_viewmodel();
}
