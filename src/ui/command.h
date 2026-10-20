/*
 * command.h - what the menu and the keyboard can ask for
 */
#ifndef UI_COMMAND_H
#define UI_COMMAND_H

typedef enum {
    CMD_NONE = 0,
    CMD_ABOUT,
    CMD_NEW_WINDOW,
    CMD_INSTALL,
    CMD_CLOSE,
    CMD_QUIT,
    CMD_VIEW_1,         /* Show as Icons, or as Specimen in a single font */
    CMD_VIEW_2,         /* Show as Samples */
    CMD_VIEW_3,         /* Show as List (not in a single font) */
    CMD_BACK,           /* leave a single font */
    CMD_OPEN            /* enter the selected family */
} Command;

/* AES key bits in kmeta, as evnt_multi() reports them */
#define KMETA_CTRL  0x0004

/*
 * The command a key stands for.  key is the AES keyboard word: the scan code
 * in the high byte, the character in the low byte.  Shortcuts go by scan
 * code, as a Control key changes the character.  CMD_NONE if it stands for
 * nothing.
 */
Command command_from_key(unsigned short key, unsigned short kmeta);

#endif
