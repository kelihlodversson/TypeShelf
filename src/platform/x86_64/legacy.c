/*
 * legacy.c (x86_64) - Ssystem() bindings for pTOS x32 userland
 *
 * x86-64 ILP32 userspace ABI: 32-bit int/long/pointer, 64-bit long long.
 * The kernel runs in 64-bit mode (LP64), but userspace stays ILP32.
 *
 * This provides the Ssystem() interface for x32 programs running under pTOS.
 */
#include <stdint.h>
#include "platform/legacy.h"

#define E_OK 0

/* Ssystem() mode values */
#define S_INQUIRE       ((WORD)0xffff)
#define S_GETCOOKIE     0x0008

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

int legacy_get_boot_drive(void)
{
    return -1;
}
