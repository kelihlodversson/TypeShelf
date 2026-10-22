#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <mint/osbind.h>
#include <gem.h>
#include "platform/files.h"
#include "ui/install.h"

#define ALERT_WIDTH 30      /* what fits a line of an alert at 320 pixels, with room */

/* where the file selector was last: it starts at the current drive */
static char sel_path[128];

/* "D:\FONTS\*.*" to "D:\FONTS\" + name */
static int selected_file(const char *mask, const char *name, char *out, size_t cap)
{
    const char *slash = strrchr(mask, '\\');
    size_t dir_len = slash != NULL ? (size_t)(slash - mask) + 1 : 0;

    if (dir_len + strlen(name) + 1 > cap)
        return -1;
    memcpy(out, mask, dir_len);
    strcpy(out + dir_len, name);
    return 0;
}

#define ASSIGN_MAX  65536L      /* a longer ASSIGN.SYS is not read */

/* ASSIGN.SYS on the boot drive, in \GEMSYS or the root: 1 if there is one,
 * 0 if not, -1 if it cannot be read whole; *path is its path= (in out) or NULL */
static int read_assign(char drive, char *out, size_t cap, const char **path)
{
    static const char *const places[] = { ":\\GEMSYS\\ASSIGN.SYS", ":\\ASSIGN.SYS" };
    char file[32];
    char *text;
    size_t i;
    int found = 0;

    *path = NULL;
    for (i = 0; i < sizeof places / sizeof places[0]; i++) {
        long size, got;

        file[0] = drive;
        strcpy(file + 1, places[i]);
        size = platform_file_size(file);
        if (size < 0)
            continue;
        found = 1;
        if (size > ASSIGN_MAX)
            return -1;
        text = malloc((size_t)size + 1);
        if (text == NULL)
            return -1;
        got = platform_read_file(file, text, size);
        if (got != size) {              /* the whole file or nothing: a cut-off path= is a wrong folder */
            free(text);
            return -1;
        }
        if (got > 0 && install_parse_assign_path(text, (size_t)got, out, cap)) {
            *path = out;
            free(text);
            break;
        }
        free(text);
    }
    return found;
}

static void alert(int icon, const char *line1, const char *line2, const char *line3)
{
    char buf[320];

    snprintf(buf, sizeof buf, "[%d][%s|%s|%s][OK]", icon, line1, line2, line3);
    form_alert(1, buf);
}

static const char *copy_error(int code)
{
    switch (code) {
    case COPY_OPEN:   return "Cannot read the file.";
    case COPY_CREATE: return "Cannot create the file.";
    case COPY_READ:   return "Read error.";
    default:          return "Disk full or write error.";
    }
}

int install_ask_and_copy(InstallKind kind, InstallDone *done)
{
    char name[16];
    char src[128], dest[160], assign[128], fit[ALERT_WIDTH + 1], line[48];
    const char *ap;
    int assign_found;
    char boot = platform_boot_drive();
    short button = 0;
    long size;
    int rc;

    if (sel_path[0] == '\0')
        snprintf(sel_path, sizeof sel_path, "%c:\\*.*", (char)('A' + Dgetdrv()));
    name[0] = '\0';
    if (fsel_exinput(sel_path, name, &button, "Install font") == 0 || button == 0 ||
        name[0] == '\0')
        return 0;
    if (selected_file(sel_path, name, src, sizeof src) != 0)
        return 0;

    /* a disk SpeedoGDOS 4.00 has the cookie of pTOS's built-in GDOS; only it has an ASSIGN.SYS */
    assign_found = read_assign(boot, assign, sizeof assign, &ap);
    if (assign_found < 0) {
        alert(3, "Cannot read ASSIGN.SYS", "to find the fonts folder.", "");
        return 0;
    }
    if (assign_found && kind == INSTALL_BUILTIN)
        kind = INSTALL_DISK;

    if (!install_extension_ok(kind, name)) {
        alert(1, "Type Shelf installs", "TrueType and OpenType",
              kind == INSTALL_DISK ? "(.TTF .OTF .TTC .SPD) fonts." : "(.TTF .OTF .TTC) fonts.");
        return 0;
    }
    size = platform_file_size(src);
    if (size < 0) {
        alert(3, "Cannot read", name, "");
        return 0;
    }

    if (install_dest_dir(kind, boot, ap, done->dir, sizeof done->dir) != 0 ||
        install_dest_file(done->dir, src, dest, sizeof dest) != 0) {
        alert(3, "Cannot work out where", "to install", name);
        return 0;
    }
    strncpy(done->name, install_basename(dest), sizeof done->name - 1);
    done->name[sizeof done->name - 1] = '\0';
    if (install_same_path(src, dest)) {
        alert(1, name, "is in the fonts folder", "already.");
        return 0;
    }

    install_fit(done->dir, ALERT_WIDTH, fit);
    snprintf(line, sizeof line, "(%ld KB) into", (size + 1023) / 1024);
    {
        char buf[320];

        snprintf(buf, sizeof buf, "[2][Install %s|%s|%s?][Install|Cancel]", done->name, line, fit);
        if (form_alert(1, buf) != 1)
            return 0;
    }

    done->replaced = platform_file_exists(dest);
    if (done->replaced) {
        char buf[320];

        snprintf(buf, sizeof buf, "[2][%s is in|%s|already. Replace it?][Replace|Cancel]",
                 done->name, fit);
        if (form_alert(2, buf) != 1)
            return 0;
    }

    graf_mouse(BUSY_BEE, NULL);
    rc = platform_make_path(done->dir) == 0 ? platform_install_file(src, dest) : COPY_CREATE;
    graf_mouse(ARROW, NULL);
    if (rc != COPY_OK) {
        alert(3, "Install failed:", copy_error(rc), done->dir);
        return 0;
    }
    return 1;
}

void install_report(const InstallDone *done, int listed)
{
    char fit[ALERT_WIDTH + 1];

    install_fit(done->dir, ALERT_WIDTH, fit);
    if (listed) {
        alert(1, done->name, "is installed in", fit);
    } else if (done->replaced) {
        alert(1, done->name, "was replaced in", fit);
        alert(1, "If it looks unchanged,", "restart: the fonts may", "be read only at start.");
    } else {
        alert(1, done->name, "was copied to", fit);
        alert(1, "If it is not listed,", "restart: the fonts may", "be listed only at start.");
    }
}
