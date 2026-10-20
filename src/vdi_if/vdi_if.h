/*
 * vdi_if.h - the seam between the font list and the VDI
 *
 * fontdb only ever sees fonts through a VdiFontSource, so it is tested on the
 * host with a fake one; vdi_if_gem.c is the real one, over gemlib.
 */
#ifndef VDI_IF_VDI_IF_H
#define VDI_IF_VDI_IF_H

/* one face, as reported by vqt_name(), vqt_ext_name() and vqt_xfntinfo() */
typedef struct {
    short id;
    unsigned short format;      /* FONT_FORMAT_* (see fontdb/group.h) */
    unsigned short flags;       /* vqt_ext_name(): bit 0 monospaced, bit 4 symbol */
    char name[34];              /* full name, always NUL-terminated */
    char family[52];            /* from vqt_xfntinfo(); "" if not available */
    char style[52];             /* from vqt_xfntinfo(); "" if not available */
} VdiFace;

typedef struct {
    int (*face_count)(void *ctx);

    /* index is 1..face_count; 0 on success, -1 if there is no such face */
    int (*face_get)(void *ctx, int index, VdiFace *out);

    void *ctx;
} VdiFontSource;

#endif
