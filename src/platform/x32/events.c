/* GEMlib's fast event interface, using the shared AES entry. */
#include <gem.h>

short mt_evnt_multi_fast(const EVMULT_IN *in, short msg[], EVMULT_OUT *out,
                         short *global)
{
    out->emo_events = mt_evnt_multi(in->emi_flags, in->emi_bclicks,
        in->emi_bmask, in->emi_bstate,
        in->emi_m1leave, in->emi_m1.g_x, in->emi_m1.g_y,
        in->emi_m1.g_w, in->emi_m1.g_h,
        in->emi_m2leave, in->emi_m2.g_x, in->emi_m2.g_y,
        in->emi_m2.g_w, in->emi_m2.g_h, msg,
        (unsigned short)in->emi_tlow | ((unsigned long)(unsigned short)in->emi_thigh << 16),
        &out->emo_mouse.p_x, &out->emo_mouse.p_y, &out->emo_mbutton,
        &out->emo_kmeta, &out->emo_kreturn, &out->emo_mclicks, global);
    return out->emo_events;
}
