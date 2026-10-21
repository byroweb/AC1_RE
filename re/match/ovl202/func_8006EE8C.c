#include "ac1/ac1.h"
/* ovl202 0x8006EE8C - func_80071610 for an SVECTOR box: if the polygon's extreme
 * corners overlap the box on every axis, its func_8006EB10 hit (0 / -1 = none) */
typedef struct Probe2 {
    u8 unk00[0x64];
    SVECTOR min;                /* +0x64 */
    SVECTOR max;                /* +0x6C */
    u8 unk74[0x20];
    CollMesh *mesh;             /* +0x94 */
    u8 unk98[4];
    u32 info;                   /* +0x9C polygon copy (CollPoly): word 0 holds the corner picks */
    u8 unkA0[4];
    s32 d;                      /* +0xA4 */
    u32 v01;                    /* +0xA8 */
    u32 v23;                    /* +0xAC */
    u8 unkB0[0x10];
    u16 *v[4];                  /* +0xC0 */
} Probe2;
s32 func_8006EFFC(s32 a0, s32 a1, s32 b0, s32 b1);
s32 func_8006EB10(s32 verts, Probe2 *o);


s32 func_8006EE8C(Probe2 *o)
{
    s32 verts;
    s32 r;

    verts = (s32)o->mesh->verts;
    o->v[0] = (u16 *)(verts + (o->v01 & 0x7FF8));
    o->v[1] = (u16 *)(verts + (((u16 *)&o->v01)[1] & 0x7FF8));
    o->v[2] = (u16 *)(verts + (o->v23 & 0x7FF8));
    if (o->info & 1) {
        o->v[3] = (u16 *)(verts + (((u16 *)&o->v23)[1] & 0x7FF8));
    }
    if (!func_8006EFFC(o->min.vx, o->max.vx, (s16)o->v[(o->info >> 4) & 3][0], (s16)o->v[(o->info >> 10) & 3][0])) {
        return 0;
    }
    if (!func_8006EFFC(o->min.vy, o->max.vy, (s16)o->v[(o->info >> 6) & 3][1], (s16)o->v[(o->info >> 12) & 3][1])) {
        return 0;
    }
    if (func_8006EFFC(o->min.vz, o->max.vz, (s16)o->v[(o->info >> 8) & 3][2], (s16)o->v[(o->info >> 14) & 3][2]) && (r = func_8006EB10(verts, o)) != 0 && r != -1) {
        return r;
    }
    return 0;
}
