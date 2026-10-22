#include "ac1/ac1.h"
/* ovl202 twin of ovl203 0x80050998: draw a box outline on the radar relative to the player AC */

void func_8004FEAC(s32 x, s32 z, s32 a, s32 b, SVECTOR *out);
void func_8004FF48(s32 x0, s32 y0, s32 x1, s32 y1, s32 color);

void func_80050088(SVECTOR *box, s32 color, s32 a, s32 b)
{
    SVECTOR pt[4];
    s32 px;
    s32 pz;
    s32 x0;
    s32 z0;
    s32 x1;
    s32 z1;

    px = g_ac_entities->pos.x;
    pz = g_ac_entities->pos.z;
    x0 = px - box[0].vx;
    x1 = px - box[1].vx;
    z0 = pz - box[0].vz;
    z1 = pz - box[1].vz;
    func_8004FEAC(x0, z0, a, b, &pt[0]);
    func_8004FEAC(x1, z0, a, b, &pt[1]);
    func_8004FEAC(x1, z1, a, b, &pt[2]);
    func_8004FEAC(x0, z1, a, b, &pt[3]);
    func_8004FF48(pt[0].vx, pt[0].vy, pt[1].vx, pt[1].vy, color);
    func_8004FF48(pt[1].vx, pt[1].vy, pt[2].vx, pt[2].vy, color);
    func_8004FF48(pt[2].vx, pt[2].vy, pt[3].vx, pt[3].vy, color);
    func_8004FF48(pt[3].vx, pt[3].vy, pt[0].vx, pt[0].vy, color);
}
