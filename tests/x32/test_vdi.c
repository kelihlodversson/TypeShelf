#define vdi typeshelf_native_vdi
#include "vdi_user.h"
#undef vdi
#include "gem_vdiP.h"
#include "gdosinfo/decode.h"
#undef CHECK
#include "test.h"

int test_checks, test_failures;
static int fail_open, fail_close, native_calls;

long typeshelf_native_vdi(VDI_USER_PB *pb)
{
    int i;

    native_calls++;
    if (pb->contrl->code == 100) {
        if (fail_open)
            return -39;                /* outputs and physical handle untouched */
        pb->contrl->handle = 7;
        pb->contrl->nintout = 45;
        pb->contrl->nptsout = 6;
        for (i = 0; i < 45; i++) pb->intout[i] = (short)(100 + i);
        for (i = 0; i < 12; i++) pb->ptsout[i] = (short)(200 + i);
        pb->intout[0] = 1279;
        pb->intout[1] = 799;
        return 0;
    }
    if (pb->contrl->code == 101)
        return fail_close ? -32 : 0;
    return -32;                         /* native vq_extnd(0) is unsupported */
}

int main(void)
{
    short control[15] = { 0 }, intin[11] = { 0 };
    short intout[45], ptsout[12];
    VDIPB pb = { control, intin, 0, intout, ptsout };
    int i, calls;

    /* GEMlib must see handle zero, even though native leaves physical 1. */
    control[0] = 100; control[6] = 1;
    fail_open = 1;
    vdi(&pb);
    CHECK_INT(control[6], 0);
    CHECK_INT(control[2], 0);
    CHECK_INT(control[4], 0);

    fail_open = 0; control[6] = 1;
    vdi(&pb);
    CHECK_INT(control[6], 7);
    CHECK_INT(control[4], 45);
    CHECK_INT(control[2], 6);

    /* Query all opening capabilities, including screen dimensions, after
     * destroying caller buffers. The rejected native query must not run. */
    memset(intout, 0, sizeof intout);
    memset(ptsout, 0, sizeof ptsout);
    control[0] = 102;
    calls = native_calls;
    vdi(&pb);
    CHECK_INT(native_calls, calls);
    CHECK_INT(intout[0] + 1, 1280);
    CHECK_INT(intout[1] + 1, 800);
    for (i = 2; i < 45; i++) CHECK_INT(intout[i], 100 + i);
    for (i = 0; i < 12; i++) CHECK_INT(ptsout[i], 200 + i);
    CHECK_INT(control[4], 45);
    CHECK_INT(control[2], 6);

    /* Unrelated handles cannot inherit cached capabilities. */
    control[6] = 8;
    vdi(&pb);
    CHECK_INT(native_calls, calls + 1);
    CHECK_INT(control[4], 0);

    /* A rejected close keeps the cache; a successful close invalidates it. */
    control[6] = 7; control[0] = 101; fail_close = 1;
    vdi(&pb);
    control[0] = 102;
    calls = native_calls;
    vdi(&pb);
    CHECK_INT(native_calls, calls);
    control[0] = 101; fail_close = 0;
    vdi(&pb);
    control[0] = 102;
    calls = native_calls;
    vdi(&pb);
    CHECK_INT(native_calls, calls + 1);

    calls = native_calls;
    CHECK_INT(vq_vgdos(), GDOS_UNKNOWN);
    CHECK_INT(vq_gdos(), 0);
    CHECK_INT(native_calls, calls);
    printf("x32 VDI adapter: %d checks, %d failures\n", test_checks, test_failures);
    return test_failures != 0;
}
