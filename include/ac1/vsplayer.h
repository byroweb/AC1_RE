/* include/ac1/vsplayer.h - the per-player record (pilot / AC state), 0x39C bytes.
 * Two blocks at 0x80040FA0 (g_players); D_80039D18 (g_player) points at the current
 * one (func_800780C0 switches it in the two-player overlay 203).  Overlay 202 reads
 * the first block through single globals around 0x80041000 (D_80040FA8 = +0x08,
 * D_80041020 = +0x80 g_target_head, D_80041024 = +0x84 g_target_list, ...), and
 * overlays 203/204 through D_80039D18.  In a versus match 0x80039D30 / 0x80039D34
 * (g_vs_sides[0] / [1]) point at this console's block and the other side's
 * (func_8004BF94: round result in flags10 & 7, wins at +0x390).
 *
 * Fields named from the overlay 203 matches first; the rest were merged from the 44
 * matched sources that declare the record locally (Pilot, Side3, CSide3, Player,
 * PlRec, VRec4, WSide4, HSide4, Pl3/Pl4, ...; tools/match/structsurvey.py merge
 * 0x80039D18).  A field is named when its use is clear or two sources agree; the
 * comment gives other names the matches used.  Include after ac1/ac1.h. */
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

/* one of the three weapon slots at VsPlayer +0x2DA */
typedef struct VsWeapon {
    u8 kind;                    /* +0x00 (type / kind) */
    u8 unk01[0x0C - 0x01];
    u8 flags;                   /* +0x0C */
    u8 unk0d;
    u16 max;                    /* +0x0E ammo capacity */
    u16 ammo;                   /* +0x10 rounds left (count) */
    u8 unk12[0x2A - 0x12];
    char name[0x12];            /* +0x2A shown on the HUD */
} VsWeapon;
AC1_ASSERT_SIZE(VsWeapon, 0x3C);

typedef struct VsPlayer {
    u8 unk000[0x08];
    u32 pad_cur;                /* +0x008 pad buttons (ovl202 D_80040FA8; held) */
    u32 pad_prev;               /* +0x00C */
    u16 flags10;                /* +0x010 HUD bits (flags in four matches; mode, btn) */
    u8 unk012[0x18 - 0x12];
    u8 mode;                    /* +0x018 (cur in two matches) */
    u8 weap_cur;                /* +0x019 selected weapon: weap[weap_cur] (cur, sel) */
    u8 unk01a[0x1E - 0x1A];
    u16 energy;                 /* +0x01E spent by func_8007B27C (en; hp in func_80050D78) */
    u16 flags20;                /* +0x020 bit 0 (ovl202 D_80040FC0) */
    u8 unk022[0x24 - 0x22];
    s32 lock;                   /* +0x024 lock-on value (shot); func_8007BBC4 reads a pointer */
    u8 unk028[0x2E - 0x28];
    s16 marker;                 /* +0x02E radar marker on (func_80052BF4, ovl204 twin) */
    SVECTOR marker_pos;         /* +0x030 */
    SVECTOR aim[2];             /* +0x038 last aim pair (func_80078FE4) */
    u8 unk048[0x4C - 0x48];
    struct VsCtx *ctx;          /* +0x04C (ovl202 D_80040FEC) */
    u8 unk050[0x80 - 0x50];
    TargetSlot *target;         /* +0x080 best target (target_head, best) */
    TargetSlot targets[8];      /* +0x084 radar / lock list (slot, items) */
    s32 range_sq;               /* +0x204 (range2, w204) */
    u8 unk208[0x210 - 0x208];
    u16 accel;                  /* +0x210 (ovl202 D_800411B0) */
    u16 maxspd;                 /* +0x212 (ovl202 D_800411B2) */
    u16 unk214;                 /* +0x214 passed to func_80078258 with +0x216 */
    u16 speed_cap;              /* +0x216 (ovl202 D_800411B6) */
    u16 turn_acc;               /* +0x218 func_80078258 rates for func_800789D8 */
    u16 turn_max;               /* +0x21A */
    u16 kick_scale;             /* +0x21C recoil = kick_scale * WeaponSpec.kick >> 8 (ovl202
                                 *        D_800411BC, func_8007D2F8) */
    u8 unk21e[0x220 - 0x21E];
    s16 scroll_pos;             /* +0x220 (ovl202 D_800411C0); the camera pitch / yaw in other
                                 *        matches (pitch x4, yaw x3) */
    s16 scroll_vel;             /* +0x222 */
    s16 scroll_timer;           /* +0x224 */
    SVECTOR box_a[2];           /* +0x226 two corner pairs func_80050D78 draws on the radar */
    SVECTOR box_b[2];           /* +0x236 */
    s16 warn;                   /* +0x246 HUD warning timers (func_80050D78) */
    s16 last_bar;               /* +0x248 */
    s16 beep;                   /* +0x24A */
    s16 beep_on;                /* +0x24C */
    u16 blink[2];               /* +0x24E; func_8007C740 calls +0x250 hit_timer */
    u8 unk252[0x254 - 0x252];
    u32 flags;                  /* +0x254 (flags in eight matches) */
    u8 stage;                   /* +0x258 */
    u8 unk259[0x261 - 0x259];
    u8 decel;                   /* +0x261 per-frame velocity decay (ovl202 D_80041201) */
    u8 icept;                   /* +0x262 */
    u8 icone;                   /* +0x263 */
    u16 energy_max;             /* +0x264 (en_max, hp_max) */
    u8 unk266[0x26E - 0x266];
    u16 dash_cost;              /* +0x26E */
    u16 range;                  /* +0x270 radar range, 0 = off (5000 when on) */
    u8 unk272[0x274 - 0x272];
    u8 dmg_scale[2];            /* +0x274 damage factor /64 for weapon kinds 0x10, 0x20 */
    u8 unk276;
    u8 radar_flags;             /* +0x277 */
    u8 unk278;
    u8 radar_mode;              /* +0x279 */
    u8 lock_on;                 /* +0x27A (lock; b27A in six matches) */
    u8 unk27b[0x290 - 0x27B];
    u16 key_up;                 /* +0x290 key map: twelve u16 button masks from +0x290 */
    u16 key_down;               /* +0x292 */
    u16 turn_l;                 /* +0x294 turn buttons (func_80078258) */
    u16 turn_r;                 /* +0x296 */
    u8 unk298[0x2A0 - 0x298];
    u16 key_left;               /* +0x2A0 (func_8007F414, func_80080FE4) */
    u16 mask_r;                 /* +0x2A2 scroll-right buttons (ovl202 +0x29A) */
    u16 unk2a4;                 /* +0x2A4 key_right in func_8007F414 / func_80080FE4 */
    u16 mask_l;                 /* +0x2A6 scroll-left buttons */
    u8 unk2a8[0x2AA - 0x2A8];
    u8 aim_arg[2];              /* +0x2AA */
    u8 unk2ac[0x2B0 - 0x2AC];
    u8 fx[4];                   /* +0x2B0 two (x, y) effect offsets (ovl202 D_80041250..53) */
    u8 unk2b4[0x2BE - 0x2B4];
    u16 flag_mask;              /* +0x2BE AI flag bits cleared with the fire nibble (func_80078B0C) */
    u8 unk2c0[0x2D8 - 0x2C0];
    u8 nweap;                   /* +0x2D8 weapons fitted */
    u8 unk2d9;
    VsWeapon weap[3];           /* +0x2DA */
    u8 unk38e[0x390 - 0x38E];
    u8 wins;                    /* +0x390 (func_8004BF94, ovl204 twin) */
    u8 unk391[0x394 - 0x391];
    struct AcEntity *ent;       /* +0x394 this player's AC (body, obj, view) */
    void *cam;                  /* +0x398 camera parameters (CamParm, ovl202_camera.h) */
} VsPlayer;
AC1_ASSERT_SIZE(VsPlayer, 0x39C);

extern VsPlayer *D_80039D18;    /* current player block (also g_player) */

/* --- symbols: GENERATED by tools/match/ctx.py gen from symbols.tsv; do not edit --- */
extern VsPlayer *g_player;                               /* 0x80039D18 */
extern VsPlayer *g_vs_sides[2];                          /* 0x80039D30 */
extern VsPlayer g_players[2];                            /* 0x80040FA0 */
/* --- end of generated symbols --- */

#endif
