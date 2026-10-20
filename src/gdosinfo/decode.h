/*
 * decode.h - turn the raw GDOS and OS facts into text for the About dialog
 *
 * Pure functions: the values come from vq_vgdos(), the FSMC and NVDI cookies
 * and the OS header, read elsewhere.
 */
#ifndef GDOSINFO_DECODE_H
#define GDOSINFO_DECODE_H

#include <stddef.h>

#define GDOS_NONE   (-2L)       /* what vq_vgdos() returns without a GDOS */

/* the GDOS vq_vgdos() reported; never NULL */
const char *gdos_vgdos_name(long value);

/* the vector-font GDOS an FSMC cookie's magic names; never NULL */
const char *gdos_fsmc_name(const char magic[4]);

/* "NVDI 4.10": bcd 0x0410 with a label; also for EdDI and others */
void gdos_format_version(const char *label, unsigned short bcd, char *out, size_t cap);

/* "SpeedoGDOS 4.00, quality -1" from an FSMC cookie's structure */
void gdos_format_fsmc(const char magic[4], short version, short quality,
                      char *out, size_t cap);

/* "TOS 2.06" or "EmuTOS or pTOS (TOS 2.06 compatible)" */
void gdos_format_os(unsigned short os_version, int is_etos, char *out, size_t cap);

#endif
