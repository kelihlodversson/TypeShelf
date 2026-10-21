/*
 * app.h - the menu bar, the windows and the event loop
 */
#ifndef UI_APP_H
#define UI_APP_H

#include <gem.h>
#include "fontdb/group.h"

/*
 * Run Type Shelf until Quit.  vh is the screen workstation, fonts the font
 * list, about the text of the About alert (a form_alert() string).  Returns
 * 0, or -1 if the menu or the first window could not be created.
 */
int app_run(short vh, const FontList *fonts, const char *about, int have_arbpt, int have_ucs2);

#endif
