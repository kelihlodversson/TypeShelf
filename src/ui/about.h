/*
 * about.h - the About dialog
 */
#ifndef UI_ABOUT_H
#define UI_ABOUT_H

#include "fontdb/group.h"
#include "gdosinfo/about.h"

/* Gather the facts: the OS, the GDOS cookies, the screen of workstation vh,
 * and the font counts. */
void about_collect(AboutFacts *f, short vh, const FontList *fonts, int have_arbpt, int have_ucs2);

/* Show the dialog and wait for OK. */
void about_show(const AboutFacts *f);

#endif
