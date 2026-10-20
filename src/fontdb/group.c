#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "fontdb/group.h"

static char *dup_str(const char *s)
{
    size_t n = strlen(s) + 1;
    char *p = malloc(n);

    if (p != NULL)
        memcpy(p, s, n);
    return p;
}

static int ci_compare(const char *a, const char *b)
{
    while (*a != '\0' && *b != '\0') {
        int ca = tolower((unsigned char)*a);
        int cb = tolower((unsigned char)*b);

        if (ca != cb)
            return ca - cb;
        a++;
        b++;
    }
    return tolower((unsigned char)*a) - tolower((unsigned char)*b);
}

void fontlist_init(FontList *list)
{
    list->families = NULL;
    list->family_count = 0;
}

int fontlist_add(FontList *list, const FontFace *face)
{
    FontNameParts parts;
    const char *family = face->family;
    const char *style = face->style;
    FontFamily *fam = NULL;
    FontStyle *styles;
    int i;

    if (family == NULL || family[0] == '\0' || style == NULL || style[0] == '\0') {
        font_split_name(face->full_name, &parts);
        family = parts.family;
        style = parts.style;
    }

    for (i = 0; i < list->family_count; i++) {
        if (ci_compare(list->families[i].name, family) == 0) {
            fam = &list->families[i];
            break;
        }
    }
    if (fam == NULL) {
        FontFamily *grown = realloc(list->families,
                                    (size_t)(list->family_count + 1) * sizeof(FontFamily));

        if (grown == NULL)
            return -1;
        list->families = grown;
        fam = &list->families[list->family_count];
        fam->name = dup_str(family);
        if (fam->name == NULL)
            return -1;
        fam->styles = NULL;
        fam->style_count = 0;
        fam->formats = 0;
        list->family_count++;
    }

    styles = realloc(fam->styles, (size_t)(fam->style_count + 1) * sizeof(FontStyle));
    if (styles == NULL)
        return -1;
    fam->styles = styles;
    styles[fam->style_count].style = dup_str(style);
    if (styles[fam->style_count].style == NULL)
        return -1;
    styles[fam->style_count].id = face->id;
    styles[fam->style_count].format = face->format;
    styles[fam->style_count].flags = face->flags;
    fam->style_count++;
    fam->formats |= face->format;
    return 0;
}

int font_style_rank(const char *style)
{
    int weight = 3;     /* regular */
    int slant = 0;
    const char *p = style;

    while (*p != '\0') {
        const char *start;
        const FontStyleWord *w;

        while (*p == ' ')
            p++;
        if (*p == '\0')
            break;
        start = p;
        while (*p != '\0' && *p != ' ')
            p++;
        w = font_style_word(start, (size_t)(p - start));
        if (w != NULL) {
            if (w->weight >= 0)
                weight = w->weight;
            if (w->slant)
                slant = 1;
        }
    }
    return weight * 2 + slant;
}

static int cmp_family(const void *a, const void *b)
{
    return ci_compare(((const FontFamily *)a)->name, ((const FontFamily *)b)->name);
}

static int cmp_style(const void *a, const void *b)
{
    const FontStyle *sa = a;
    const FontStyle *sb = b;
    int ra = font_style_rank(sa->style);
    int rb = font_style_rank(sb->style);

    if (ra != rb)
        return ra - rb;
    return ci_compare(sa->style, sb->style);
}

void fontlist_sort(FontList *list)
{
    int i;

    if (list->family_count > 1)
        qsort(list->families, (size_t)list->family_count, sizeof(FontFamily), cmp_family);
    for (i = 0; i < list->family_count; i++) {
        FontFamily *fam = &list->families[i];

        if (fam->style_count > 1)
            qsort(fam->styles, (size_t)fam->style_count, sizeof(FontStyle), cmp_style);
    }
}

const FontFamily *fontlist_find(const FontList *list, const char *name)
{
    int i;

    for (i = 0; i < list->family_count; i++) {
        if (ci_compare(list->families[i].name, name) == 0)
            return &list->families[i];
    }
    return NULL;
}

void fontlist_free(FontList *list)
{
    int i, j;

    for (i = 0; i < list->family_count; i++) {
        FontFamily *fam = &list->families[i];

        for (j = 0; j < fam->style_count; j++)
            free(fam->styles[j].style);
        free(fam->styles);
        free(fam->name);
    }
    free(list->families);
    fontlist_init(list);
}
