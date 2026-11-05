/* Host-side native VDI mock. Pointer fields use host pointers so the adapter
 * can run with real GEMlib headers on a host without an x32 execution ABI.
 * The production build uses pTOS's vdi_user.h and its layout assertions. */
#ifndef TYPESHELF_TEST_VDI_USER_H
#define TYPESHELF_TEST_VDI_USER_H

typedef struct {
    void *fd_addr;
    short fd_w, fd_h, fd_wdwidth, fd_stand, fd_nplanes;
    short fd_r1, fd_r2, fd_r3;
} VDI_USER_FDB;

typedef struct {
    short code, nptsin, nptsout, nintin, nintout, subcode, handle, reserved;
    union {
        struct { VDI_USER_FDB *source, *destination; } raster;
    } args;
} VDI_USER_CONTROL;

typedef struct {
    VDI_USER_CONTROL *contrl;
    short *intin, *ptsin, *intout, *ptsout;
} VDI_USER_PB;

long vdi(VDI_USER_PB *pb);
#endif
