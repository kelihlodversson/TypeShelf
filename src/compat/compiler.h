/*
 * compiler.h - stand-in for the mintlib header gemlib's headers include
 *
 * On m68k it comes with the toolchain's sysroot.  The ARM and x86 builds have
 * no such sysroot, so they put this directory on the include path instead.
 */
#ifndef TYPESHELF_COMPAT_COMPILER_H
#define TYPESHELF_COMPAT_COMPILER_H

#ifdef __cplusplus
#define __BEGIN_DECLS extern "C" {
#define __END_DECLS   }
#else
#define __BEGIN_DECLS
#define __END_DECLS
#endif

#define __CDECL

#endif
