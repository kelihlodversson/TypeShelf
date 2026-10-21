/*
 * vdi_extra.h - VDI calls gemlib has but does not declare
 */
#ifndef VDI_IF_VDI_EXTRA_H
#define VDI_IF_VDI_EXTRA_H

/* NVDI's, SpeedoGDOS's and FSMGDOS's arbitrary point size; see
 * gem_source_open() for how support is detected */
short vst_arbpt(short handle, short point, short *wchar, short *hchar, short *wcell, short *hcell);

/* NVDI 4's and pTOS's character map: MAP_UNICODE (2) makes the WORDs of
 * v_gtext16n() Unicode.  Returns the mode now in force, or -1 on a VDI
 * without it. */
short vst_map_mode(short handle, short mode);

#endif
