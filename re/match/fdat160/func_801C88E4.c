/* fdat160 0x801C88E4  size 268  zero-agent sweep (tools/loop/sweep.py): neighbour+imm draft of fdat008 0x801C6AB0 (fdat008/func_801C6AB0.c, similarity 0.985), 2026-10-22 */
#include "ac1/ac1.h"

typedef struct {
    u8 pad0[8];
    u8 tick;            /* +0x08 */
    u8 pad9[3];
    u32 range;          /* +0x0C */
} Func801C6AB0Timer;
typedef struct {
    AcPos pos;          /* +0x00 */
    u8 pad8[0x0C];
    Func801C6AB0Timer *timer; /* +0x14 */
} Func801C6AB0Obj;
extern void func_801C89F0(Func801C6AB0Obj *, SVECTOR *, SVECTOR *, SVECTOR *, s32);

void func_801C88E4(Func801C6AB0Obj *o)
{
    SVECTOR b;
    SVECTOR a;
    Func801C6AB0Timer *t;
    AcEntity *e;
    s32 i;
    s32 dx, dy, dz;

    t = o->timer;
    if (!(++t->tick & 7)) {
        e = g_ac_entities;
        for (i = 2; i != 0; e++, i--) {
            if (e->def != 0) {
                dx = (e->pos.x - o->pos.x) >> 2;
                dy = (e->pos.y - o->pos.y) >> 2;
                dz = (e->pos.z - o->pos.z) >> 2;
                if ((u32)(dx * dx + dy * dy + dz * dz) < t->range) {
                    func_801C89F0(o, &a, &b, &b, 0);
                }
            }
        }
    }
}
