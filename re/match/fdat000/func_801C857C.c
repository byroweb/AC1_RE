/* fdat000 (mission library, entry 0 @ 0x801C4B40; 31 twins, group g0016): fire a
 * single-shot weapon.  The projectile's range comes from the weapon table selected by
 * the high byte of w->type (0x2000/0x4000/0x6000 tables, 600 for 0x7000), or, for
 * type < 0x100, the whole shot record is filled from the 28-byte table at 0x8008E530.
 * Then aim (801C7B1C) and, while the mover is idle (+0x65 == 255) and the cool-down at
 * +0x3F has run out, fire through func_80077B78 with the muzzle flash and AI flag. */
#include "ac1/ac1.h"
typedef struct {
    u8 b[4];
} Func801C857CTag;
typedef struct {
    u32 w0;             /* +0x00 */
    u32 w4;             /* +0x04 */
    s16 range;          /* +0x08 */
    u8 pada[2];
    u16 h0c;            /* +0x0C */
    u16 h0e;            /* +0x0E */
    u16 h10;            /* +0x10 */
    u16 h12;            /* +0x12 */
    u8 pad14[4];
    Func801C857CTag tag; /* +0x18 */
} Func801C857CSpec;
typedef struct {
    u8 pad[8];
    s16 range;          /* +0x08 */
} Func801C857CRange;
typedef struct {
    u8 pad0[2];
    u16 type;           /* +0x02 */
    u8 b4;              /* +0x04 */
    u8 proj_a;          /* +0x05 */
    u8 sub_a;           /* +0x06 */
    u8 proj_b;          /* +0x07 */
    u8 sub_b;           /* +0x08 */
    u8 pad9[2];
    s8 recoil;          /* +0x0B */
    u8 rest;            /* +0x0C */
    u8 padd[2];
    u8 chance;          /* +0x0F */
    u8 clip;            /* +0x10 */
    u8 pad11;
    u16 h12;            /* +0x12 */
    u8 pad14[0x20 - 0x14];
    u8 cool;            /* +0x20 */
    u8 pad21;
    u8 b22;             /* +0x22 */
    u8 pad23;
    u16 h24;            /* +0x24 */
    u8 b26;             /* +0x26 */
} Func801C857CWeapon;
typedef struct {
    u8 pad0[0x3E];
    u8 queued;          /* +0x3E */
    u8 cool;            /* +0x3F */
    u8 pad40[0x65 - 0x40];
    u8 weapon_slot;     /* +0x65 AcMover.weapon_slot, 255 = ready */
    u8 pad66[0x7C - 0x66];
    s16 recoil;         /* +0x7C */
    u8 pad7e[0x148 - 0x7E];
    u16 fire_flags;     /* +0x148 */
    u8 pad14a[0x14E - 0x14A];
    u8 flash_r;         /* +0x14E */
    u8 flash_g;         /* +0x14F */
    u8 flash_b;         /* +0x150 */
    u8 pad151[0x15C - 0x151];
    u32 flags;          /* +0x15C */
} Func801C857CObj;
typedef struct {
    u16 proj;           /* +0x00 */
    u16 sub;            /* +0x02 */
    s16 range;          /* +0x04 */
    u16 h06;            /* +0x06 */
    u16 h08;            /* +0x08 */
    u16 h0a;            /* +0x0A */
    u16 h0c;            /* +0x0C */
    u16 h0e;            /* +0x0E */
    u16 b10;            /* +0x10 */
    Func801C857CTag tag; /* +0x12 */
    u8 pad16[2];
    u32 w18;            /* +0x18 */
    u32 w1c;            /* +0x1C */
    u8 pad20[8];        /* the frame has room for 0x28 bytes */
} Func801C857CShot;
extern Func801C857CSpec D_8008E530[];
extern struct { Func801C857CRange r; u8 pad[40 - 10]; } D_8008FC40[];
extern struct { Func801C857CRange r; u8 pad[36 - 10]; } D_8008FEE0[];
extern struct { Func801C857CRange r; u8 pad[24 - 10]; } D_8008FB50[];
extern s32 func_801C7B1C(Func801C857CObj *o, Func801C857CWeapon *w, s32 flags, s32 range);
extern s32 func_8002A4A0(void);
extern void func_80077B78();
void func_801C857C(Func801C857CObj *o, Func801C857CWeapon *w)
{
    Func801C857CShot shot;
    Func801C857CSpec *spec;
    s32 side = 0;
    s32 range, hit, n;

    if (w->type & 0xFF00) {
        switch (w->type & 0xF000) {
        case 0x2000:
            range = D_8008FC40[(w->type & 0xF00) >> 8].r.range;
            break;
        case 0x4000:
            range = D_8008FEE0[(w->type & 0xF00) >> 8].r.range;
            break;
        case 0x6000:
            range = D_8008FB50[(w->type & 0xF00) >> 8].r.range;
            break;
        case 0x7000:
            range = 600;
            break;
        }
    } else {
        spec = &D_8008E530[w->type & 0xF];
        shot.range = range = spec->range;
        shot.h08 = spec->h0c;
        shot.h0a = spec->h0e;
        shot.h0c = spec->h10;
        shot.h0e = spec->h12;
        shot.b10 = w->b4;
        shot.tag = spec->tag;
        shot.w18 = spec->w0;
        shot.w1c = spec->w4;
    }
    hit = func_801C7B1C(o, w, 0, range);
    o->recoil = w->recoil;
    if (o->weapon_slot != 255)
        return;
    if (o->cool == 0) {
        if (o->queued == 0) {
            if (hit == 0)
                return;
            if (w->chance < (func_8002A4A0() & 0xFF))
                return;
            o->queued = w->clip;
        }
        if (!(w->type & 0x40)) {
            shot.proj = w->proj_a;
            shot.sub = w->sub_a;
        } else if (o->queued & 1) {
            shot.proj = w->proj_a;
            shot.sub = w->sub_a;
        } else {
            shot.proj = w->proj_b;
            shot.sub = w->sub_b;
        }
        if (w->type & 0x20)
            shot.proj |= 0x8000;
        shot.proj |= w->type & 0xFF00;
        shot.h06 = w->h12 + 400;
        n = 1;
        func_80077B78(o, w->b22, w->h24, w->b26, &shot, n, side, side + 1);
        o->flash_r = 180;
        o->flash_g = 160;
        o->flash_b = 140;
        o->fire_flags |= 0xF0;
        o->flags |= n << shot.sub;
        if (--o->queued == 0) {
            o->weapon_slot = w->rest;
            o->cool = w->cool;
        }
    } else {
        o->cool--;
    }
}
