/*
 * files.c - see files.h
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <mint/osbind.h>
#include "platform/files.h"
#include "platform/legacy.h"

#define COPY_CHUNK  8192L
#define FA_DIR      0x10

long platform_file_size(const char *path)
{
    long fd = Fopen(path, 0);
    long size;

    if (fd < 0)
        return -1;
    size = Fseek(0L, (short)fd, 2);         /* to the end: the position is the size */
    Fclose((short)fd);
    return size < 0 ? -1 : size;
}

int platform_file_exists(const char *path)
{
    long fd = Fopen(path, 0);

    if (fd < 0)
        return 0;
    Fclose((short)fd);
    return 1;
}

/* Fsfirst() leaves out hidden and system folders: ask for the attributes */
static int dir_exists(const char *dir)
{
    long attr = Fattrib(dir, 0, 0);

    return attr >= 0 && (attr & FA_DIR) != 0;
}

int platform_make_path(const char *dir)
{
    char buf[128];
    size_t n = strlen(dir), i;

    if (n >= sizeof buf)
        return -1;
    memcpy(buf, dir, n + 1);
    for (i = 3; i <= n; i++) {              /* start after "C:\" */
        if (buf[i] == '\\' || buf[i] == '\0') {
            char c = buf[i];

            buf[i] = '\0';
            if (i > 3 && !dir_exists(buf) && Dcreate(buf) != 0 && !dir_exists(buf))
                return -1;
            buf[i] = c;
        }
    }
    return dir_exists(buf) || n <= 3 ? 0 : -1;
}

int platform_copy_file(const char *src, const char *dst)
{
    long in, out, got;
    char *buf;
    int result = COPY_OK;

    buf = malloc(COPY_CHUNK);
    if (buf == NULL)
        return COPY_READ;
    in = Fopen(src, 0);
    if (in < 0) {
        free(buf);
        return COPY_OPEN;
    }
    out = Fcreate(dst, 0);
    if (out < 0) {
        Fclose((short)in);
        free(buf);
        return COPY_CREATE;
    }
    for (;;) {
        got = Fread((short)in, COPY_CHUNK, buf);
        if (got < 0) {
            result = COPY_READ;
            break;
        }
        if (got == 0)
            break;
        if (Fwrite((short)out, got, buf) != got) {
            result = COPY_WRITE;
            break;
        }
    }
    Fclose((short)in);
    if (Fclose((short)out) < 0 && result == COPY_OK)
        result = COPY_WRITE;
    if (result != COPY_OK)
        (void)Fdelete(dst);
    free(buf);
    return result;
}

/*
 * A scratch name in dst's folder that nothing has: "TSnnn.ext".  Names are
 * never reused, so another install's temporary file or an interrupted
 * install's backup (which may be the only copy of a font) is left alone.
 * Nothing else runs between this check and the creation, as the AES
 * multitasks cooperatively and GEMDOS calls do not yield.
 */
static int scratch_name(const char *dst, const char *ext, char *out, size_t cap)
{
    const char *slash = strrchr(dst, '\\');
    size_t dir = slash != NULL ? (size_t)(slash - dst) + 1 : 0;
    int n;

    if (dir + 10 > cap)
        return -1;
    memcpy(out, dst, dir);
    for (n = 0; n < 1000; n++) {
        sprintf(out + dir, "TS%03d.%s", n, ext);
        if (!platform_file_exists(out) && Fsfirst(out, 0x16) != 0)
            return 0;
    }
    return -1;
}

int platform_install_file(const char *src, const char *dst)
{
    char tmp[160], bak[160];
    int rc, had_old;

    if (scratch_name(dst, "TMP", tmp, sizeof tmp) != 0)
        return COPY_CREATE;
    rc = platform_copy_file(src, tmp);
    if (rc != COPY_OK)
        return rc;

    had_old = platform_file_exists(dst);
    if (had_old) {
        if (scratch_name(dst, "BAK", bak, sizeof bak) != 0 || Frename(0, dst, bak) != 0) {
            (void)Fdelete(tmp);
            return COPY_CREATE;
        }
    }
    if (Frename(0, tmp, dst) != 0) {
        if (had_old)
            (void)Frename(0, bak, dst);     /* back as it was */
        (void)Fdelete(tmp);
        return COPY_CREATE;
    }
    if (had_old)
        (void)Fdelete(bak);
    return COPY_OK;
}

long platform_read_file(const char *path, char *buf, long cap)
{
    long fd = Fopen(path, 0);
    long got;

    if (fd < 0)
        return -1;
    got = Fread((short)fd, cap, buf);
    Fclose((short)fd);
    return got;
}

char platform_boot_drive(void)
{
    int drive = legacy_get_boot_drive();

    return drive >= 0 && drive < 26 ? (char)('A' + drive) : 'C';
}
