#include <stddef.h>
#include <gem.h>
#include "ui/icons.h"

#define TREE_16     0
#define TREE_8      1

static OBJECT *trees[2];

int icons_load(void)
{
    int i;

    if (rsrc_load(ICONS_RSC_NAME) == 0)
        return -1;
    for (i = 0; i < 2; i++) {
        if (rsrc_gaddr(R_TREE, (short)i, &trees[i]) == 0 || trees[i] == NULL) {
            rsrc_free();
            trees[0] = trees[1] = NULL;
            return -1;
        }
    }
    return 0;
}

void icons_free(void)
{
    if (trees[0] != NULL)
        rsrc_free();
    trees[0] = trees[1] = NULL;
}

void icons_draw(IconId id, int size, int x, int y, int selected, const Rect *clip)
{
    OBJECT *tree = trees[size == 16 ? TREE_16 : TREE_8];
    int index = 1 + (int)id;

    if (tree == NULL || (unsigned)id >= (unsigned)ICON_COUNT)
        return;
    /* the icons sit at 0, 0 inside the root, so the root says where they go */
    tree[0].ob_x = (short)x;
    tree[0].ob_y = (short)y;
    if (selected)
        tree[index].ob_state |= OS_SELECTED;
    objc_draw(tree, (short)index, 0, (short)clip->x, (short)clip->y, (short)clip->w, (short)clip->h);
    tree[index].ob_state &= (unsigned short)~OS_SELECTED;
}
