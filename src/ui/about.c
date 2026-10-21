#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <gem.h>
#include "gdosinfo/decode.h"
#include "platform/platform.h"
#include "ui/about.h"

#define COOKIE_FSMC 0x46534D43L     /* 'FSMC' */
#define COOKIE_NVDI 0x4E564449L     /* 'NVDI' */
#define COOKIE_EDDI 0x45644449L     /* 'EdDI' */

void about_collect(AboutFacts *f, short vh, const FontList *fonts, int have_arbpt, int have_ucs2)
{
    PlatformOs os;
    long value;
    short ext[57];
    int i, j;

    memset(f, 0, sizeof *f);
    if (platform_get_os(&os) == 0) {
        if (os.name != 0)
            gdos_format_osname(os.name, os.version, f->os, sizeof f->os);
        else if (os.tos_version != 0)
            gdos_format_os(os.tos_version, os.is_etos, f->os, sizeof f->os);
    }
    f->vgdos = vq_vgdos();
#if defined(__m68k__) || defined(__arm__)
    /* the cookie points at {magic, version, quality}; a pointer is 32 bits on these */
    if (platform_get_cookie(COOKIE_FSMC, &value) == 0 && value != 0 && (value & 1) == 0) {
        const struct { unsigned long magic; unsigned short version, quality; } *p =
            (const void *)(uintptr_t)(unsigned long)value;

        f->fsmc_magic[0] = (char)(p->magic >> 24);      /* '_SPD', whatever the byte order */
        f->fsmc_magic[1] = (char)(p->magic >> 16);
        f->fsmc_magic[2] = (char)(p->magic >> 8);
        f->fsmc_magic[3] = (char)p->magic;
        f->fsmc_version = (short)p->version;
        f->fsmc_quality = (short)p->quality;
        f->have_fsmc = 1;
    }
#endif
    if (platform_get_cookie(COOKIE_NVDI, &value) == 0) {
        f->have_nvdi = 1;
#if defined(__m68k__) || defined(__arm__)
        /* the cookie points at a structure whose first word is the BCD version */
        if (value != 0 && (value & 1) == 0)
            f->nvdi_bcd = *(const unsigned short *)(uintptr_t)(unsigned long)value;
#endif
    }
    f->have_eddi = platform_get_cookie(COOKIE_EDDI, &value) == 0;
    memset(ext, 0, sizeof ext);
    vq_extnd(vh, 0, ext);               /* the size, as the workstation was opened */
    f->screen_w = ext[0] + 1;
    f->screen_h = ext[1] + 1;
    memset(ext, 0, sizeof ext);
    vq_extnd(vh, 1, ext);               /* the extended facts: the planes */
    f->planes = ext[4];
    f->families = fonts->family_count;
    for (i = 0; i < fonts->family_count; i++) {
        for (j = 0; j < fonts->families[i].style_count; j++) {
            f->styles++;
            if (fonts->families[i].styles[j].format != FONT_FORMAT_BITMAP)
                f->scalable_styles++;
        }
    }
    f->have_arbpt = have_arbpt;
    f->have_ucs2 = have_ucs2;
}

typedef struct {
    OBJECT obj[ABOUT_MAX_LINES + 2];
    char text[ABOUT_MAX_LINES][ABOUT_WIDTH + 1];
} Dialog;

static void place(OBJECT *o, int x, int y, int w, int h)
{
    o->ob_x = (short)x;
    o->ob_y = (short)y;
    o->ob_width = (short)w;
    o->ob_height = (short)h;
}

void about_show(const AboutFacts *f)
{
    Dialog *d = calloc(1, sizeof *d);
    OBJECT *o;
    int n, i, btn;
    short x, y, w, h;

    if (d == NULL)
        return;
    o = d->obj;
    n = about_format(f, d->text);
    btn = n + 1;

    o[0].ob_type = G_BOX;
    o[0].ob_spec.index = 0x00021100L;           /* a thick outline, white fill */
    o[0].ob_state = OS_OUTLINED;
    o[0].ob_head = 1;
    o[0].ob_tail = (short)btn;
    o[0].ob_next = -1;
    place(&o[0], 0, 0, ABOUT_WIDTH + 2 * ABOUT_PAD, n + 4);
    for (i = 0; i < n; i++) {
        OBJECT *s = &o[1 + i];

        s->ob_type = G_STRING;
        s->ob_spec.free_string = d->text[i];
        s->ob_next = (short)(2 + i);
        s->ob_head = s->ob_tail = -1;
        place(s, ABOUT_PAD, 1 + i, ABOUT_WIDTH, 1);
    }
    o[btn].ob_type = G_BUTTON;
    o[btn].ob_flags = OF_SELECTABLE | OF_DEFAULT | OF_EXIT | OF_LASTOB;
    o[btn].ob_spec.free_string = "OK";
    o[btn].ob_next = 0;
    o[btn].ob_head = o[btn].ob_tail = -1;
    place(&o[btn], (ABOUT_WIDTH + 2 * ABOUT_PAD - 8) / 2, n + 2, 8, 1);
    for (i = 0; i <= btn; i++)
        rsrc_obfix(o, i);

    form_center(o, &x, &y, &w, &h);
    wind_update(BEG_UPDATE);            /* nobody else draws or reads the mouse meanwhile */
    wind_update(BEG_MCTRL);
    form_dial(FMD_START, 0, 0, 0, 0, x, y, w, h);
    objc_draw(o, 0, 8, x, y, w, h);
    form_do(o, 0);
    o[btn].ob_state &= (unsigned short)~OS_SELECTED;
    form_dial(FMD_FINISH, 0, 0, 0, 0, x, y, w, h);
    wind_update(END_MCTRL);
    wind_update(END_UPDATE);
    free(d);
}
