/*
 * legacy.c (m68k) - cookie jar and OS header, with calls every TOS has
 * (Setexc() and Super()), for when there is no Ssystem()
 */
#include <mint/osbind.h>
#include "platform/legacy.h"

#define P_COOKIES   0x5A0L          /* system variable: the cookie jar */
#define SYSBASE     0x4F2L          /* system variable: the OS header */

int legacy_get_cookie(long id, long *value)
{
    /* Setexc() with a vector number below 0x100 would be an exception vector;
     * 0x5A0 / 4 is the system variable's address as a vector number */
    const long *jar = (const long *)Setexc((short)(P_COOKIES / 4), -1L);

    if (jar == 0 || (long)jar == -1L)
        return -1;
    for (; jar[0] != 0; jar += 2) {
        if (jar[0] == id) {
            if (value != 0)
                *value = jar[1];
            return 0;
        }
    }
    return -1;
}

int legacy_get_os(unsigned short *os_version, int *is_etos)
{
    const unsigned char *header;
    long ssp = Super(0L);

    /* the first word of the OS header is a branch; os_beg is at 8, the
     * version at 2 and the EmuTOS marker at 0x2C */
    header = *(const unsigned char *const *)SYSBASE;
    header = *(const unsigned char *const *)(header + 8);
    *os_version = (unsigned short)((header[2] << 8) | header[3]);
    *is_etos = header[0x2C] == 'E' && header[0x2D] == 'T' &&
               header[0x2E] == 'O' && header[0x2F] == 'S';

    SuperToUser(ssp);
    return 0;
}
