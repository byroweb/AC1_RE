/* fdat000 (mission library, entry 0 @ 0x801C4B40; 31 twins, group g0009): per-frame
 * AI driver.  Mask the AI flags with def+0x38 (plus bit def+0x2D in modes 47/56).
 * A dying entity (+0x40 != 255) spawns two debris effects and runs its death sequence.
 * Otherwise: entering the target node's box triggers fn_move(9); a weapon change
 * (+0x132) resets the burst counters; direction 32/33 stops here; a visible target
 * within 1000 height units (func_80073C2C) starts the alert (direction 32/33, or
 * 34 -> 35); finally the current weapon record (def+0x4E, 0x28 bytes each) is fired
 * by type: 0/128 single (801C857C), 1 burst (801C88EC), 2 801C7DA8. */
#include "ac1/ac1.h"
typedef struct {
    u16 type;           /* +0x00 */
    u8 rest[0x28 - 2];
} Func801C8CBCWeapon;
typedef struct {
    u8 b0;              /* +0x00 */
    u8 pad1[0x2D - 0x01];
    u8 b2d;             /* +0x2D */
    u8 pad2e[0x38 - 0x2E];
    u16 flag_mask;      /* +0x38 */
    u8 pad3a[0x3E - 0x3A];
    u16 alert;          /* +0x3E */
    u8 pad40[0x42 - 0x40];
    u16 sight;          /* +0x42 */
    u16 alert_time;     /* +0x44 */
    u8 pad46[0x4B - 0x46];
    u8 death_fx;        /* +0x4B */
    u16 n_weapons;      /* +0x4C */
    Func801C8CBCWeapon weapons[1]; /* +0x4E */
} Func801C8CBCDef;
typedef struct {
    AcPos pos;          /* +0x00 */
    u8 pad08[0x22 - 0x08];
    s8 lo[3];           /* +0x22 */
    s8 hi[3];           /* +0x25 */
    u8 pad28[0x34 - 0x28];
} Func801C8CBCNode;
typedef struct {
    u8 pad0[0x10];
    AcPos pos;          /* +0x10 */
    u8 pad18[0x30 - 0x18];
} Func801C8CBCHit;
typedef struct {
    u8 pad0[0x12];
    u16 life;           /* +0x12 */
} Func801C8CBCFx;
typedef struct {
    u8 pad0[4];
    u8 at[0x20 - 4];    /* +0x04 */
} Func801C8CBCDebris;
typedef struct Func801C8CBCObj Func801C8CBCObj;
struct Func801C8CBCObj {
    u8 pad0[0x08];
    AcPos pos;          /* +0x08 */
    u8 x10[2];          /* +0x10 */
    s16 yaw;            /* +0x12 */
    u8 pad14[0x36 - 0x14];
    u16 faction;        /* +0x36 */
    u8 pad38[0x3C - 0x38];
    u8 dir;             /* +0x3C */
    u8 kind;            /* +0x3D */
    u8 queued;          /* +0x3E */
    u8 cool;            /* +0x3F */
    u8 ai_mode;         /* +0x40 */
    u8 pad41[0x48 - 0x41];
    Func801C8CBCObj *target; /* +0x48 */
    u8 pad4c[0x54 - 0x4C];
    void (*fn_move)(Func801C8CBCObj *, s32); /* +0x54 */
    u8 pad58[0x60 - 0x58];
    void *model;        /* +0x60 */
    u8 mode;            /* +0x64 */
    u8 weapon_slot;     /* +0x65 */
    u8 pad66[0x7C - 0x66];
    s16 recoil;         /* +0x7C */
    u8 pad7e[0x108 - 0x7E];
    Func801C8CBCDef *def; /* +0x108 */
    u8 pad10c[0x126 - 0x10C];
    s16 node;           /* +0x126 */
    u8 pad128[0x12E - 0x128];
    u16 alert;          /* +0x12E */
    u8 pad130;
    u8 weapon;          /* +0x131 */
    u8 next_weapon;     /* +0x132 */
    u8 shots;           /* +0x133 */
    u8 rounds;          /* +0x134 */
    u8 pad135[0x148 - 0x135];
    u16 fire_flags;     /* +0x148 */
    u8 pad14a[0x15C - 0x14A];
    u32 flags;          /* +0x15C */
};
extern Func801C8CBCNode D_80198B08[];
extern u8 D_801CBB7C[][36];
extern u8 D_801CBBC4[];
extern u8 D_8019F52E[];
extern u8 D_801CBBCC[];
extern u8 D_801CBBD0[];
extern void func_80078598(Func801C8CBCObj *o);
extern s32 func_800564AC(void *model, s32 a, AcPos *pos, void *b, s32 n, Func801C8CBCDebris *out);
extern s32 func_8002A4A0(void);
extern Func801C8CBCFx *func_80086F20(s32 a, void *at, s32 b, s32 c, s32 d, void *e, void *f, void *g, void *h);
extern s32 func_800754D0(Func801C8CBCObj *o, void *fx);
extern void func_80078E1C(Func801C8CBCObj *o);
extern void func_8007C790(Func801C8CBCObj *o);
extern s32 func_80073C2C(void *target, AcPos *pos, s32 yaw, s32 zero, s32 flags, s32 faction,
                         s16 yaw_min, s16 yaw_max, s16 pitch_min, s16 pitch_max, s16 range,
                         Func801C8CBCHit *hit);
extern void func_801C857C(Func801C8CBCObj *o, Func801C8CBCWeapon *w);
extern void func_801C88EC(Func801C8CBCObj *o, Func801C8CBCWeapon *w);
extern void func_801C7DA8(Func801C8CBCObj *o, Func801C8CBCWeapon *w);
void func_801C8CBC(Func801C8CBCObj *o)
{
    Func801C8CBCDef *d = o->def;
    Func801C8CBCDebris debris[3];
    Func801C8CBCHit hit;
    Func801C8CBCDebris *p;
    Func801C8CBCNode *n;
    Func801C8CBCWeapon *w;
    s32 i;

    func_80078598(o);
    o->fire_flags &= 0xF;
    if (o->mode == 56 || o->mode == 47)
        o->flags &= d->flag_mask | (1 << d->b2d);
    else
        o->flags &= d->flag_mask;
    if (o->ai_mode != 255) {
        if (func_800564AC(o->model, o->pos.w + 128, &o->pos, o->x10, 2, debris)) {
            for (i = 2, p = debris; i != 0; i--, p++) {
                func_80086F20(0, p->at, 41, 20, (func_8002A4A0() & 1) + 3, D_801CBBC4, D_8019F52E,
                              D_801CBBCC, D_801CBBD0)->life = 80;
            }
        }
        if (func_800754D0(o, D_801CBB7C[d->death_fx]))
            func_80078E1C(o);
        return;
    }
    if (o->node != -1) {
        n = &D_80198B08[o->node];
        if (o->pos.x >= n->pos.x + (n->lo[0] << 8) && n->pos.x + (n->hi[0] << 8) >= o->pos.x
            && o->pos.y >= n->pos.y + (n->lo[1] << 8) - 8 && n->pos.y + (n->hi[1] << 8) >= o->pos.y
            && o->pos.z >= n->pos.z + (n->lo[2] << 8) && n->pos.z + (n->hi[2] << 8) >= o->pos.z
            && o->fn_move)
            o->fn_move(o, 9);
    }
    if (o->cool == 0 && o->weapon != o->next_weapon) {
        o->weapon = o->next_weapon;
        if (o->dir == 34 && d->b0 == 0) {
            o->dir = 35;
            o->faction |= 0x10;
        }
        o->shots = 0;
        o->rounds = 0;
        o->queued = 0;
    }
    if (o->weapon_slot != 255)
        func_8007C790(o);
    o->recoil = -1;
    if (o->dir == 32 || o->dir == 33)
        return;
    if (o->alert == 0) {
        if (o->target != 0 && d->alert != 0
            && (o->pos.y - o->target->pos.y >= 0 ? o->pos.y - o->target->pos.y
                                                 : o->target->pos.y - o->pos.y) < 1000
            && func_80073C2C(o->target, &o->pos, o->yaw, 0, 0, o->faction, d->sight & 0xFFF,
                             -512, 512, -512, 512, &hit)
            && (hit.pos.w & 2)) {
            if (!(o->faction & 0x10)) {
                if (o->kind == 192) {
                    o->dir = 33;
                    o->faction |= 0x10;
                } else {
                    o->dir = 32;
                    o->faction |= 0x10;
                }
                o->alert = d->alert_time;
                return;
            }
            if (o->dir == 34 && d->b0 == 0) {
                o->dir = 35;
                o->faction |= 0x10;
                return;
            }
        }
    } else {
        o->alert--;
    }
    if (d->n_weapons == 0)
        return;
    w = &d->weapons[o->weapon];
    switch (w->type) {
    case 0:
    case 128:
        func_801C857C(o, w);
        break;
    case 1:
        func_801C88EC(o, w);
        break;
    case 2:
        func_801C7DA8(o, w);
        break;
    }
}
