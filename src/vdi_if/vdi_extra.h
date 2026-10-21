/*
 * vdi_extra.h - VDI calls gemlib has but does not declare
 */
#ifndef VDI_IF_VDI_EXTRA_H
#define VDI_IF_VDI_EXTRA_H

/* NVDI's, SpeedoGDOS's and FSMGDOS's arbitrary point size; see
 * gem_source_open() for how support is detected */
short vst_arbpt(short handle, short point, short *wchar, short *hchar, short *wcell, short *hcell);

#endif
