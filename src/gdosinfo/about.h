/*
 * about.h - the lines of the About dialog
 *
 * Pure: the facts are gathered elsewhere (ui/about.c) and turned into text here.
 */
#ifndef GDOSINFO_ABOUT_H
#define GDOSINFO_ABOUT_H

#define ABOUT_MAX_LINES 14
#define ABOUT_WIDTH     38      /* characters a line may have, without the end mark */

typedef struct {
    char os[48];                /* "pTOS 0.0.0a", "TOS 2.06", or "" */
    long vgdos;                 /* vq_vgdos() */
    int have_fsmc;
    char fsmc_magic[4];         /* the FSMC cookie's structure */
    short fsmc_version, fsmc_quality;
    int have_nvdi;
    unsigned short nvdi_bcd;
    int have_eddi;
    int screen_w, screen_h, planes;
    int families, styles, scalable_styles;
    int have_arbpt, have_ucs2;
} AboutFacts;

/* The lines to show, each at most ABOUT_WIDTH characters; returns how many
 * (at most ABOUT_MAX_LINES). */
int about_format(const AboutFacts *f, char lines[][ABOUT_WIDTH + 1]);

#endif
