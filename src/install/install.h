/*
 * install.h - where Install Font puts a font, and whether it can
 *
 * Pure: the facts come from About's AboutFacts, the file names and the text
 * of ASSIGN.SYS from the caller.  The file and GEM calls are in ui/install.c.
 */
#ifndef INSTALL_INSTALL_H
#define INSTALL_INSTALL_H

#include <stddef.h>
#include "gdosinfo/about.h"

typedef enum {
    INSTALL_NONE,       /* no GDOS that loads scalable fonts: Install is disabled */
    INSTALL_BUILTIN,    /* pTOS's built-in GDOS: TTF, OTF and TTC in \GEMSYS\FONTS */
    INSTALL_DISK        /* NVDI, SpeedoGDOS ...: the folder is ASSIGN.SYS's path= */
} InstallKind;

#define INSTALL_BUILTIN_DIR "\\GEMSYS\\FONTS"
#define INSTALL_DISK_DIR    "\\GEMSYS"          /* when ASSIGN.SYS has no path= */

/* A cookie of SpeedoGDOS 4.00 ("_SPD", 0x0400) is also what pTOS's built-in
 * GDOS has, and nothing tells the two apart but ASSIGN.SYS (a GDOS from disk
 * reads one; pTOS's has none): INSTALL_BUILTIN is only the first guess, and
 * the caller makes it INSTALL_DISK when it finds an ASSIGN.SYS.  FONTGDOS
 * ("_FNT") has no scalable fonts. */
InstallKind install_kind(const AboutFacts *f);

/* 1 if name (a file name or a path) ends in an extension the GDOS loads by
 * itself: .TTF, .OTF, .TTC, and for a GDOS from disk .SPD; any case */
int install_extension_ok(InstallKind kind, const char *name);

/* the file name at the end of a path, after the last '\' , '/' or ':' */
const char *install_basename(const char *path);

/* The value of the first "path=" line of ASSIGN.SYS's text (len bytes, not
 * NUL-terminated), spaces and the "drive:" kept, without a trailing '\' (but a root, "\" or "C:\", stays).
 * 1 and out set if there is one, else 0. */
int install_parse_assign_path(const char *text, size_t len, char *out, size_t cap);

/* The folder to install into, "X:\GEMSYS\FONTS" with no trailing '\'.
 * assign_path is ASSIGN.SYS's path= or NULL.  Drives without a letter take
 * boot_drive ('A'..'Z').  0 and out set, or -1 if INSTALL_NONE or too long. */
int install_dest_dir(InstallKind kind, char boot_drive, const char *assign_path,
                     char *out, size_t cap);

/* dir + '\' + the file name of path, upper-cased.  0, or -1 if too long. */
int install_dest_file(const char *dir, const char *path, char *out, size_t cap);

/* 1 if two paths name the same file (case-insensitively, as TOS does) */
int install_same_path(const char *a, const char *b);

/* s shortened to at most width characters, keeping the end: "...\FONTS" */
void install_fit(const char *s, size_t width, char *out);

#endif
