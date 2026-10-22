#include "ac1/ac1.h"

typedef struct {
    u8 b[8];
} Func801C6BBCBytes8;
typedef struct {
    void (*fn)();       /* +0x00 */
    u8 pad4[4];
    u8 tick;            /* +0x08 */
    u8 done;            /* +0x09 */
} Func801C6BBCTimer;
typedef struct {
    u16 x;              /* +0x00 */
    u16 y;              /* +0x02 */
    u16 z;              /* +0x04 */
    s16 slot;           /* +0x06 */
    u8 pad8[6];
    s16 flag;           /* +0x0E */
    u8 pad10[4];
    Func801C6BBCTimer *timer; /* +0x14 */
    u8 pad18[4];
    u16 flags;          /* +0x1C */
    u8 pad1E[2];
    u16 height;         /* +0x20 */
    u8 next;            /* +0x22 */
    u8 pad23;
    u8 snd;             /* +0x24 */
    u8 snd_b;           /* +0x25 */
    u16 snd_c;          /* +0x26 */
} Func801C6BBCObj;
typedef struct {
    u8 kind;            /* +0x00 */
    u8 pad1;
    u16 c;              /* +0x02 */
    u8 b;               /* +0x04 */
    u8 pad5[3];
    Func801C6BBCBytes8 pos; /* +0x08 */
} Func801C6BBCSnd;
typedef struct {
    u8 pad0[6];
    s16 slot;           /* +0x06 */
    u8 pad8[0x2C];
} Func801C6BBCKind;     /* 0x34 */
extern Func801C6BBCKind D_80198B08[];
extern u32 D_801AC860[];
extern u8 D_801CC4D8[];
extern void func_80073170();
extern void func_80086854(s32, SVECTOR *, void *);
extern void func_800873F8(void *, SVECTOR *, s32, s32, s32, s32);
extern void func_80087798(s32, void *, Func801C6BBCSnd *, u8, s32);
extern void func_801C674C();

s32 func_801C6BBC(Func801C6BBCObj *o, s32 hit)
{
    SVECTOR v;
    Func801C6BBCSnd snd;
    Func801C6BBCTimer *t;

    t = o->timer;
    if (hit == 0) {
        return;
    }
    if (t->done != 0) {
        return 0;
    }
    t->done = 1;
    o->flags |= 0x4000;
    func_80073170();
    if (D_801AC860[o->flag >> 5] & (1 << (o->flag & 0x1F))) {
        v.vx = o->x;
        v.vz = o->z;
        v.vy = o->y - (o->height >> 1);
        func_80086854(0, &v, D_801CC4D8);
        func_800873F8(o, &v, 10, 20, 8, 0);
    }
    if (o->snd != 0) {
        snd.kind = 23;
        snd.c = o->snd_c;
        snd.b = o->snd_b;
        snd.pos = *(Func801C6BBCBytes8 *)o;
        func_80087798(0, o, &snd, o->snd, 1);
    }
    if (o->next != 0) {
        o->slot = D_80198B08[o->next].slot;
        t->fn = func_801C674C;
        t->tick = 0;
    } else {
        o->slot = -1;
        o->timer = 0;
        o->flags |= 0x8000;
        func_80073170();
    }
    return 0;
}
