/* Adapt GEMlib's packed control words to the native pTOS VDI entry.
 * The application continues to use the standard GEMlib VDI API. */
#define vdi typeshelf_native_vdi
#include "vdi_user.h"
#undef vdi
#include "gem_vdiP.h"
#include "gdosinfo/decode.h"

_Static_assert(sizeof(MFDB) == sizeof(VDI_USER_FDB), "native MFDB layout");

/* All TypeShelf windows share one virtual screen workstation. Keep its
 * opening capabilities for vq_extnd(0), which the native client rejects. */
static short screen_handle;
static short screen_intout[45], screen_ptsout[12];

void vdi(VDIPB *pb)
{
    VDI_USER_CONTROL control = { 0 };
    VDI_USER_PB native;
    int i;
    long result;

    control.code = pb->control[0];
    control.nptsin = pb->control[1];
    control.nintin = pb->control[3];
    control.subcode = pb->control[5];
    control.handle = pb->control[6];
    if (control.code == 109 || control.code == 110 || control.code == 121) {
        control.args.raster.source = vdi_control_get_ptr(pb->control, 0);
        control.args.raster.destination = vdi_control_get_ptr(pb->control, 1);
    }
    native.contrl = &control;
    native.intin = NO_CONST(pb->intin);
    native.ptsin = NO_CONST(pb->ptsin);
    native.intout = pb->intout;
    native.ptsout = pb->ptsout;
    if (control.code == 102 && pb->intin[0] == 0 &&
        screen_handle != 0 && control.handle == screen_handle) {
        for (i = 0; i < 45; i++)
            pb->intout[i] = screen_intout[i];
        for (i = 0; i < 12; i++)
            pb->ptsout[i] = screen_ptsout[i];
        control.nintout = 45;
        control.nptsout = 6;
        result = 0;
    } else {
        result = typeshelf_native_vdi(&native);
    }
    if (result < 0) {
        if (control.code == 100)
            control.handle = 0;
        /* These optional font calls are rejected by current pTOS x32.
         * Leave deterministic probe results, as a legacy VDI would. */
        if (control.code == 119 || control.code == 246)
            pb->intout[0] = 0;
        if (control.code == 229) {
            pb->intout[0] = 0;
            pb->intout[1] = 0;
            pb->intout[2] = 0;
        }
        if (control.code == 246) {
            pb->ptsout[0] = pb->ptsout[1] = 0;
            pb->ptsout[2] = pb->ptsout[3] = 0;
        }
    }
    if (control.code == 100 && result >= 0 && control.handle != 0) {
        screen_handle = control.handle;
        for (i = 0; i < 45; i++)
            screen_intout[i] = pb->intout[i];
        for (i = 0; i < 12; i++)
            screen_ptsout[i] = pb->ptsout[i];
    }
    if (control.code == 101 && result >= 0 && control.handle == screen_handle)
        screen_handle = 0;
    pb->control[2] = control.nptsout;
    pb->control[4] = control.nintout;
    pb->control[6] = control.handle;
}

/* pTOS master rejects trap-class-2 inquiries from ring 3. Until a supported
 * GDOS inquiry exists, distinguish unavailable information from no GDOS. */
long vq_vgdos(void)
{
    return GDOS_UNKNOWN;
}

short vq_gdos(void)
{
    return 0;
}
