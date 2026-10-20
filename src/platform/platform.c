/*
 * platform.c - cookies and the OS through Ssystem(), with the m68k fallback
 */
#include <stdint.h>
#include <mint/mintbind.h>
#include "platform/legacy.h"
#include "platform/platform.h"

#define E_OK 0

/* S_INQUIRE: does this Ssystem() know the mode?  0 on a system without
 * Ssystem() at all, where the GEMDOS call fails with EINVFN. */
static int ssystem_knows(int mode)
{
    return Ssystem(S_INQUIRE, mode, 0) == E_OK;
}

int platform_get_cookie(long id, long *value)
{
    long v = 0;

    if (!ssystem_knows(S_GETCOOKIE))
        return legacy_get_cookie(id, value);
    if (Ssystem(S_GETCOOKIE, id, (long)(uintptr_t)&v) != E_OK)
        return -1;
    if (value != 0)
        *value = v;
    return 0;
}

int platform_get_os(PlatformOs *os)
{
    os->name = 0;
    os->version = 0;
    os->tos_version = 0;
    os->is_etos = 0;

    if (!ssystem_knows(S_OSNAME))
        return legacy_get_os(&os->tos_version, &os->is_etos);

    os->name = Ssystem(S_OSNAME, 0, 0);
    if (ssystem_knows(S_OSVERSION))
        os->version = (unsigned long)Ssystem(S_OSVERSION, 0, 0);
    os->is_etos = os->name == (((long)'p' << 24) | ((long)'T' << 16) | ((long)'O' << 8) | 'S');
    return 0;
}
