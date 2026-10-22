/*
 * menu.h - the menu bar, built as an AES object tree in code
 *
 * The tree is built here and not loaded from a resource file: a resource file
 * is big-endian m68k data, and building it lets the layout follow the screen.
 */
#ifndef UI_MENU_H
#define UI_MENU_H

#include <gem.h>
#include "ui/command.h"
#include "ui/winstate.h"

/* Build the tree for a screen cols characters wide and rows high, and fix its
 * character coordinates to pixels.  NULL if there is no memory. */
OBJECT *menu_create(int cols, int rows);

void menu_destroy(OBJECT *tree);

/* The command a menu_selected message's item stands for; CMD_NONE for any
 * other. */
Command menu_command(int item);

/* Bring the entries in line with the front window's state (ws may be NULL
 * when there is none): the first view entry reads "Show as Specimen" in a
 * single font, and List is greyed out there. */
void menu_update(OBJECT *tree, const WinState *ws);

/* Install Font... is greyed out when the GDOS has no scalable fonts */
void menu_set_install_enabled(OBJECT *tree, int enabled);

#endif
