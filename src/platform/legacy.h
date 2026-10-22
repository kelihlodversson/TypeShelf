/*
 * legacy.h - cookies and the OS header without Ssystem()
 *
 * What platform.c falls back to on an m68k TOS or EmuTOS that has no
 * Ssystem().  Other targets answer "not available".
 */
#ifndef PLATFORM_LEGACY_H
#define PLATFORM_LEGACY_H

/* as platform_get_cookie() */
int legacy_get_cookie(long id, long *value);

/* the OS header's version word (0x0206 is TOS 2.06) and whether the 'ETOS'
 * marker EmuTOS and pTOS carry is there; 0, or -1 if they cannot be read */
int legacy_get_os(unsigned short *os_version, int *is_etos);

/* the drive the system booted from, 0 for A; -1 if it cannot be read */
int legacy_get_boot_drive(void);

#endif
