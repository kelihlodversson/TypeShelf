#include <stdio.h>
#include <string.h>
#include "gdosinfo/decode.h"

const char *gdos_vgdos_name(long value)
{
    switch (value) {
    case GDOS_NONE:     return "no GDOS";
    case 0x0007E88AL:   return "Atari GDOS 1.1";
    case 0x0007E864L:   return "AMC-GDOS";
    case 0x0007E8BAL:   return "GEMINI special GDOS";
    case 0x3E5D0957L:   return "ttf-gdos";
    case 0x5F464E54L:   return "FONTGDOS";
    case 0x5F46534DL:   return "FSMGDOS";
    case 0x66564449L:   return "fVDI";
    default:            return "GDOS 1.0 or 1.2";
    }
}

const char *gdos_fsmc_name(const char magic[4])
{
    if (memcmp(magic, "_SPD", 4) == 0)
        return "SpeedoGDOS";
    if (memcmp(magic, "_FSM", 4) == 0)
        return "FSMGDOS";
    if (memcmp(magic, "_FNT", 4) == 0)
        return "FONTGDOS";
    if (memcmp(magic, "TTFG", 4) == 0)
        return "ttf-gdos";
    return "unknown vector GDOS";
}

void gdos_format_version(const char *label, unsigned short bcd, char *out, size_t cap)
{
    snprintf(out, cap, "%s %d.%02x", label, (int)(bcd >> 8), (unsigned)(bcd & 0xff));
}

void gdos_format_fsmc(const char magic[4], short version, short quality,
                      char *out, size_t cap)
{
    snprintf(out, cap, "%s %d.%02x, quality %d", gdos_fsmc_name(magic),
             (int)((version >> 8) & 0xff), (unsigned)(version & 0xff), (int)quality);
}

void gdos_format_os(unsigned short os_version, int is_etos, char *out, size_t cap)
{
    if (is_etos)
        snprintf(out, cap, "EmuTOS or pTOS (TOS %d.%02x compatible)",
                 (int)(os_version >> 8), (unsigned)(os_version & 0xff));
    else
        snprintf(out, cap, "TOS %d.%02x",
                 (int)(os_version >> 8), (unsigned)(os_version & 0xff));
}
