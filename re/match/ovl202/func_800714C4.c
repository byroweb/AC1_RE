#include "ac1/ac1.h"
/* ovl202 0x800714C4 - set up the probe polygon record (corner slots, normal, y range)
 * and run func_80071244 at the four x/z corners of the box */
typedef struct Probe {
    u8 unk00[0x5C];
    s32 min[3];                 /* +0x5C */
    s32 max[3];                 /* +0x68 */
    u8 unk74[0x2A];
    u16 n[3];                   /* +0x9E */
    u8 unkA4[8];
    u32 info;                   /* +0xAC */
    u8 unkB0[0x10];
    u16 *v[4];                  /* +0xC0 */
    u8 unkD0[0x2C];
    u16 c[8];                   /* +0xFC */
    u16 n2[3];                  /* +0x10C */
    s16 y0;                     /* +0x112 */
    s16 y1;                     /* +0x114 */
} Probe;
void func_80071244(Probe *o, s32 x, s32 z);



void func_800714C4(s32 unused, Probe *o)
{
    s32 x0;
    s32 x1;
    s32 z0;
    s32 z1;

    o->c[0] = o->v[0][2];
    o->c[1] = o->v[0][0];
    o->c[2] = o->v[1][2];
    o->c[3] = o->v[1][0];
    o->c[4] = o->v[2][2];
    o->c[5] = o->v[2][0];
    if ((o->info & 0x7FF80000) != 0x7FF80000) {
        o->c[6] = o->v[3][2];
        o->c[7] = o->v[3][0];
    }
    x0 = (s16)o->min[0];
    o->n2[0] = o->n[1];
    o->n2[1] = o->n[0];
    o->n2[2] = o->n[2];
    o->y0 = o->min[1];
    z0 = (s16)o->min[2];
    o->y1 = o->max[1];
    x1 = o->max[0];
    z1 = o->max[2];
    func_80071244(o, x0, z0);
    func_80071244(o, (s16)x1, z0);
    func_80071244(o, x0, (s16)z1);
    func_80071244(o, (s16)x1, (s16)z1);
}
