/*
 * files.c - see files.h
 */
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

static int dir_exists(const char *dir)
{
    return Fsfirst(dir, FA_DIR) == 0;
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
        Fdelete(dst);
    free(buf);
    return result;
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
