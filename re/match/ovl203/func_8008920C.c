#include "ac1/ac1.h"

extern void func_80014728(s32 ang, SVECTOR *out);
extern void func_80070398(SVECTOR *pos, s32 a, s32 b, SVECTOR *dir, SVECTOR *out, s32 c, s32 d);
extern void func_800890A4(s32 kind, SVECTOR *dir);

void func_8008920C(SVECTOR *p, SVECTOR *rot)
{
    SVECTOR pos;
    SVECTOR dir;
    SVECTOR hit;
    SVECTOR save;

    func_80014728(rot->vy, &dir);
    pos.vx = p->vx + ((dir.vx * 125) >> 11);
    pos.vz = p->vz + ((dir.vz * 125) >> 11);
    pos.vy = p->vy;
    save = dir;
    dir.vx >>= 5;
    dir.vz >>= 5;
    func_80070398(&pos, 200, 575, &dir, &hit, -1, 7);
    func_800890A4(0x8100, &save);
}
