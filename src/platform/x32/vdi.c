/* Adapt GEMlib's packed control words to the native pTOS VDI entry.
 * The application continues to use the standard GEMlib VDI API. */
#define vdi typeshelf_native_vdi
#include "vdi_user.h"
#undef vdi
#include "gem_vdiP.h"

_Static_assert(sizeof(MFDB) == sizeof(VDI_USER_FDB), "native MFDB layout");

void vdi(VDIPB *pb)
{
    VDI_USER_CONTROL control = { 0 };
    VDI_USER_PB native;

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
    if (typeshelf_native_vdi(&native) < 0) {
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
    pb->control[2] = control.nptsout;
    pb->control[4] = control.nintout;
    pb->control[6] = control.handle;
}

/* The GDOS inquiry is the special trap-2 function, outside the VDI PB API. */
long vq_vgdos(void)
{
    register unsigned long long result __asm__("rax") = (2ULL << 32) | 0xfffffffeU;

    __asm__ volatile ("syscall" : "+r"(result) : : "rcx", "r11", "memory", "cc");
    return (long)result;
}

short vq_gdos(void)
{
    return vq_vgdos() != -2;
}
