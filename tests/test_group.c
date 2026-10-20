#include "test.h"
#include "fontdb/group.h"

static FontFace face(short id, unsigned short format, const char *name)
{
    FontFace f;

    f.id = id;
    f.format = format;
    f.flags = 0;
    f.full_name = name;
    f.family = NULL;
    f.style = NULL;
    return f;
}

void test_group(void)
{
    FontList list;
    FontFace f;
    const FontFamily *fam;

    fontlist_init(&list);
    CHECK_INT(list.family_count, 0);

    f = face(11, FONT_FORMAT_BITMAP, "Swiss Bold");
    CHECK_INT(fontlist_add(&list, &f), 0);
    f = face(12, FONT_FORMAT_BITMAP, "Dutch");
    CHECK_INT(fontlist_add(&list, &f), 0);
    f = face(13, FONT_FORMAT_BITMAP, "swiss");
    CHECK_INT(fontlist_add(&list, &f), 0);
    f = face(14, FONT_FORMAT_TRUETYPE, "Swiss Bold Italic");
    CHECK_INT(fontlist_add(&list, &f), 0);
    f = face(15, FONT_FORMAT_TRUETYPE, "Swiss Light");
    CHECK_INT(fontlist_add(&list, &f), 0);

    /* "swiss" and "Swiss" are the same family */
    CHECK_INT(list.family_count, 2);

    fontlist_sort(&list);
    CHECK_STR(list.families[0].name, "Dutch");
    CHECK_STR(list.families[1].name, "Swiss");

    fam = fontlist_find(&list, "SWISS");
    CHECK(fam != NULL);
    if (fam != NULL) {
        CHECK_INT(fam->style_count, 4);
        /* Light, Regular, Bold, Bold Italic */
        CHECK_STR(fam->styles[0].style, "Light");
        CHECK_STR(fam->styles[1].style, "Regular");
        CHECK_STR(fam->styles[2].style, "Bold");
        CHECK_STR(fam->styles[3].style, "Bold Italic");
        CHECK_INT(fam->styles[3].id, 14);
        CHECK_INT(fam->formats, FONT_FORMAT_BITMAP | FONT_FORMAT_TRUETYPE);
    }
    CHECK(fontlist_find(&list, "Nothing") == NULL);

    /* an explicit family and style win over the full name */
    f = face(20, FONT_FORMAT_SPEEDO, "DejaVu Sans Mono");
    f.family = "DejaVu Sans Mono";
    f.style = "Book";
    CHECK_INT(fontlist_add(&list, &f), 0);
    fam = fontlist_find(&list, "DejaVu Sans Mono");
    CHECK(fam != NULL);
    if (fam != NULL)
        CHECK_STR(fam->styles[0].style, "Book");

    /* a family without a style is not trusted: the name is split instead */
    f = face(21, FONT_FORMAT_SPEEDO, "Foo Bold");
    f.family = "Foo Bold";
    f.style = "";
    CHECK_INT(fontlist_add(&list, &f), 0);
    CHECK(fontlist_find(&list, "Foo") != NULL);

    CHECK_INT(font_style_rank("Regular"), 6);
    CHECK_INT(font_style_rank("Italic"), 7);
    CHECK_INT(font_style_rank("Bold Italic"), 13);
    CHECK_INT(font_style_rank("Thin"), 0);
    CHECK_INT(font_style_rank("Whatever"), 6);

    fontlist_free(&list);
    CHECK_INT(list.family_count, 0);
    CHECK(list.families == NULL);
}
