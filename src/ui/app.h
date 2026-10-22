/*
 * app.h - the menu bar, the windows and the event loop
 */
#ifndef UI_APP_H
#define UI_APP_H

#include <gem.h>
#include "fontdb/group.h"
#include "vdi_if/gem_source.h"

/*
 * Run Type Shelf until Quit.  gs is the open font source and src what reads
 * its faces; fonts is the list built from it, which the app rebuilds in place
 * when fonts are installed.  Returns 0, or -1 if the menu or the first window
 * could not be created.
 */
int app_run(GemSource *gs, const VdiFontSource *src, FontList *fonts);

#endif
