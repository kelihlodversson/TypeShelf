/*
 * gem_source.h - the font source over the real VDI (gemlib)
 *
 * Not built on the host: it needs a screen workstation.
 */
#ifndef VDI_IF_GEM_SOURCE_H
#define VDI_IF_GEM_SOURCE_H

#include "vdi_if/vdi_if.h"

typedef struct {
    short handle;       /* virtual screen workstation */
    short count;        /* number of faces */
    short base_count;   /* the faces without the loaded fonts */
    int have_ext;       /* vqt_ext_name() answers (NVDI, SpeedoGDOS) */
    int have_arbpt;     /* vst_arbpt() sizes a vector face continuously */
    int have_ucs2;      /* vst_map_mode(MAP_UNICODE) works: Unicode text (NVDI 4, pTOS) */
    int have_xfnt;      /* vqt_xfntinfo() answers (NVDI 3.02 and later) */
} GemSource;

/*
 * Open a virtual workstation, load the GDOS fonts (once) and describe the
 * faces through out.  0 on success, -1 if no workstation could be opened.
 */
int gem_source_open(GemSource *gs, VdiFontSource *out);

/*
 * Unload and load the GDOS fonts again, so that fonts added since are seen,
 * and update gs->count.  The caller rebuilds its list from the source.
 */
void gem_source_reload(GemSource *gs);

void gem_source_close(GemSource *gs);

#endif
