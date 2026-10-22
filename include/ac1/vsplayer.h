/* include/ac1/vsplayer.h - overlay 203 (two-player versus) per-player block.
 * Two 0x39C-byte blocks at D_80040FA0; D_80039D18 points at the current one
 * (func_800780C0 switches it).  Much of what overlay 202 keeps in single
 * globals around 0x80041000 lives here, at other offsets.  Fields named from
 * the overlay 203 matches; the rest is padding.  Include after ac1/ac1.h. */
#ifndef AC1_VSPLAYER_H
#define AC1_VSPLAYER_H

#include "types.h"

typedef struct VsCtx {
    u8 slot0;                   /* +0x00 */
    u8 unk01[3];
    u8 slot1;                   /* +0x04 */
    u8 unk05[0x11 - 5];
    u8 id;                      /* +0x11 */
} VsCtx;

typedef struct VsPlayer {
    u8 unk000[0x08];
    u32 pad_cur;                /* +0x008 pad buttons (ovl202 D_80040FA8) */
    u32 pad_prev;               /* +0x00C */
    u8 unk010[0x1E - 0x10];
    u16 energy;                 /* +0x01E spent by func_8007B27C */
    u16 flags20;                /* +0x020 bit 0 (ovl202 D_80040FC0) */
    u8 unk022[0x38 - 0x22];
    SVECTOR aim[2];             /* +0x038 last aim pair (func_80078FE4) */
    u8 unk048[0x4C - 0x48];
    struct VsCtx *ctx;          /* +0x04C (ovl202 D_80040FEC) */
    u8 unk050[0x210 - 0x50];
    u16 accel;                  /* +0x210 (ovl202 D_800411B0) */
    u16 maxspd;                 /* +0x212 (ovl202 D_800411B2) */
    u16 unk214;                 /* +0x214 passed to func_80078258 with +0x216 */
    u16 speed_cap;              /* +0x216 (ovl202 D_800411B6) */
    u16 turn_acc;               /* +0x218 func_80078258 rates for func_800789D8 */
    u16 turn_max;               /* +0x21A */
    u8 unk21c[0x220 - 0x21C];
    s16 scroll_pos;             /* +0x220 (ovl202 D_800411C0) */
    s16 scroll_vel;             /* +0x222 */
    s16 scroll_timer;           /* +0x224 */
    u8 unk226[0x261 - 0x226];
    u8 decel;                   /* +0x261 per-frame velocity decay (ovl202 D_80041201) */
    u8 unk262[0x274 - 0x262];
    u8 dmg_scale[2];            /* +0x274 damage factor /64 for weapon kinds 0x10, 0x20 */
    u8 unk276[0x294 - 0x276];
    u16 turn_l;                 /* +0x294 turn buttons (func_80078258) */
    u16 turn_r;                 /* +0x296 */
    u8 unk298[0x2A2 - 0x298];
    u16 mask_r;                 /* +0x2A2 scroll-right buttons (ovl202 +0x29A) */
    u16 unk2a4;
    u16 mask_l;                 /* +0x2A6 scroll-left buttons */
    u8 unk2a8[0x2AA - 0x2A8];
    u8 aim_arg[2];              /* +0x2AA */
    u8 unk2ac[0x2B0 - 0x2AC];
    u8 fx[4];                   /* +0x2B0 two (x, y) effect offsets (ovl202 D_80041250..53) */
    u8 unk2b4[0x2BE - 0x2B4];
    u16 flag_mask;              /* +0x2BE AI flag bits cleared with the fire nibble (func_80078B0C) */
    u8 unk2c0[0x39C - 0x2C0];
} VsPlayer;
AC1_ASSERT_SIZE(VsPlayer, 0x39C);

extern VsPlayer *D_80039D18;    /* current player block */

#endif
