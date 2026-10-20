/*
 * group.h - the installed fonts, grouped into families and their styles
 *
 * Memory is allocated exactly: the list is built on machines with a few
 * hundred kilobytes, so there are no fixed-size arrays of names.
 */
#ifndef FONTDB_GROUP_H
#define FONTDB_GROUP_H

#include "fontdb/split.h"

/* what vqt_ext_name() calls font_format, plus one of ours */
#define FONT_FORMAT_BITMAP    0x01
#define FONT_FORMAT_SPEEDO    0x02
#define FONT_FORMAT_TRUETYPE  0x04
#define FONT_FORMAT_TYPE1     0x08
#define FONT_FORMAT_VECTOR    0x10  /* a vector font, format not reported */

/* one face as the VDI reports it */
typedef struct {
    short id;
    unsigned short format;
    unsigned short flags;
    const char *full_name;      /* always set */
    const char *family;         /* NULL or "": work it out from full_name */
    const char *style;          /* NULL or "": work it out from full_name */
} FontFace;

typedef struct {
    short id;
    unsigned short format;
    unsigned short flags;
    char *style;
} FontStyle;

typedef struct {
    char *name;
    FontStyle *styles;
    int style_count;
    unsigned short formats;     /* the styles' formats, ORed */
} FontFamily;

typedef struct {
    FontFamily *families;
    int family_count;
} FontList;

void fontlist_init(FontList *list);

/* 0 on success, -1 if out of memory (the list stays usable) */
int fontlist_add(FontList *list, const FontFace *face);

/* families by name, ignoring case; styles by weight, upright before slanted */
void fontlist_sort(FontList *list);

/* the family called name, ignoring case, or NULL */
const FontFamily *fontlist_find(const FontList *list, const char *name);

void fontlist_free(FontList *list);

/* the sort key of a style name: weight * 2 + 1 if it is italic or oblique */
int font_style_rank(const char *style);

#endif
