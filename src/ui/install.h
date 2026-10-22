/*
 * install.h - Install Font: pick a font file, copy it into the fonts folder
 *
 * The pure decisions (the folder, the extension) are in install/install.h.
 * Reloading the font list afterwards is the app's job (ui/app.c).
 */
#ifndef UI_INSTALL_H
#define UI_INSTALL_H

#include "install/install.h"

typedef struct {
    char name[16];          /* the installed file's name */
    char dir[128];          /* the folder it went to */
    int replaced;           /* there was a file of that name already */
} InstallDone;

/* Ask for a font file and copy it.  1 if a font was copied (done is filled in),
 * 0 if the user cancelled or an alert has said why not. */
int install_ask_and_copy(InstallKind kind, InstallDone *done);

/* The closing alert.  listed is 1 if the font list has more faces now. */
void install_report(const InstallDone *done, int listed);

#endif
