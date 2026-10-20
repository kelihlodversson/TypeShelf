#include <gem.h>

int main(void)
{
    if (appl_init() < 0)
        return 1;
    form_alert(1, "[1][Type Shelf][OK]");
    appl_exit();
    return 0;
}
