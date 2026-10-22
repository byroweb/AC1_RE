#include "ac1/ac1.h"

typedef struct {
    s32 x;
    s32 y;
    s32 z;
} IVec3;

typedef struct {
    void *unk0;
    WorldStruct *ws;
} SegHit;

extern void *func_8006D534(AcEntity *e, SVECTOR *seg, SVECTOR *d, SVECTOR *out);
extern void func_800835E8(AcEntity *e, void *hit, s32 x, SVECTOR *d, SVECTOR *out);
extern void func_800873F8(void *hit, SVECTOR *out, s32 a, s32 b, s32 c, s32 f);
extern void func_800869F0(s32 a, SVECTOR *out, s32 b, s32 c, s32 *col);
extern SegHit *func_800705D8(IVec3 *v, SVECTOR *out, s32 k);

void func_8007A334(AcEntity *e, SVECTOR *seg, s32 x)
{
    SVECTOR d;
    SVECTOR out;
    IVec3 v[2];
    s32 col[1];
    s32 n;
    void *hit;
    SegHit *r;
    WorldStruct *w;

    col[0] = 0xFA581C4A;
    n = 10;
    d.vx = seg[1].vx - seg[0].vx;
    d.vy = seg[1].vy - seg[0].vy;
    d.vz = seg[1].vz - seg[0].vz;
    hit = func_8006D534(e, seg, &d, &out);
    if (hit != 0) {
        func_800835E8(e, hit, x, &d, &out);
        func_800873F8(hit, &out, 20, 20, n, 1);
        func_800869F0(0, &out, 500, 6, col);
    }
    v[0].x = seg[0].vx;
    v[0].y = seg[0].vy;
    v[0].z = seg[0].vz;
    v[1].x = seg[1].vx;
    v[1].y = seg[1].vy;
    v[1].z = seg[1].vz;
    r = func_800705D8(v, &out, 7);
    if (r != 0) {
        w = r->ws;
        if (*(u32 *)w->unk30 & 0x20000) {
            func_800873F8(hit, &out, 20, 20, n, 1);
            func_800869F0(0, &out, 500, 6, col);
        }
        func_800835E8(e, w, x, &d, &out);
    }
}
