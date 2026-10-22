#include <stdlib.h>
#include <string.h>
#include "ui/menu.h"

/*
 * Object indexes.  The AES expects the root's children to be the bar and then
 * the screen (the area the drop-down boxes appear in), the bar's one child to
 * be the box the titles are in, and fills the first drop-down box (Desk) with
 * the desk accessories' entries by taking the objects that follow the box in the
 * array, so those must come right after it: the About entry, the separator
 * line, then a slot for each accessory.
 */
enum {
    M_ROOT, M_BAR, M_ACTIVE,
    M_T_DESK, M_T_FILE, M_T_VIEW,
    M_SCREEN,
    M_B_DESK, M_ABOUT, M_SEP, M_DA0,
    M_DA5 = M_DA0 + 5,
    M_B_FILE, M_NEW, M_INSTALL, M_CLOSE, M_QUIT,
    M_B_VIEW, M_VIEW1, M_VIEW2, M_VIEW3,
    M_COUNT
};

#define ITEM_W  24      /* characters, a leading space included */

typedef struct {
    OBJECT obj[M_COUNT];
    char text[M_COUNT][ITEM_W + 1];
} Menu;

static const struct {
    int index;
    const char *text;       /* "label|shortcut", padded to ITEM_W when built */
    Command cmd;
} items[] = {
    { M_ABOUT,   "About Type Shelf...|", CMD_ABOUT },
    { M_NEW,     "New Window|^N",        CMD_NEW_WINDOW },
    { M_INSTALL, "Install Font...|^O",   CMD_INSTALL },
    { M_CLOSE,   "Close|^W",             CMD_CLOSE },
    { M_QUIT,    "Quit|^Q",              CMD_QUIT },
    { M_VIEW1,   "Show as Icons|^1",     CMD_VIEW_1 },
    { M_VIEW2,   "Show as Samples|^2",   CMD_VIEW_2 },
    { M_VIEW3,   "Show as List|^3",      CMD_VIEW_3 },
};
#define ITEM_COUNT ((int)(sizeof items / sizeof items[0]))

/* " label        ^N", ITEM_W characters */
static void set_item(Menu *m, int index, const char *label, const char *key)
{
    char *t = m->text[index];
    size_t n = strlen(label);

    memset(t, ' ', ITEM_W);
    t[ITEM_W] = '\0';
    memcpy(t + 1, label, n);
    memcpy(t + ITEM_W - 1 - strlen(key), key, strlen(key));
}

static void set_child(OBJECT *o, int next, int head, int tail)
{
    o->ob_next = (short)next;
    o->ob_head = (short)head;
    o->ob_tail = (short)tail;
}

static void set_geom(OBJECT *o, int x, int y, int w, int h)
{
    o->ob_x = (short)x;
    o->ob_y = (short)y;
    o->ob_width = (short)w;
    o->ob_height = (short)h;
}

/* a drop-down box of count entries from first on, at column x */
static void link_box(OBJECT *o, int box, int next, int first, int count, int x)
{
    int i;

    o[box].ob_type = G_BOX;
    o[box].ob_spec.index = 0x00FF1100L;
    set_child(&o[box], next, first, first + count - 1);
    set_geom(&o[box], x, 0, ITEM_W, count);
    for (i = 0; i < count; i++) {
        o[first + i].ob_next = (short)(i == count - 1 ? box : first + i + 1);
        set_geom(&o[first + i], 0, i, ITEM_W, 1);
    }
}

OBJECT *menu_create(int cols, int rows)
{
    Menu *m = calloc(1, sizeof *m);
    OBJECT *o;
    int i;

    if (m == NULL)
        return NULL;
    o = m->obj;

    for (i = 0; i < M_COUNT; i++) {
        o[i].ob_type = G_STRING;
        o[i].ob_flags = 0;
        o[i].ob_state = OS_NORMAL;
        o[i].ob_spec.free_string = m->text[i];
        set_child(&o[i], NIL, NIL, NIL);
    }

    /* the root, the bar, and the area the drop-down boxes appear in */
    o[M_ROOT].ob_type = G_BOX;
    o[M_ROOT].ob_spec.index = 0L;
    set_child(&o[M_ROOT], NIL, M_BAR, M_SCREEN);
    set_geom(&o[M_ROOT], 0, 0, cols, rows);

    o[M_BAR].ob_type = G_BOX;
    o[M_BAR].ob_spec.index = 0x00001180L;
    set_child(&o[M_BAR], M_SCREEN, M_ACTIVE, M_ACTIVE);
    set_geom(&o[M_BAR], 0, 0, cols, 0x0201);        /* a character and two pixels */

    o[M_ACTIVE].ob_type = G_IBOX;
    o[M_ACTIVE].ob_spec.index = 0L;
    set_child(&o[M_ACTIVE], M_BAR, M_T_DESK, M_T_VIEW);
    set_geom(&o[M_ACTIVE], 2, 0, 18, 1);

    o[M_SCREEN].ob_type = G_IBOX;
    o[M_SCREEN].ob_spec.index = 0L;
    set_child(&o[M_SCREEN], M_ROOT, M_B_DESK, M_B_VIEW);
    set_geom(&o[M_SCREEN], 0, 0x0201, cols, rows - 1);

    /* the titles */
    strcpy(m->text[M_T_DESK], " Desk ");
    strcpy(m->text[M_T_FILE], " File ");
    strcpy(m->text[M_T_VIEW], " View ");
    for (i = M_T_DESK; i <= M_T_VIEW; i++) {
        o[i].ob_type = G_TITLE;
        o[i].ob_next = (short)(i == M_T_VIEW ? M_ACTIVE : i + 1);
        set_geom(&o[i], 6 * (i - M_T_DESK), 0, 6, 1);
    }

    /* the drop-down boxes and their entries; only the About entry is linked
     * in the Desk box, as the AES builds the rest of it */
    link_box(o, M_B_DESK, M_B_FILE, M_ABOUT, 1, 2);
    link_box(o, M_B_FILE, M_B_VIEW, M_NEW, 4, 8);
    link_box(o, M_B_VIEW, M_SCREEN, M_VIEW1, 3, 14);
    memset(m->text[M_SEP], '-', ITEM_W);
    o[M_SEP].ob_state |= OS_DISABLED;
    for (i = M_SEP; i <= M_DA5; i++) {
        if (i != M_SEP)
            memset(m->text[i], ' ', ITEM_W);
        set_geom(&o[i], 0, i - M_ABOUT, ITEM_W, 1);
    }

    for (i = 0; i < ITEM_COUNT; i++) {
        const char *bar = strchr(items[i].text, '|');
        char label[ITEM_W];

        memcpy(label, items[i].text, (size_t)(bar - items[i].text));
        label[bar - items[i].text] = '\0';
        set_item(m, items[i].index, label, bar + 1);
    }
    o[M_VIEW3].ob_flags |= OF_LASTOB;

    for (i = 0; i < M_COUNT; i++)
        rsrc_obfix(o, i);
    return o;
}

void menu_destroy(OBJECT *tree)
{
    free(tree);
}

Command menu_command(int item)
{
    int i;

    for (i = 0; i < ITEM_COUNT; i++)
        if (items[i].index == item)
            return items[i].cmd;
    return CMD_NONE;
}

void menu_update(OBJECT *tree, const WinState *ws)
{
    Menu *m = (Menu *)tree;
    int single = ws != NULL && ws->mode == MODE_SINGLE;

    set_item(m, M_VIEW1, single ? "Show as Specimen" : "Show as Icons", "^1");
    if (ws != NULL && !winstate_list_enabled(ws))
        tree[M_VIEW3].ob_state |= OS_DISABLED;
    else
        tree[M_VIEW3].ob_state &= (unsigned short)~OS_DISABLED;
}

void menu_set_install_enabled(OBJECT *tree, int enabled)
{
    if (enabled)
        tree[M_INSTALL].ob_state &= (unsigned short)~OS_DISABLED;
    else
        tree[M_INSTALL].ob_state |= OS_DISABLED;
}
