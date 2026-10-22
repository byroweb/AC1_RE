#include "ac1/ac1.h"
/* ovl203 0x80050998 - draw a box outline on the radar, relative to the pilot (four corners through func_800507B8) */
typedef struct Body {
    u8 unk00[8];
    s16 x;                      /* +0x08 */
    u8 unk0A[2];
    s16 z;                      /* +0x0C */
} Body;
typedef struct Pilot {
    u8 unk000[0x394];
    Body *body;                 /* +0x394 */
} Pilot;
extern Pilot *D_80039D18;
void func_800507B8(s32 x, s32 z, s32 a, s32 b, SVECTOR *out);
void func_80050858(s32 x0, s32 y0, s32 x1, s32 y1, s32 color);


void func_80050998(SVECTOR *box, s32 color, s32 a, s32 b)
{
    SVECTOR pt[4];
    s32 px;
    s32 pz;
    s32 x0;
    s32 z0;
    s32 x1;
    s32 z1;
    Body *p;

    p = D_80039D18->body;
    px = p->x;
    pz = p->z;
    x0 = px - box[0].vx;
    x1 = px - box[1].vx;
    z0 = pz - box[0].vz;
    z1 = pz - box[1].vz;
    func_800507B8(x0, z0, a, b, &pt[0]);
    func_800507B8(x1, z0, a, b, &pt[1]);
    func_800507B8(x1, z1, a, b, &pt[2]);
    func_800507B8(x0, z1, a, b, &pt[3]);
    func_80050858(pt[0].vx, pt[0].vy, pt[1].vx, pt[1].vy, color);
    func_80050858(pt[1].vx, pt[1].vy, pt[2].vx, pt[2].vy, color);
    func_80050858(pt[2].vx, pt[2].vy, pt[3].vx, pt[3].vy, color);
    func_80050858(pt[3].vx, pt[3].vy, pt[0].vx, pt[0].vy, color);
}
