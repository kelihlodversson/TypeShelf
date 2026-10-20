/* legacy.c (generic) - nothing is available; see legacy.h */
#include "platform/legacy.h"

int legacy_get_cookie(long id, long *value)
{
    (void)id;
    (void)value;
    return -1;
}

int legacy_get_os(unsigned short *os_version, int *is_etos)
{
    (void)os_version;
    (void)is_etos;
    return -1;
}
