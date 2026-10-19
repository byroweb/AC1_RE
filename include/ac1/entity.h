/* include/ac1/entity.h - overlay 202 (mission) AC / actor entity.
 *
 * g_ac_entities (0x801A26B8) holds 16 AcEntity of 0x170 bytes.  Slot 0 is
 * the player; slots 1..15 are handed out by ac_entity_alloc (0x80078A2C,
 * li a1,15 from 0x801A2828, free when def == 0), and ac_entity_find_def
 * (0x80078ADC) scans all 16 (li a1,16).  The same array holds MTs, the
 * train, aircraft and enemy ACs (ENTITY_TYPES.md).
 *
 * Per frame, ac_entity_update (0x80078970) saves pos, calls fn_update,
 * refreshes section from the new pos, calls fn_think, and finally stores
 * the saved pos into prev_pos (so prev_pos is the start-of-frame copy).
 *
 * Sources: ENTITY_AI_FRAMEWORK.md, COMBAT_PHYSICS.md sections 4 and 6,
 * RUNTIME_RAM_MAP.md; conflicts between them were settled from the retail
 * code, see include/ac1/README.md.
 */
#ifndef AC1_ENTITY_H
#define AC1_ENTITY_H

#include "types.h"

struct AcEntity;

/* Three s16 axes plus a fourth half-word, copied as one 8-byte unit by
 * ac_entity_update (lwl/lwr pairs at 0x80078984). */
typedef struct AcPos {
    s16 x;                      /* +0x00 */
    s16 y;                      /* +0x02 */
    s16 z;                      /* +0x04 */
    s16 w;                      /* +0x06 */
} AcPos;

/* Movement / integrator block at AcEntity +0x64.  The mover 0x8004F024 is
 * called as (ent + 0x64, ent + 0x08, ...) and reads one 8-byte record per
 * axis at +0x20/+0x28/+0x30 of this block (live S-3, ENTITY_AI_FRAMEWORK.md).
 * Offsets in the field comments are AcEntity offsets. */
typedef struct AcMover {
    u8 mode;                    /* +0x64 */
    u8 weapon_slot;             /* +0x65 set from the ammo record's byte 11 on a shot */
    u8 unk66[0x0E];
    s16 fire_timer;             /* +0x74 cleared on every shot (0x8007CD04) */
    u8 unk76[0x0E];
    u8 axis_x[8];               /* +0x84 */
    u8 axis_y[8];               /* +0x8C */
    u8 axis_z[8];               /* +0x94 */
    u8 unk9c[0x18];
} AcMover;
AC1_ASSERT_SIZE(AcMover, 0x50);

typedef struct AcEntity {
    void *def;                  /* +0x00 definition record; 0 = free slot.  def+0x14 is
                                 *       a u16 whose bit 15 keeps the slot from being reused */
    void *geom;                 /* +0x04 geometry-block record table (0x8019F5xx) */
    AcPos pos;                  /* +0x08 world position X/Y/Z, written by the mover */
    u8 unk10[2];
    s16 yaw;                    /* +0x12 facing (HYPOTHESIS, Zinfidel ENTITY_YAW) */
    u8 unk14[2];
    s16 section;                /* +0x16 world-section index, refreshed every frame */
    u8 unk18[8];
    AcPos prev_pos;             /* +0x20 start-of-frame copy of pos */
    u8 unk28[0x0E];
    u16 faction;                /* +0x36 team in the low 3 bits */
    u16 ignore_mask;            /* +0x38 targeting skips scanners whose mask bits match */
    u8 unk3a[4];
    u8 ai_sub;                  /* +0x3E AI sub-state */
    u8 ai_sub2;                 /* +0x3F AI sub-state (also set by the fire path) */
    u8 ai_mode;                 /* +0x40 0xFF = special branch */
    u8 state;                   /* +0x41 passed as the 2nd argument of fn_state */
    u8 unk42[6];
    struct AcEntity *target;    /* +0x48 current target, rescanned every frame */
    void (*fn_update)(struct AcEntity *);           /* +0x4C first call of the frame */
    void (*fn_state)(struct AcEntity *, u8);        /* +0x50 (ent, ent->state) */
    void (*fn_move)(struct AcEntity *, s32, s32);   /* +0x54 movement state machine */
    void (*fn_think)(struct AcEntity *);            /* +0x58 called after the section refresh */
    void (*fn_hit)(struct AcEntity *);              /* +0x5C on-hit / death reaction */
    s32 unk60;
    AcMover mover;              /* +0x64 */
    u8 unkb4[0x94];
    u16 fire_flags;             /* +0x148 low nibble tested, |= 0xF0 on a shot */
    u8 unk14a[4];
    u8 flash_r;                 /* +0x14E muzzle-flash colour (180,160,140) */
    u8 flash_g;                 /* +0x14F */
    u8 flash_b;                 /* +0x150 */
    u8 unk151[0x0B];
    u32 flags;                  /* +0x15C AI flag bits (mission scripts set/toggle them) */
    u16 ap;                     /* +0x160 armour points, 4x the HUD value */
    u16 ap_max;                 /* +0x162 */
    u8 unk164[0x0C];
} AcEntity;
AC1_ASSERT_SIZE(AcEntity, 0x170);

/* Radar / lock-on list (exe data): g_target_head (0x80041020) then
 * g_target_list, 8 x 0x30 from 0x80041024 (COMBAT_PHYSICS.md section 7).
 * Slot 0's target is the lock-on target. */
typedef struct TargetSlot {
    AcEntity *target;           /* +0x00 0 = empty */
    s32 lock;                   /* +0x04 range / lock value */
    u16 unk08;
    s16 unk0a;
    u8 unk0c[4];
    s16 proj[3];                /* +0x10 projected coordinates (radar blip, screen) */
    u16 flags;                  /* +0x16 2 = in cone / lockable, 4 = in firing range */
    s16 aim[4];                 /* +0x18 */
    u8 unk20[0x10];
} TargetSlot;
AC1_ASSERT_SIZE(TargetSlot, 0x30);

#endif
