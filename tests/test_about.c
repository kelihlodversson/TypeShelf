#include <string.h>
#include "test.h"
#include "gdosinfo/about.h"

void test_about(void)
{
    AboutFacts f;
    char lines[ABOUT_MAX_LINES][ABOUT_WIDTH + 1];
    int n, i, all_fit = 1;

    memset(&f, 0, sizeof f);
    strcpy(f.os, "pTOS 0.0.0a");
    f.vgdos = 0x5F46534DL;
    memcpy(f.fsmc_magic, "_SPD", 4);
    f.fsmc_version = 0x0400;
    f.fsmc_quality = -1;
    f.have_fsmc = 1;
    f.screen_w = 1280;
    f.screen_h = 800;
    f.planes = 32;
    f.families = 4;
    f.styles = 5;
    f.scalable_styles = 4;
    f.have_arbpt = 1;
    f.have_ucs2 = 1;
    n = about_format(&f, lines);
    CHECK(n > 8 && n <= ABOUT_MAX_LINES);
    CHECK_STR(lines[0], "Type Shelf");
    CHECK_STR(lines[3], "System: pTOS 0.0.0a");
    CHECK_STR(lines[4], "GDOS:   FSMGDOS");
    CHECK_STR(lines[5], "Vector: SpeedoGDOS 4.00, quality -1");
    CHECK_STR(lines[6], "Screen: 1280x800, 32-bit colour");
    CHECK_STR(lines[7], "Fonts:  4 families, 5 styles");
    CHECK_STR(lines[8], "        1 bitmap, 4 scalable");
    CHECK_STR(lines[9], "Sizes:  any point size");
    CHECK_STR(lines[10], "Text:   Unicode");
    for (i = 0; i < n; i++)
        if (strlen(lines[i]) > ABOUT_WIDTH)
            all_fit = 0;
    CHECK(all_fit);
    CHECK((ABOUT_WIDTH + 2 * ABOUT_PAD) * 8 + 8 <= 320);    /* the dialog and its outline fit an ST-LOW screen */

    /* a plain ST: no cookies, 16 colours, one family */
    memset(&f, 0, sizeof f);
    f.vgdos = -2L;
    f.screen_w = 320;
    f.screen_h = 200;
    f.planes = 4;
    f.families = 1;
    f.styles = 1;
    n = about_format(&f, lines);
    CHECK_STR(lines[3], "System: unknown");
    CHECK_STR(lines[4], "GDOS:   no GDOS");
    CHECK_STR(lines[5], "Screen: 320x200, 16 colours");
    CHECK_STR(lines[6], "Fonts:  1 family, 1 style");
    CHECK_STR(lines[8], "Sizes:  the sizes the fonts have");
    CHECK_STR(lines[9], "Text:   the Atari character set");
    CHECK_INT(n, 10);

    /* NVDI and EdDI add a line each */
    f.have_nvdi = 1;
    f.nvdi_bcd = 0x0410;
    f.have_eddi = 1;
    n = about_format(&f, lines);
    CHECK_STR(lines[5], "        NVDI 4.10");
    CHECK_STR(lines[6], "        EdDI present");
    CHECK_INT(n, 12);

    f.nvdi_bcd = 0;                                 /* a version that could not be read */
    about_format(&f, lines);
    CHECK_STR(lines[5], "        NVDI present");
}
