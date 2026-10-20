#include <string.h>
#include <ctype.h>
#include "fontdb/split.h"

#define MAX_TOKENS 32

static const FontStyleWord style_words[] = {
    { "Thin",       0, 0 },
    { "ExtraLight", 1, 0 },
    { "Light",      2, 0 },
    { "Regular",    3, 0 },
    { "Roman",      3, 0 },
    { "Book",       3, 0 },
    { "Normal",     3, 0 },
    { "Medium",     4, 0 },
    { "SemiBold",   5, 0 },
    { "DemiBold",   5, 0 },
    { "Bold",       6, 0 },
    { "ExtraBold",  7, 0 },
    { "Heavy",      8, 0 },
    { "Black",      8, 0 },
    { "Italic",    -1, 1 },
    { "Oblique",   -1, 1 },
    { NULL,         0, 0 }
};

const FontStyleWord *font_style_word(const char *s, size_t len)
{
    const FontStyleWord *w;
    size_t i;

    for (w = style_words; w->word != NULL; w++) {
        if (strlen(w->word) != len)
            continue;
        for (i = 0; i < len; i++) {
            if (tolower((unsigned char)s[i]) != tolower((unsigned char)w->word[i]))
                break;
        }
        if (i == len)
            return w;
    }
    return NULL;
}

/* join tokens [from, to) with single spaces into dst, never past its end */
static void join_tokens(char *dst, const char *const *start, const size_t *len,
                        int from, int to)
{
    size_t used = 0;
    int i;

    dst[0] = '\0';
    for (i = from; i < to; i++) {
        size_t need = len[i] + (used > 0 ? 1 : 0);

        if (used + need >= SPLIT_NAME_MAX) {
            /* a first word that alone does not fit is cut to fit; a later one
             * that does not fit is left out whole */
            if (used == 0) {
                used = SPLIT_NAME_MAX - 1;
                memcpy(dst, start[i], used);
                dst[used] = '\0';
            }
            break;
        }
        if (used > 0)
            dst[used++] = ' ';
        memcpy(dst + used, start[i], len[i]);
        used += len[i];
        dst[used] = '\0';
    }
}

void font_split_name(const char *full, FontNameParts *out)
{
    const char *start[MAX_TOKENS];
    size_t len[MAX_TOKENS];
    int n = 0;
    int first_style;
    const char *p = full;

    while (*p != '\0' && n < MAX_TOKENS) {
        while (*p == ' ')
            p++;
        if (*p == '\0')
            break;
        start[n] = p;
        while (*p != '\0' && *p != ' ')
            p++;
        len[n] = (size_t)(p - start[n]);
        n++;
    }

    /* the family keeps at least one token: the name "Bold" is a family */
    first_style = n;
    while (first_style > 1 &&
           font_style_word(start[first_style - 1], len[first_style - 1]) != NULL)
        first_style--;

    join_tokens(out->family, start, len, 0, first_style);
    if (first_style == n)
        strcpy(out->style, "Regular");
    else
        join_tokens(out->style, start, len, first_style, n);
}
