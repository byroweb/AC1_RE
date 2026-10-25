/* fdat022: carrier / dropship think.  Lowers the units it carries until they
 * land, then flies a keyframed path (14-byte records at 0x801C7538, values
 * interpolated from the previous record by the countdown +0x44), playing the
 * engine sound and spawning the next unit at path events. */
#include "ac1/ac1.h"
typedef struct {
    u8 type;            /* +0x00 */
    u8 idx;             /* +0x01 */
    u8 pad2[0x2C - 2];
} Func801C60B0Slot;
typedef struct {
    s16 h0;             /* +0x00 */
    s16 dy;             /* +0x02 */
    s16 spd;            /* +0x04 */
    s16 h6;             /* +0x06 */
    u16 dur;            /* +0x08 */
    u8 event;           /* +0x0A */
    u8 vol;             /* +0x0B */
    u8 pitch;           /* +0x0C */
    u8 mode;            /* +0x0D */
} Func801C60B0Key;
typedef struct {
    u8 pad0[4];
    void *node;         /* +0x04 */
    u8 pad8[0x0E - 0x08];
    u16 hE;             /* +0x0E */
} Func801C60B0Hit;
typedef struct {
    u8 pad0[0x0E];
    u16 hE;             /* +0x0E */
} Func801C60B0Gnd;
typedef struct {
    u8 pad0[0x08];
    s16 x, y, z;        /* +0x08 */
    s16 sect;           /* +0x0E */
    u8 pad10[2];
    s16 yaw;            /* +0x12 */
    u8 pad14[2];
    s16 h16;            /* +0x16 */
    u8 pad18[0x2C - 0x18];
    s16 top;            /* +0x2C */
    u16 h2e;            /* +0x2E */
    s16 h30;            /* +0x30 */
    u8 pad32[0x40 - 0x32];
    u8 ai_mode;         /* +0x40 */
    u8 pad41;
    u8 step;            /* +0x42 */
    u8 pad43;
    u16 timer;          /* +0x44 */
    u8 pad46[0x58 - 0x46];
    void *fn_think;     /* +0x58 */
    u8 pad5c[0x64 - 0x5C];
    u8 mover[0xCE - 0x64]; /* +0x64 */
    u16 fall;           /* +0xCE */
    u8 padd0[0xFC - 0xD0];
    u16 delay;          /* +0xFC */
    u8 slot0;           /* +0xFE */
    u8 nslots;          /* +0xFF */
    u8 hold;            /* +0x100 */
    u8 pad101[0x108 - 0x101];
    s16 floor;          /* +0x108 */
    u8 spawned;         /* +0x10A */
    u8 landed;          /* +0x10B */
    u8 pad10c[0x170 - 0x10C];
} Func801C60B0Obj;
extern Func801C60B0Slot D_8019FAB8[];
extern Func801C60B0Obj D_801A26B8[];
extern void (**D_8019F51C)(Func801C60B0Obj *, s32);
extern s32 D_801A5DC0;
extern s32 D_801A5DC8;
extern Func801C60B0Key D_801C7538[];
extern u8 D_801C75F0[];
extern u8 D_801D0B68[][64];
extern void func_80014728(s32 yaw, SVECTOR *v);
extern void func_8004CC30(s32 a, s32 b, void *pos, s32 vol, s32 c, s32 d, s32 pitch);
extern void func_8004E98C(u8 *mover, s32 a, s32 b, s32 c);
extern s32 func_8004EBF8(u8 *mover);
extern void func_8004ECA8(u8 *mover);
extern Func801C60B0Gnd *func_8006D4BC(s32 a);
extern s16 func_8006E6B4(s32 sect, void *pos, s32 dy);
extern Func801C60B0Hit *func_80075030(Func801C60B0Obj *o, s32 r);
extern s32 func_800754D0(Func801C60B0Obj *o, void *fx);
extern void func_80078E1C(Func801C60B0Obj *o);
extern Func801C60B0Obj *func_801C6044(Func801C60B0Slot *s);
void func_801C60B0(Func801C60B0Obj *o)
{
    SVECTOR v;
    s16 spare[8];
    Func801C60B0Slot *s;
    Func801C60B0Obj *e;
    Func801C60B0Obj *child;
    Func801C60B0Key *k;
    Func801C60B0Gnd *g;
    Func801C60B0Hit *h;
    s32 n;
    s32 spd;
    s32 dy;
    s32 t;
    s32 fl;
    s32 c;
    u16 top;

    if (o->delay != 0) {
        o->delay--;
        return;
    }
    fl = 0;
    n = o->nslots;
    s = &D_8019FAB8[o->slot0];
    while (--n != -1) {
        if (s->type == 1) {
            e = &D_801A26B8[s->idx];
            if (e->fn_think == 0) {
                e->y += (s16)e->fall >> 6;
                c = e->y < o->floor;
                fl = o->floor;
                if (!c) {
                    if (e->hold == 0)
                        e->y = fl;
                    D_8019F51C[e->sect + 8](e, fl);
                    o->landed++;
                } else {
                    e->fall += 256;
                }
            }
        }
        s++;
    }
    if (o->ai_mode != 255) {
        h = (Func801C60B0Hit *)func_8006D4BC(1);
        if (h != 0)
            o->h2e = h->hE;
        else
            o->h2e = 0x7FFF;
        o->y += 50;
        if (o->y >= o->top)
            o->ai_mode = 0;
        if (func_800754D0(o, D_801C75F0))
            func_80078E1C(o);
        return;
    }
    if (o->timer == 0) {
        k = &D_801C7538[++o->step];
        o->timer = k->dur;
    } else {
        k = &D_801C7538[o->step];
    }
    func_80014728(o->yaw, &v);
    spd = (k[-1].spd - k->spd) * o->timer / k->dur + k->spd;
    o->x += (v.vx * spd) >> 12;
    o->z += (v.vz * spd) >> 12;
    c = k->dur;
    c = (k[-1].dy - k->dy) * o->timer / c;
    dy = k->dy + c;
    o->y += dy;
    o->h16 = func_8006E6B4(o->h16, &o->x, dy);
    func_8004CC30(133, D_801A5DC0 + 0x1000, &o->x,
                  (k[-1].vol - k->vol) * o->timer / k->dur + k->vol, 13000, 14000,
                  (k[-1].pitch - k->pitch) * o->timer / k->dur + k->pitch);
    D_801A5DC8 = 1;
    h = func_80075030(o, 0x1800);
    if (h != 0) {
        o->top = h->hE;
        t = (u8 (*)[64])h->node - D_801D0B68;
    } else {
        o->top = 0x7FFF;
        t = -1;
    }
    o->h30 = t;
    D_801A5DC8 = 0;
    switch (k->mode) {
    case 0:
        func_8004E98C(o->mover, 0, 0, 0);
        func_8004ECA8(o->mover);
        break;
    case 1:
        func_8004E98C(o->mover, 1, 1, 0);
        func_8004ECA8(o->mover);
        break;
    case 2:
        func_8004E98C(o->mover, 1, -1, 0);
        func_8004ECA8(o->mover);
        break;
    case 3:
        func_8004E98C(o->mover, 2, 1, 0);
        func_8004ECA8(o->mover);
        if (!func_8004EBF8(o->mover))
            func_8004CC30(134, D_801A5DC0 + 0x1100, &o->x, 127, 13000, 14000, 0);
        break;
    case 4:
        func_8004E98C(o->mover, 2, -1, 0);
        func_8004ECA8(o->mover);
        if (!func_8004EBF8(o->mover))
            func_8004CC30(134, D_801A5DC0 + 0x1100, &o->x, 127, 13000, 14000, 0);
        break;
    }
    switch (k->event) {
    case 1:
        if (o->spawned != o->nslots && o->timer == 2) {
            top = o->top;
            o->floor = top;
            if (child->hold == 1)
                o->floor = top - 2000;
            child = func_801C6044(&D_8019FAB8[o->slot0 + o->spawned]);
            child->x = o->x;
            child->y = o->y;
            child->z = o->z;
            child->h16 = o->h16;
            child->fall = 0;
            o->timer = 20;
            o->spawned++;
        }
        break;
    case 9:
        if ((u16)(o->x + 32000) > 64000 || (u16)(o->z + 32000) > 64000)
            func_80078E1C(o);
        break;
    }
    o->timer--;
}
