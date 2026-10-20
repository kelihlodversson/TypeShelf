/*
 * glyphs.h - the toolbar's monochrome bitmaps, as text
 *
 * Each glyph is `size` rows of `size` characters, '#' for a set pixel and '.'
 * for a clear one, in 8 and 16 pixel sizes.  glyph_pack() turns one into the
 * words vrt_cpyfm() wants (one 16 bit word per row, the pixels in its high
 * bits).
 */
#ifndef UI_GLYPHS_H
#define UI_GLYPHS_H

typedef enum {
    GLYPH_BACK,
    GLYPH_ICONS,
    GLYPH_SAMPLES,
    GLYPH_LIST,
    GLYPH_SPECIMEN,
    GLYPH_COUNT
} GlyphId;

/* the rows of a glyph at size 8 or 16, or NULL for any other size */
const char *const *glyph_rows(GlyphId id, int size);

/* Pack size rows into out[size].  0, or -1 if a row is not size characters
 * of '#' and '.'. */
int glyph_pack(const char *const *rows, int size, unsigned short *out);

#endif
