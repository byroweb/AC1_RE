#include "ac1/ac1.h"
#include "m2c_compat.h"

extern void func_80014338(void *, s32, SVECTOR *);
extern s32 func_80071DC4(SVECTOR *, u8);
extern void func_8007732C(void *, s32, s32, s32, void *, void *);
extern void func_80078598(void *);
extern void func_801C5E40(void *, s32);
extern void func_801C5848();

void func_801C5A0C(u8 *o, u8 state)
{
    SVECTOR box[2];
    SVECTOR p;
    u8 *anim;

    anim = o + 0xCC;
    func_80014338(anim, 500, &p);
    p.vx += *(u16 *)(o + 8);
    p.vz += *(u16 *)(o + 0xC);
    box[0].vx = p.vx - 50;
    box[1].vx = p.vx + 50;
    box[0].vz = p.vz - 50;
    box[1].vz = p.vz + 50;
    box[0].vy = *(u16 *)(o + 0xA) - *(u16 *)(o + 0x2A);
    box[1].vy = *(u16 *)(o + 0xA) + 100;
    if (func_80071DC4(box, o[0x3A]) == 0) {
        func_801C5E40(o, 5);
        state = o[0x41];
    }
    o[0x3C] = state;
    func_80078598(o);
    func_8007732C(o, 0, 0, 0, anim, func_801C5848);
}
