/*
 * icons.h - the toolbar's icons, from the resource file
 *
 * TYPESHLF.RSC holds them as G_ICON objects, in two trees: 16x16 and 8x8.
 * tools/mkrsc.py writes the file from res/icons/; the order below is the order
 * of the objects in each tree.  The file has to sit where the AES finds it, next
 * to the program.
 */
#ifndef UI_ICONS_H
#define UI_ICONS_H

#include "ui/rect.h"

typedef enum {
    ICON_BACK,
    ICON_ICONS,
    ICON_SAMPLES,
    ICON_LIST,
    ICON_SPECIMEN,
    ICON_COUNT
} IconId;

#define ICONS_RSC_NAME  "TYPESHLF.RSC"

/* Load the resource file.  0, or -1 if it is not there or not readable. */
int icons_load(void);

void icons_free(void);

/* Draw an icon of size 16 or 8 with its top left at x, y, inside clip.  A
 * selected icon is drawn inverted. */
void icons_draw(IconId id, int size, int x, int y, int selected, const Rect *clip);

#endif
