/* ovl202 0x8007B994  size 152  permuter tier (tools/loop/permute.py): levers args func_8004DE88 st+ proto, from the file draft, 2026-10-20 */
#include "ac1/ac1.h"

typedef struct {
    u8 pad[0x3C];
    u8 b3c;
    u8 pad3d[0x8F];
    SVECTOR vcc;        /* +0xCC */
} AcJump;

extern void func_8004DE88(AcMover *m, SVECTOR *out, s32);
extern void func_8007732C(AcEntity *e, s32 a, s32 b, s32 c, SVECTOR *v, void *fn);
extern void func_8007B160(AcEntity *e);

void func_8007B994(AcEntity *e, s32 a, s32 st)
{
    SVECTOR *v;

    v = &((AcJump *)e)->vcc;
    func_8004DE88(&e->mover, v, st);
    v->vx <<= 6;
    v->vz <<= 6;
    func_8007732C(e, a, 1, 512, v, func_8007B160);
    ((AcJump *)e)->b3c = st;
    e->state = st;
}
