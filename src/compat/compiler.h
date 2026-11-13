/*
 * compiler.h - stand-in for the mintlib header gemlib's headers include
 *
 * MiNTLib toolchains supply this header themselves. The libcmini-based pTOS
 * SDK does not; its m68k stack ABI and the x32 builds use this shim instead.
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
