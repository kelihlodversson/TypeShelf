/*
 * platform.h - the few things that differ from one machine to the next
 *
 * One implementation per directory under src/platform/; the Makefile picks
 * it.  The generic one answers "not available" to everything, which is what a
 * target gets until its own is written.
 */
#ifndef PLATFORM_PLATFORM_H
#define PLATFORM_PLATFORM_H

/* 0 and *value set if the cookie is in the jar, -1 if not (or no jar) */
int platform_get_cookie(long id, long *value);

/*
 * The OS header's version word (0x0206 is TOS 2.06) and whether the 'ETOS'
 * marker EmuTOS and pTOS carry is there.  Returns 0 and sets both, or -1 if
 * they cannot be read.
 */
int platform_get_os(unsigned short *os_version, int *is_etos);

#endif
