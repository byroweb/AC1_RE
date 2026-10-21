#include "ac1/ac1.h"

typedef struct {
    u8 pad[0x3D];
    u8 sub3d;           /* +0x3D */
    u8 pad3e[0x8E];
    SVECTOR v;          /* +0xCC */
} AcSlide;

extern void func_8004DE88(AcMover *m, SVECTOR *out);
extern void func_80076548(AcEntity *e, s32 sub, void *fn);
extern void func_8004E98C(AcMover *m, s32 a, s32 b, s32 c);

void func_80076D20(AcEntity *e, s32 a, s32 b, void *fn)
{
    if (((AcSlide *)e)->sub3d != 18) {
        func_8004DE88(&e->mover, &((AcSlide *)e)->v);
        ((AcSlide *)e)->v.vy = 0;
        ((AcSlide *)e)->v.vx <<= 6;
        ((AcSlide *)e)->v.vz <<= 6;
    }
    if (((AcSlide *)e)->v.vx > -2 && ((AcSlide *)e)->v.vx < 2) {
        ((AcSlide *)e)->v.vx = 0;
    } else {
        ((AcSlide *)e)->v.vx -= ((AcSlide *)e)->v.vx >> 1;
    }
    if (((AcSlide *)e)->v.vz > -2 && ((AcSlide *)e)->v.vz < 2) {
        ((AcSlide *)e)->v.vz = 0;
    } else {
        ((AcSlide *)e)->v.vz -= ((AcSlide *)e)->v.vz >> 1;
    }
    func_80076548(e, 19, fn);
    func_8004E98C(&e->mover, a, b, 0);
}
