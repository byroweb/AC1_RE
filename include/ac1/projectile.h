/* include/ac1/projectile.h - overlay 202 projectile pool and weapon defs.
 *
 * g_proj_pool (0x801AEB3C): 256 x 0xE0.  proj_alloc (0x80089ECC) scans for
 * the first slot whose free half-word is -1 (lh 6, addiu 224, li a1,256)
 * and returns &g_proj_none (0x8008EC88) when the pool is full.  proj_init
 * (0x80085764) copies an 8-byte position into +0x00, owner into +0x08,
 * clears free/kind/think, sets +0x0C/+0x12 to -1, +0x11 to 0xFF and
 * computes the world section (+0x0E).
 *
 * The generic straight-line think 0x800851A8 (kind 1) proves the motion
 * fields: pos += vel, ang += spin, tint += dtint, all per frame.  Other
 * kinds reuse +0x18..+0xDF differently (guided missiles treat +0x18..+0x1A
 * as type/step/phase, COMBAT_PHYSICS.md section 5), so treat the names
 * past +0x17 as the kind-1 meaning.
 */
#ifndef AC1_PROJECTILE_H
#define AC1_PROJECTILE_H

#include "types.h"

typedef struct Projectile {
    s16 x;                      /* +0x00 position */
    s16 y;                      /* +0x02 */
    s16 z;                      /* +0x04 */
    s16 free;                   /* +0x06 -1 = free slot, 0 = live */
    void *owner;                /* +0x08 */
    s16 unk0c;                  /* +0x0C -1 at init */
    s16 section;                /* +0x0E world-section index */
    u8 kind;                    /* +0x10 1 ballistic, 2 missile, 3 and 4 other */
    u8 unk11;                   /* +0x11 0xFF at init */
    s16 unk12;                  /* +0x12 -1 at init */
    void (*think)(struct Projectile *);   /* +0x14 per-frame handler */
    u8 dtint[3];                /* +0x18 added to tint each frame */
    u8 unk1b;
    s16 vel[3];                 /* +0x1C added to x/y/z each frame */
    s16 unk22;
    s16 spin[3];                /* +0x24 added to ang each frame */
    u8 unk2a[0x8E];
    s16 ang[3];                 /* +0xB8 */
    s16 unkbe;
    u8 tint[3];                 /* +0xC0 */
    u8 unkc3;
    u8 unkc4[0x18];
    s32 unkdc;                  /* +0xDC cleared by one init path (0x800857FC) */
} Projectile;
AC1_ASSERT_SIZE(Projectile, 0xE0);

/* g_weapon_defs (0x8008E530): weapon ids 0..19, 0x1C each.  The fire path
 * indexes it as (id * 8 - id) * 4 at 0x8007C88C and reads +0x08 with lh.
 * Ids 20.. use other tables (0x28 stride from 0x8008FC48 for 20..39,
 * 0x24 stride from 0x8008FEE8 for 40..59).  Field meanings other than
 * range are HYPOTHESIS from one rifle entry (COMBAT_PHYSICS.md section 4). */
typedef struct WeaponDef {
    u16 model;                  /* +0x00 type / model id */
    u16 speed;                  /* +0x02 */
    u8 unk04[4];
    s16 range;                  /* +0x08 range / life in frames (lh) */
    s16 unk0a;
    s16 damage;                 /* +0x0C */
    u8 unk0e[0x0A];
    u8 tracer[3];               /* +0x18 tracer RGB */
    u8 unk1b;
} WeaponDef;
AC1_ASSERT_SIZE(WeaponDef, 0x1C);

/* g_weapon_spreads (0x8008E610): 9 rows, indexed by the low nibble of a unit weapon's
 * type (fdat000 func_801C88EC) or by the player weapon kind (func_8007D068). */
typedef struct WeaponSpread {
    u8 shot_a;                  /* +0x00 copied into the shot record (func_801C88EC) */
    u8 shot_c;                  /* +0x01 */
    u8 burst[2];                /* +0x02 rounds per barrel, both barrels (func_8007D068) */
    u16 spread_x;               /* +0x04 random spread, centred: rand(n) - n / 2 */
    u16 spread_y;               /* +0x06 */
} WeaponSpread;
AC1_ASSERT_SIZE(WeaponSpread, 8);

/* g_weapon_specs (0x8008E658): 7 rows, indexed by the player's arm-weapon id
 * (func_8007D2F8) and by the low nibble of a unit weapon's type (fdat000
 * func_801C7DA8).  Retail rows: range 500 or 600, kick 32 or 256. */
typedef struct WeaponSpec {
    u16 range;                  /* +0x00 aim range */
    u16 kick;                   /* +0x02 recoil, times VsPlayer.kick_scale >> 8 */
    u8 trig;                    /* +0x04 0 = semi-automatic, 1 = automatic */
    u8 cool;                    /* +0x05 cool-down (2, or 36 for row 3) */
} WeaponSpec;
AC1_ASSERT_SIZE(WeaponSpec, 6);

#endif
