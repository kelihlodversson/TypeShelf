#include <string.h>
#include "test.h"
#include "fontdb/build.h"

typedef struct {
    short id;
    unsigned short format;
    const char *name;
    const char *family;
    const char *style;
    int missing;                /* face_get fails for this one */
} FakeFace;

static const FakeFace fake_faces[] = {
    { 1,  FONT_FORMAT_BITMAP,   "Arial Bold Italic", "", "", 0 },
    { 2,  FONT_FORMAT_BITMAP,   "Arial",             "", "", 0 },
    { 3,  FONT_FORMAT_BITMAP,   "Broken",            "", "", 1 },
    { 4,  FONT_FORMAT_TRUETYPE, "DejaVu Sans Mono",  "DejaVu Sans Mono", "Book", 0 },
    { 5,  FONT_FORMAT_BITMAP,   "Arial Bold",        "", "", 0 },
    { 6,  FONT_FORMAT_BITMAP,   "Dutch 801 Roman",   "", "", 0 }
};

#define FAKE_COUNT ((int)(sizeof fake_faces / sizeof fake_faces[0]))

static int fake_count(void *ctx)
{
    (void)ctx;
    return FAKE_COUNT;
}

static int fake_get(void *ctx, int index, VdiFace *out)
{
    const FakeFace *f;

    (void)ctx;
    if (index < 1 || index > FAKE_COUNT)
        return -1;
    f = &fake_faces[index - 1];
    if (f->missing)
        return -1;
    memset(out, 0, sizeof *out);
    out->id = f->id;
    out->format = f->format;
    strcpy(out->name, f->name);
    strcpy(out->family, f->family);
    strcpy(out->style, f->style);
    return 0;
}

static int no_faces(void *ctx)
{
    (void)ctx;
    return 0;
}

void test_build(void)
{
    VdiFontSource src;
    FontList list;
    const FontFamily *arial;

    src.face_count = fake_count;
    src.face_get = fake_get;
    src.ctx = NULL;

    CHECK_INT(fontdb_build(&src, &list), 5);        /* "Broken" is skipped */
    CHECK_INT(list.family_count, 3);
    CHECK_STR(list.families[0].name, "Arial");
    CHECK_STR(list.families[1].name, "DejaVu Sans Mono");
    CHECK_STR(list.families[2].name, "Dutch 801");

    arial = fontlist_find(&list, "Arial");
    CHECK(arial != NULL);
    if (arial != NULL) {
        CHECK_INT(arial->style_count, 3);
        CHECK_STR(arial->styles[0].style, "Regular");
        CHECK_STR(arial->styles[1].style, "Bold");
        CHECK_STR(arial->styles[2].style, "Bold Italic");
        CHECK_INT(arial->styles[2].id, 1);
    }
    CHECK_STR(list.families[1].styles[0].style, "Book");
    CHECK_INT(list.families[1].formats, FONT_FORMAT_TRUETYPE);
    fontlist_free(&list);

    src.face_count = no_faces;
    CHECK_INT(fontdb_build(&src, &list), 0);
    CHECK_INT(list.family_count, 0);
    fontlist_free(&list);
}
