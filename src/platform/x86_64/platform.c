/*
 * platform.c (x86_64) - cookies and the OS through Ssystem() for x32 userland
 */
#include <stdint.h>
#include "platform/legacy.h"
#include "platform/platform.h"

#define E_OK 0

/* Ssystem() mode values */
#define S_INQUIRE       ((WORD)0xffff)
#define S_OSNAME        0x0000
#define S_OSVERSION     0x0002
#define S_GETCOOKIE     0x0008

/* Typed syscall for x32 userland */
static __inline__ long ssystem_call(WORD mode, long arg1, long arg2)
{
    register unsigned long long rax __asm__("rax") = 0x100000154ULL;
    register long long rdi __asm__("rdi") = (long long)(unsigned short)mode;
    register long long rsi __asm__("rsi") = arg1;
    register long long rdx __asm__("rdx") = arg2;
    register long long r10 __asm__("r10") = 0;

    __asm__ volatile ("syscall" : "+r"(rax)
                      : "r"(rdi), "r"(rsi), "r"(rdx), "r"(r10)
                      : "rcx", "r11", "memory", "cc");
    return (long)rax;
}

/* S_INQUIRE: does this Ssystem() know the mode? */
static int ssystem_knows(int mode)
{
    return ssystem_call(S_INQUIRE, mode, 0) == E_OK;
}

int platform_get_cookie(long id, long *value)
{
    long v = 0;

    if (!ssystem_knows(S_GETCOOKIE))
        return legacy_get_cookie(id, value);
    if (ssystem_call(S_GETCOOKIE, id, (long)(uintptr_t)&v) != E_OK)
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

    os->name = ssystem_call(S_OSNAME, 0, 0);
    if (ssystem_knows(S_OSVERSION))
        os->version = (unsigned long)ssystem_call(S_OSVERSION, 0, 0);
    os->is_etos = os->name == (((long)'p' << 24) | ((long)'T' << 16) | ((long)'O' << 8) | 'S');
    return 0;
}
