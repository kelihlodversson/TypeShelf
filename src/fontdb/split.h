/*
 * split.h - split a full font name into family and style
 *
 * The VDI reports one name per face ("Dutch 801 Bold Italic").  Where
 * vqt_xfntinfo() is not there to say which part is the family and which the
 * style, the style is taken to be the trailing weight and slope words.
 * Width words (Condensed, Narrow, ...) stay in the family: "Arial Narrow" is
 * a family of its own.
 */
#ifndef FONTDB_SPLIT_H
#define FONTDB_SPLIT_H

#include <stddef.h>

#define SPLIT_NAME_MAX 64

typedef struct {
    char family[SPLIT_NAME_MAX];
    char style[SPLIT_NAME_MAX];
} FontNameParts;

/* a word that can be part of a style name */
typedef struct {
    const char *word;
    int weight;     /* 0 thin ... 3 regular ... 8 black; -1 if not a weight */
    int slant;      /* 1 for italic and oblique */
} FontStyleWord;

/* the style word equal to s[0..len), ignoring case, or NULL */
const FontStyleWord *font_style_word(const char *s, size_t len);

/* never fails; a name with no style word gets the style "Regular" */
void font_split_name(const char *full, FontNameParts *out);

#endif
