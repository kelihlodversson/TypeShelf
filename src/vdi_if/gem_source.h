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
    int have_ext;       /* vqt_ext_name() answers (NVDI, SpeedoGDOS) */
    int have_arbpt;     /* vst_arbpt() sizes a vector face continuously */
    int have_xfnt;      /* vqt_xfntinfo() answers (NVDI 3.02 and later) */
} GemSource;

/*
 * Open a virtual workstation, load the GDOS fonts (once) and describe the
 * faces through out.  0 on success, -1 if no workstation could be opened.
 */
int gem_source_open(GemSource *gs, VdiFontSource *out);

void gem_source_close(GemSource *gs);

#endif
