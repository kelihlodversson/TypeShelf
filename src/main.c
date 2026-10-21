/*
 * main.c - Type Shelf
 *
 * Read the fonts, then run the windows.
 */
#include <stdio.h>
#include <string.h>
#include <gem.h>
#include "fontdb/build.h"
#include "gdosinfo/decode.h"
#include "platform/platform.h"
#include "ui/app.h"
#include "vdi_if/gem_source.h"

int main(void)
{
    GemSource gs;
    VdiFontSource source;
    FontList list;
    char os_text[64];
    char alert[256];
    PlatformOs os;
    int faces;

    if (appl_init() < 0)
        return 1;

    if (gem_source_open(&gs, &source) != 0) {
        form_alert(1, "[3][Type Shelf|Cannot open a workstation.][OK]");
        appl_exit();
        return 1;
    }

    faces = fontdb_build(&source, &list);
    if (faces < 0) {
        form_alert(1, "[3][Type Shelf|Out of memory.][OK]");
    } else {
        if (platform_get_os(&os) != 0)
            strcpy(os_text, "unknown OS");
        else if (os.name != 0)
            gdos_format_osname(os.name, os.version, os_text, sizeof os_text);
        else if (os.tos_version != 0)
            gdos_format_os(os.tos_version, os.is_etos, os_text, sizeof os_text);
        else
            strcpy(os_text, "unknown OS");
        snprintf(alert, sizeof alert, "[1][Type Shelf|%d families, %d faces|%s|%s][OK]",
                 list.family_count, faces, gdos_vgdos_name(vq_vgdos()), os_text);
        app_run(gs.handle, &list, alert, gs.have_ext);
        fontlist_free(&list);
    }

    gem_source_close(&gs);
    appl_exit();
    return 0;
}
