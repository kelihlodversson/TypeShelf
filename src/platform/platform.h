/*
 * platform.h - the few things that differ from one machine to the next
 *
 * Everything comes from Ssystem() (FreeMiNT, pTOS), which every target tries
 * first.  Only on m68k is there a fallback, for a TOS or EmuTOS without it:
 * the cookie jar and the OS header, read with Setexc() and Super() (legacy.h).
 */
#ifndef PLATFORM_PLATFORM_H
#define PLATFORM_PLATFORM_H

/* 0 and *value set if the cookie is in the jar, -1 if not (or no jar) */
int platform_get_cookie(long id, long *value);

typedef struct {
    long name;                  /* Ssystem()'s four characters ('pTOS'); 0 if it has none */
    unsigned long version;      /* its version: major<<24 | minor<<16 | patch<<8 | release character; 0 if unknown */
    unsigned short tos_version; /* the OS header's version word (0x0206 is TOS 2.06); 0 if unknown */
    int is_etos;                /* the 'ETOS' marker EmuTOS and pTOS carry */
} PlatformOs;

/* Returns 0 and fills in *os, or -1 if nothing about the OS can be read */
int platform_get_os(PlatformOs *os);

#endif
