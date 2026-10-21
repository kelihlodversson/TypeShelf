#include "ui/command.h"

#define SCAN_ESC        0x01
#define SCAN_1          0x02
#define SCAN_2          0x03
#define SCAN_3          0x04
#define SCAN_Q          0x10
#define SCAN_W          0x11
#define SCAN_O          0x18
#define SCAN_RETURN     0x1C
#define SCAN_N          0x31
#define SCAN_ENTER      0x72    /* the keypad's */
#define SCAN_UP         0x48
#define SCAN_LEFT       0x4B
#define SCAN_RIGHT      0x4D
#define SCAN_DOWN       0x50

Command command_from_key(unsigned short key, unsigned short kmeta)
{
    unsigned scan = (key >> 8) & 0xff;

    if (kmeta & KMETA_CTRL) {
        switch (scan) {
        case SCAN_N:    return CMD_NEW_WINDOW;
        case SCAN_O:    return CMD_INSTALL;
        case SCAN_W:    return CMD_CLOSE;
        case SCAN_Q:    return CMD_QUIT;
        case SCAN_1:    return CMD_VIEW_1;
        case SCAN_2:    return CMD_VIEW_2;
        case SCAN_3:    return CMD_VIEW_3;
        default:        return CMD_NONE;
        }
    }
    switch (scan) {
    case SCAN_ESC:      return CMD_BACK;
    case SCAN_RETURN:
    case SCAN_ENTER:    return CMD_OPEN;
    case SCAN_UP:       return CMD_UP;
    case SCAN_DOWN:     return CMD_DOWN;
    case SCAN_LEFT:     return CMD_LEFT;
    case SCAN_RIGHT:    return CMD_RIGHT;
    default:            return CMD_NONE;
    }
}
