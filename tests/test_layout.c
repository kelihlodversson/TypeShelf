#include "test.h"
#include "layout/layout.h"

void test_layout(void)
{
    RowRange r;
    int pt, pos, prev;

    /* columns: 320 wide, 64 wide tiles, 8 gap: 4 fit (4*64 + 3*8 = 280) */
    CHECK_INT(layout_columns(320, 64, 8), 4);
    CHECK_INT(layout_columns(280, 64, 8), 4);      /* exactly 4 tiles and 3 gaps */
    CHECK_INT(layout_columns(279, 64, 8), 3);
    CHECK_INT(layout_columns(50, 64, 8), 1);
    CHECK_INT(layout_columns(0, 64, 8), 1);
    CHECK_INT(layout_columns(320, 0, 8), 1);
    CHECK_INT(layout_rows(10, 4), 3);
    CHECK_INT(layout_rows(8, 4), 2);
    CHECK_INT(layout_rows(0, 4), 0);
    CHECK_INT(layout_rows(5, 0), 0);

    /* 10 rows of 20 px in a 50 px view */
    r = layout_visible_rows(0, 50, 20, 10);
    CHECK_INT(r.first, 0);
    CHECK_INT(r.end, 3);
    r = layout_visible_rows(30, 50, 20, 10);       /* rows 1 (partly) to 4 */
    CHECK_INT(r.first, 1);
    CHECK_INT(r.end, 4);
    r = layout_visible_rows(190, 50, 20, 10);      /* the end of the list */
    CHECK_INT(r.first, 9);
    CHECK_INT(r.end, 10);
    r = layout_visible_rows(1000, 50, 20, 10);     /* scrolled past the end */
    CHECK_INT(r.first, 10);
    CHECK_INT(r.end, 10);
    r = layout_visible_rows(0, 50, 20, 0);
    CHECK_INT(r.end, 0);
    r = layout_visible_rows(0, 50, 0, 10);
    CHECK_INT(r.end, 0);

    /* zoom: ends, middle, and a round trip at every size */
    CHECK_INT(zoom_from_slider(0, 200), ZOOM_MIN_PT);
    CHECK_INT(zoom_from_slider(200, 200), ZOOM_MAX_PT);
    CHECK_INT(zoom_from_slider(100, 200), 40);
    CHECK_INT(zoom_from_slider(-5, 200), ZOOM_MIN_PT);
    CHECK_INT(zoom_from_slider(999, 200), ZOOM_MAX_PT);
    CHECK_INT(zoom_from_slider(5, 0), ZOOM_MIN_PT);
    CHECK_INT(zoom_to_slider(ZOOM_MIN_PT, 200), 0);
    CHECK_INT(zoom_to_slider(ZOOM_MAX_PT, 200), 200);
    CHECK_INT(zoom_to_slider(1, 200), 0);
    CHECK_INT(zoom_to_slider(500, 200), 200);
    for (pt = ZOOM_MIN_PT; pt <= ZOOM_MAX_PT; pt++)
        CHECK_INT(zoom_from_slider(zoom_to_slider(pt, 200), 200), pt);

    /* a short track (ST-LOW): not every size is reachable, but it never goes down */
    prev = ZOOM_MIN_PT;
    for (pos = 0; pos <= 48; pos++) {
        int z = zoom_from_slider(pos, 48);

        CHECK(z >= prev);
        prev = z;
    }
    CHECK_INT(prev, ZOOM_MAX_PT);

    /* scroll slider: content 1000, view 250 */
    CHECK_INT(scroll_slider_size(1000, 250), 250);
    CHECK_INT(scroll_slider_size(200, 250), SLIDER_MAX);
    CHECK_INT(scroll_slider_size(100000, 1), 1);
    CHECK_INT(scroll_slider_pos(0, 1000, 250), 0);
    CHECK_INT(scroll_slider_pos(750, 1000, 250), SLIDER_MAX);
    CHECK_INT(scroll_slider_pos(9999, 1000, 250), SLIDER_MAX);
    CHECK_INT(scroll_slider_pos(375, 1000, 250), 500);
    CHECK_INT(scroll_slider_pos(100, 200, 250), 0);
    CHECK_INT(scroll_offset_from_slider(0, 1000, 250), 0);
    CHECK_INT(scroll_offset_from_slider(1, 1000, 250), 0);
    CHECK_INT(scroll_offset_from_slider(500, 1000, 250), 375);
    CHECK_INT(scroll_offset_from_slider(SLIDER_MAX, 1000, 250), 750);
    CHECK_INT(scroll_offset_from_slider(500, 200, 250), 0);
    /* very tall documents: these products do not fit in 32 bits */
    CHECK_INT(scroll_slider_pos(2500000, 3000000, 250), 833);
    CHECK_INT(scroll_offset_from_slider(900, 3000000, 250), 2699775);
    /* position 1 of a 1,000,000 pixel document is not the top */
    CHECK_INT(scroll_offset_from_slider(1, 1000000, 250), 999);
    /* the slider position and the offset agree to within a pixel step */
    pos = scroll_slider_pos(375, 1000, 250);
    CHECK(scroll_offset_from_slider(pos, 1000, 250) >= 374 - 1);
    CHECK(scroll_offset_from_slider(pos, 1000, 250) <= 375);
}
