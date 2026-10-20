#include <string.h>
#include "test.h"
#include "fontdb/split.h"

static void check_split(const char *full, const char *family, const char *style)
{
    FontNameParts parts;

    font_split_name(full, &parts);
    CHECK_STR(parts.family, family);
    CHECK_STR(parts.style, style);
}

void test_split(void)
{
    FontNameParts parts;
    char longname[200];
    int i;

    check_split("Arial", "Arial", "Regular");
    check_split("Arial Bold", "Arial", "Bold");
    check_split("Arial Bold Italic", "Arial", "Bold Italic");
    check_split("DejaVu Sans Mono Bold Oblique", "DejaVu Sans Mono", "Bold Oblique");
    check_split("Arial Rounded MT Bold", "Arial Rounded MT", "Bold");
    check_split("  Dutch 801   Roman ", "Dutch 801", "Roman");
    check_split("arial BOLD", "arial", "BOLD");

    /* width words are part of the family */
    check_split("Arial Narrow", "Arial Narrow", "Regular");
    check_split("Arial Narrow Bold", "Arial Narrow", "Bold");

    /* a name made only of style words is a family */
    check_split("Bold", "Bold", "Regular");
    check_split("Black Italic", "Black", "Italic");

    check_split("", "", "Regular");
    check_split("   ", "", "Regular");

    /* too long: cut, never overrun */
    memset(longname, 'x', sizeof longname - 1);
    longname[sizeof longname - 1] = '\0';
    font_split_name(longname, &parts);
    CHECK(strlen(parts.family) < SPLIT_NAME_MAX);

    /* more words than tokens: still terminates */
    longname[0] = '\0';
    for (i = 0; i < 50; i++)
        strcat(longname, "a ");
    font_split_name(longname, &parts);
    CHECK(strlen(parts.family) < SPLIT_NAME_MAX);

    CHECK(font_style_word("bold", 4) != NULL);
    CHECK(font_style_word("bolder", 6) == NULL);
    CHECK_INT(font_style_word("Heavy", 5)->weight, 8);
    CHECK_INT(font_style_word("Oblique", 7)->slant, 1);
}
