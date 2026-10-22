#include <ctype.h>
#include <string.h>
#include "install/install.h"

static int upper(int c)
{
    return toupper((unsigned char)c);
}

InstallKind install_kind(const AboutFacts *f)
{
    int outline = f->have_fsmc && (memcmp(f->fsmc_magic, "_SPD", 4) == 0 ||
                                   memcmp(f->fsmc_magic, "_FSM", 4) == 0);

    /* pTOS's built-in GDOS looks like SpeedoGDOS 4.00; the caller tells them apart by ASSIGN.SYS */
    if (outline && memcmp(f->fsmc_magic, "_SPD", 4) == 0 && f->fsmc_version == 0x0400 && !f->have_nvdi)
        return INSTALL_BUILTIN;
    /* FONTGDOS ('_FNT') has bitmap fonts only: it counts only with an NVDI that has outlines */
    if (outline || (f->have_nvdi && f->nvdi_bcd >= 0x0300))
        return INSTALL_DISK;
    return INSTALL_NONE;
}

const char *install_basename(const char *path)
{
    const char *p, *base = path;

    for (p = path; *p != '\0'; p++)
        if (*p == '\\' || *p == '/' || *p == ':')
            base = p + 1;
    return base;
}

int install_extension_ok(InstallKind kind, const char *name)
{
    const char *base = install_basename(name);
    const char *dot = strrchr(base, '.');
    char ext[4];
    size_t i;

    if (kind == INSTALL_NONE || dot == NULL || strlen(dot + 1) != 3)
        return 0;
    for (i = 0; i < 3; i++)
        ext[i] = (char)upper(dot[1 + i]);
    ext[3] = '\0';
    if (strcmp(ext, "TTF") == 0 || strcmp(ext, "OTF") == 0 || strcmp(ext, "TTC") == 0)
        return 1;
    return kind == INSTALL_DISK && strcmp(ext, "SPD") == 0;
}

int install_parse_assign_path(const char *text, size_t len, char *out, size_t cap)
{
    size_t i = 0;

    while (i < len) {
        size_t start, end;

        while (i < len && (text[i] == ' ' || text[i] == '\t'))
            i++;
        start = i;
        while (i < len && text[i] != '\n' && text[i] != '\r')
            i++;
        end = i;
        while (i < len && (text[i] == '\n' || text[i] == '\r'))
            i++;

        if (end - start >= 4 && upper(text[start]) == 'P' && upper(text[start + 1]) == 'A' &&
            upper(text[start + 2]) == 'T' && upper(text[start + 3]) == 'H') {
            size_t p = start + 4;
            size_t n;

            while (p < end && (text[p] == ' ' || text[p] == '\t'))
                p++;
            if (p >= end || text[p] != '=')
                continue;               /* "pathology" and the like */
            p++;
            while (p < end && (text[p] == ' ' || text[p] == '\t'))
                p++;
            while (end > p && (text[end - 1] == ' ' || text[end - 1] == '\t'))
                end--;
            while (end > p && (text[end - 1] == '\\' || text[end - 1] == '/')) {
                /* keep the root of a drive: "C:\" stays a path of its own */
                if (end - p == 3 && text[p + 1] == ':')
                    break;
                if (end - p == 1)       /* and the root of the boot drive */
                    break;
                end--;
            }
            n = end - p;
            if (n == 0 || n >= cap)
                return 0;
            memcpy(out, text + p, n);
            out[n] = '\0';
            return 1;
        }
    }
    return 0;
}

int install_dest_dir(InstallKind kind, char boot_drive, const char *assign_path,
                     char *out, size_t cap)
{
    const char *dir;
    char tmp[128];
    size_t n = 0, i;

    if (kind == INSTALL_NONE)
        return -1;
    if (kind == INSTALL_BUILTIN || assign_path == NULL || assign_path[0] == '\0')
        dir = kind == INSTALL_BUILTIN ? INSTALL_BUILTIN_DIR : INSTALL_DISK_DIR;
    else
        dir = assign_path;

    if (!(dir[0] != '\0' && dir[1] == ':')) {
        tmp[n++] = (char)upper(boot_drive);
        tmp[n++] = ':';
        if (dir[0] != '\\')
            tmp[n++] = '\\';
    }
    if (n + strlen(dir) + 1 > sizeof tmp)
        return -1;
    for (i = 0; dir[i] != '\0'; i++)
        tmp[n++] = dir[i] == '/' ? '\\' : (char)upper(dir[i]);
    tmp[n] = '\0';
    if (n > 3 && tmp[n - 1] == '\\')
        tmp[--n] = '\0';
    if (n + 1 > cap)
        return -1;
    memcpy(out, tmp, n + 1);
    return 0;
}

int install_dest_file(const char *dir, const char *path, char *out, size_t cap)
{
    const char *base = install_basename(path);
    size_t d = strlen(dir), b = strlen(base), i;

    if (b == 0 || d + 1 + b + 1 > cap)
        return -1;
    memcpy(out, dir, d);
    if (d > 0 && dir[d - 1] != '\\')
        out[d++] = '\\';
    if (d + b + 1 > cap)
        return -1;
    for (i = 0; i < b; i++)
        out[d + i] = (char)upper(base[i]);
    out[d + b] = '\0';
    return 0;
}

int install_same_path(const char *a, const char *b)
{
    for (; *a != '\0' && *b != '\0'; a++, b++) {
        int x = *a == '/' ? '\\' : upper(*a);
        int y = *b == '/' ? '\\' : upper(*b);

        if (x != y)
            return 0;
    }
    return *a == *b;
}

void install_fit(const char *s, size_t width, char *out)
{
    size_t n = strlen(s);

    if (n <= width) {
        memcpy(out, s, n + 1);
    } else if (width <= 3) {
        memcpy(out, s + n - width, width + 1);
    } else {
        memcpy(out, "...", 3);
        memcpy(out + 3, s + n - (width - 3), width - 3 + 1);
    }
}
