#include "fontdb/build.h"

int fontdb_build(const VdiFontSource *src, FontList *out)
{
    int count = src->face_count(src->ctx);
    int i;
    int added = 0;
    VdiFace vf;
    FontFace ff;

    fontlist_init(out);
    for (i = 1; i <= count; i++) {
        if (src->face_get(src->ctx, i, &vf) != 0)
            continue;
        ff.id = vf.id;
        ff.format = vf.format;
        ff.flags = vf.flags;
        ff.full_name = vf.name;
        ff.family = vf.family;
        ff.style = vf.style;
        if (fontlist_add(out, &ff) != 0) {
            fontlist_free(out);
            return -1;
        }
        added++;
    }
    fontlist_sort(out);
    return added;
}
