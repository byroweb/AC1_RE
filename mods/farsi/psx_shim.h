/*
 * psx_shim.h — PSX SDK type and macro substitutes for mipsel-linux-gnu-gcc
 *
 * Replaces libgte.h / libgpu.h / libetc.h for cross-compilation without
 * the PSY-Q SDK headers.
 */
#ifndef PSX_SHIM_H
#define PSX_SHIM_H

#include <stdint.h>

/* -------------------------------------------------------------------------
 * Basic PSX types
 * ---------------------------------------------------------------------- */
typedef uint8_t   u_char;
typedef uint16_t  u_short;
typedef uint32_t  u_long;
typedef int8_t    s_char;
typedef int16_t   s_short;
typedef int32_t   s_long;

/* -------------------------------------------------------------------------
 * GTE / GPU vector types
 * ---------------------------------------------------------------------- */
typedef struct { short vx, vy, vz, _pad; } SVECTOR;
typedef struct { u_char r, g, b, cd; }      CVECTOR;

/* DR_MODE: draw-mode change primitive (12 bytes, ilen=2) */
typedef struct {
    u_long tag;    /* +0x00: OT next-ptr (24-bit) | ilen (8-bit) = 2 */
    u_long code;   /* +0x04: GP0 0xE1 draw-mode command word          */
    u_long _pad;   /* +0x08: (unused)                                 */
} DR_MODE;

/* -------------------------------------------------------------------------
 * OT / primitive macros
 * ---------------------------------------------------------------------- */

/* setlen: store ilen in the HIGH byte (bits 24-31) of the OT tag word.
   The OT tag is (next_addr_24 | ilen_8<<24); AddPrim fills the low 24 bits later. */
#define setlen(p, n) \
    (((u_long *)(p))[0] = (((u_long *)(p))[0] & 0x00FFFFFFU) | ((u_long)(n) << 24))

/* AddPrim: link primitive p into OT bucket ot */
#define AddPrim(ot, p) do {                                              \
    u_long *_ot  = (u_long *)(ot);                                      \
    u_long *_p   = (u_long *)(p);                                       \
    *_p  = (*_p  & 0xFF000000U) | (*_ot  & 0x00FFFFFFU);               \
    *_ot = (*_ot & 0xFF000000U) | ((u_long)(_p) & 0x00FFFFFFU);        \
} while (0)

/* TermPrim: write an end-of-OT marker into p (tag = 0x00FFFFFF) */
#define TermPrim(p) (((u_long *)(p))[0] = 0x00FFFFFFU)

/* -------------------------------------------------------------------------
 * SetDrawMode — PSX SDK function at fixed RAM address 0x8002CEF4
 * ---------------------------------------------------------------------- */
extern void SetDrawMode(DR_MODE *p, int dfe, int dtd, int tpage, void *tw);

/* -------------------------------------------------------------------------
 * GTE (COP2) intrinsics
 *
 * PS1 uses MIPS I with COP2 = GTE. The lwc2/swc2 instructions load/store
 * 32-bit words to/from COP2 data registers. GCC with -march=r3000 supports
 * them via inline asm using the $cN register class.
 *
 * Register numbers (CP2 data):
 *   $0  VXY0  (vx | vy<<16)      $1  VZ0  (vz)
 *   $6  RGBC  (input colour)
 *   $22 RGB0  (output colour, NCCS result)
 * ---------------------------------------------------------------------- */
#define gte_ldv0(sv) __asm__ volatile (                 \
    "lwc2 $0, 0(%0)\n\tlwc2 $1, 4(%0)"                 \
    : : "r" (sv) : "memory")

#define gte_ldrgb(cv) __asm__ volatile (                \
    "lwc2 $6, 0(%0)"                                    \
    : : "r" (cv) : "memory")

#define gte_nccs() __asm__ volatile ("cop2 0x041E800")

#define gte_strgb(cv) __asm__ volatile (                \
    "swc2 $22, 0(%0)"                                   \
    : : "r" (cv) : "memory")

#endif /* PSX_SHIM_H */
