/*
 * files.h - the few GEMDOS file calls Install Font needs
 *
 * Paths are GEMDOS paths ("C:\GEMSYS\FONTS\A.TTF").  Not built on the host.
 */
#ifndef PLATFORM_FILES_H
#define PLATFORM_FILES_H

/* the size in bytes, or -1 if the file cannot be opened */
long platform_file_size(const char *path);

/* 1 if the file can be opened */
int platform_file_exists(const char *path);

/* Create the folder and any missing folders above it ("C:\A\B" makes C:\A,
 * then C:\A\B).  0 if it is there afterwards, else -1. */
int platform_make_path(const char *dir);

enum { COPY_OK = 0, COPY_OPEN = -1, COPY_CREATE = -2, COPY_READ = -3, COPY_WRITE = -4 };

/* Copy src over dst in 8 KB pieces.  A copy that fails half way is deleted. */
int platform_copy_file(const char *src, const char *dst);

/* Read up to cap bytes from the start of a file; the number read, or -1 */
long platform_read_file(const char *path, char *buf, long cap);

/* the drive the system booted from, 'A'..'Z'; 'C' if that cannot be told */
char platform_boot_drive(void);

#endif
