#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include "gdosinfo/about.h"
#include "gdosinfo/decode.h"

static void put(char lines[][ABOUT_WIDTH + 1], int *n, const char *fmt, ...) __attribute__((format(printf, 3, 4)));

static void put(char lines[][ABOUT_WIDTH + 1], int *n, const char *fmt, ...)
{
    va_list ap;

    if (*n >= ABOUT_MAX_LINES)
        return;
    va_start(ap, fmt);
    vsnprintf(lines[*n], ABOUT_WIDTH + 1, fmt, ap);
    va_end(ap);
    (*n)++;
}

int about_format(const AboutFacts *f, char lines[][ABOUT_WIDTH + 1])
{
    int n = 0;
    char tmp[48];

    put(lines, &n, "Type Shelf");
    put(lines, &n, "A font viewer for GEM");
    put(lines, &n, " ");
    put(lines, &n, "System: %s", f->os[0] != '\0' ? f->os : "unknown");
    put(lines, &n, "GDOS:   %s", gdos_vgdos_name(f->vgdos));
    if (f->have_fsmc) {
        gdos_format_fsmc(f->fsmc_magic, f->fsmc_version, f->fsmc_quality, tmp, sizeof tmp);
        put(lines, &n, "Vector: %s", tmp);
    }
    if (f->have_nvdi) {
        gdos_format_version("NVDI", f->nvdi_bcd, tmp, sizeof tmp);
        put(lines, &n, "        %s", tmp);
    }
    if (f->have_eddi)
        put(lines, &n, "        EdDI present");
    if (f->planes > 8)
        put(lines, &n, "Screen: %dx%d, %d-bit colour", f->screen_w, f->screen_h, f->planes);
    else
        put(lines, &n, "Screen: %dx%d, %d colours", f->screen_w, f->screen_h, 1 << f->planes);
    put(lines, &n, "Fonts:  %d %s, %d %s", f->families, f->families == 1 ? "family" : "families", f->styles,
        f->styles == 1 ? "style" : "styles");
    put(lines, &n, "        %d bitmap, %d scalable", f->styles - f->scalable_styles, f->scalable_styles);
    put(lines, &n, "Sizes:  %s", f->have_arbpt ? "any point size" : "the sizes the fonts have");
    put(lines, &n, "Text:   %s", f->have_ucs2 ? "Unicode" : "the Atari character set");
    return n;
}
