#include <string.h>
#include <gem.h>
#include <gemx.h>
#include "fontdb/group.h"
#include "vdi_if/gem_source.h"

#define XFNT_FLAGS_NAMES  0x07  /* full name, family name, style name */

static int gem_face_count(void *ctx)
{
    return ((GemSource *)ctx)->count;
}

/*
 * vqt_ext_name() and vqt_xfntinfo() are NVDI/SpeedoGDOS calls; a VDI without
 * them returns without filling anything in.  So each is probed on face 1 and
 * trusted only if it agrees with vqt_name().
 */
static int probe_ext(const GemSource *gs)
{
    char name[34];
    short format = 0;
    short flags = 0;
    short id1 = vqt_name(gs->handle, 1, name);
    short id2 = vqt_ext_name(gs->handle, 1, name, &format, &flags);

    /* a VDI without the call leaves the id it was preset to, 0, so the ids
     * differ; a vector face may legitimately report format 0 in the short
     * reply, so the format is not part of the test */
    return id1 > 0 && id1 == id2;
}

static int probe_xfnt(const GemSource *gs)
{
    char name[34];
    XFNT_INFO info;
    short id = vqt_name(gs->handle, 1, name);

    memset(&info, 0, sizeof info);
    return id > 0 &&
           vqt_xfntinfo(gs->handle, XFNT_FLAGS_NAMES, id, 0, &info) == id &&
           info.family_name[0] != '\0';
}

static int gem_face_get(void *ctx, int index, VdiFace *out)
{
    GemSource *gs = ctx;
    char name[34];
    short format = 0;
    short flags = 0;
    short id;

    memset(out, 0, sizeof *out);
    memset(name, 0, sizeof name);
    if (gs->have_ext)
        id = vqt_ext_name(gs->handle, (short)index, name, &format, &flags);
    else
        id = vqt_name(gs->handle, (short)index, name);
    if (id <= 0)
        return -1;

    /* vqt_name() reports no format, and vqt_ext_name() gives 0 for a vector
     * face in its short (34 word) reply: the bitmap/vector flag in name[32]
     * decides in both cases */
    if (format == 0)
        format = name[32] != 0 ? FONT_FORMAT_VECTOR : FONT_FORMAT_BITMAP;
    name[32] = '\0';

    out->id = id;
    out->format = (unsigned short)format;
    out->flags = (unsigned short)flags;
    memcpy(out->name, name, 33);

    if (gs->have_xfnt) {
        XFNT_INFO info;

        memset(&info, 0, sizeof info);
        if (vqt_xfntinfo(gs->handle, XFNT_FLAGS_NAMES, id, 0, &info) == id) {
            strncpy(out->family, info.family_name, sizeof out->family - 1);
            strncpy(out->style, info.style_name, sizeof out->style - 1);
        }
    }
    return 0;
}

int gem_source_open(GemSource *gs, VdiFontSource *out)
{
    short work_in[11];
    short work_out[57];
    short cw, ch, cell_w, cell_h;
    short extra = 0;
    int i;

    memset(gs, 0, sizeof *gs);
    gs->handle = graf_handle(&cw, &ch, &cell_w, &cell_h);
    for (i = 0; i < 10; i++)
        work_in[i] = 1;
    work_in[10] = 2;                    /* raster coordinates */
    v_opnvwk(work_in, &gs->handle, work_out);
    if (gs->handle == 0)
        return -1;

    /*
     * Only with a GDOS, and only once: a second call returns 0 until
     * vst_unload_fonts().  work_out[10] is the number of faces without them.
     */
    if (vq_gdos() != 0)
        extra = vst_load_fonts(gs->handle, 0);
    gs->count = (short)(work_out[10] + extra);
    gs->have_ext = probe_ext(gs);
    gs->have_xfnt = probe_xfnt(gs);

    out->face_count = gem_face_count;
    out->face_get = gem_face_get;
    out->ctx = gs;
    return 0;
}

void gem_source_close(GemSource *gs)
{
    if (gs->handle != 0)
        v_clsvwk(gs->handle);
    gs->handle = 0;
}
