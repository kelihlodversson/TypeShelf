#ifndef FONTDB_BUILD_H
#define FONTDB_BUILD_H

#include "fontdb/group.h"
#include "vdi_if/vdi_if.h"

/*
 * Read every face from src into out, grouped and sorted.  Faces the source
 * cannot give are skipped.  Returns the number of faces added, or -1 if out of
 * memory (out is then empty).  Free out with fontlist_free().
 */
int fontdb_build(const VdiFontSource *src, FontList *out);

#endif
