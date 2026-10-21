#include "ac1/ac1.h"
/* ovl202 0x80071610 - point the probe record at the polygon's corner vertices; if the
 * polygon's per-axis extreme corners overlap the box on x, y and z, func_800714C4 */
typedef struct Probe {
    u8 unk00[0x5C];
    s32 min[3];                 /* +0x5C */
    s32 max[3];                 /* +0x68 */
    u8 unk74[0x20];
    CollMesh *mesh;             /* +0x94 */
    u8 unk98[4];
    u32 info;                   /* +0x9C polygon copy (CollPoly; n at +0x9E): word 0 holds the corner picks */
    u8 unkA0[4];
    s32 d;                      /* +0xA4 */
    u32 v01;                    /* +0xA8 */
    u32 v23;                    /* +0xAC */
    u8 unkB0[0x10];
    u16 *v[4];                  /* +0xC0 */
} Probe;
s32 func_8006EFFC(s32 a0, s32 a1, s32 b0, s32 b1);
void func_800714C4(s32 verts, Probe *o);

void func_80071610(Probe *o)
{
    s32 verts;

    verts = (s32)o->mesh->verts;
    o->v[0] = (u16 *)(verts + (o->v01 & 0x7FF8));
    o->v[1] = (u16 *)(verts + (((u16 *)&o->v01)[1] & 0x7FF8));
    o->v[2] = (u16 *)(verts + (o->v23 & 0x7FF8));
    if (o->info & 1) {
        o->v[3] = (u16 *)(verts + (((u16 *)&o->v23)[1] & 0x7FF8));
    }
    if (func_8006EFFC(o->min[0], o->max[0], (s16)o->v[(o->info >> 4) & 3][0], (s16)o->v[(o->info >> 10) & 3][0])
        && func_8006EFFC(o->min[1], o->max[1], (s16)o->v[(o->info >> 6) & 3][1], (s16)o->v[(o->info >> 12) & 3][1])
        && func_8006EFFC(o->min[2], o->max[2], (s16)o->v[(o->info >> 8) & 3][2], (s16)o->v[(o->info >> 14) & 3][2])) {
        func_800714C4(verts, o);
    }
}
