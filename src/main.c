/*
 * main.c - Type Shelf
 *
 * Read the fonts, then run the windows.
 */
#include <stdio.h>
#include <string.h>
#include <gem.h>
#include "fontdb/build.h"
#include "ui/app.h"
#include "vdi_if/gem_source.h"

int main(void)
{
    GemSource gs;
    VdiFontSource source;
    FontList list;
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
        app_run(gs.handle, &list, gs.have_arbpt, gs.have_ucs2);
        fontlist_free(&list);
    }

    gem_source_close(&gs);
    appl_exit();
    return 0;
}
