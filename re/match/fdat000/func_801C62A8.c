/* fdat000 (mission library, entry 0 @ 0x801C4B40; 31 twins, group g0016): attack
 * step.  With bit 15 of def+0x42 set, probe def+0x42 & 0xFFF units along the facing
 * (func_8006D5BC) and, if that hits an entity in g_ac_entities, fire at it
 * (func_800835E8 / func_800873F8 / func_800869F0).  Otherwise fetch the two points from
 * func_800745D0, send their midpoints with the previous ones (+0x10C) and then the new
 * points to func_8007A334, and keep the new points. */
#include "ac1/ac1.h"
typedef struct {
    AcPos p0;
    AcPos p1;
} Func801C62A8Pair;
typedef struct {
    u32 rgbc;           /* colour + code byte, stored as one word */
} Func801C62A8Col;
typedef struct {
    u8 pad0[0x2C];
    u8 b2c;             /* +0x2C */
    u8 b2d;             /* +0x2D */
    u8 pad2e[0x42 - 0x2E];
    u16 attack;         /* +0x42 bit 15 = probe, low 12 bits = range */
} Func801C62A8Def;
typedef struct {
    u8 pad0[0x08];
    AcPos pos;          /* +0x08 */
    u8 pad10[2];
    s16 yaw;            /* +0x12 */
    u8 pad14[0x6C - 0x14];
    u16 height;         /* +0x6C */
    u8 pad6e[0x10C - 0x6E];
    Func801C62A8Pair pts; /* +0x10C */
    u8 pad11c[0x170 - 0x11C];
} Func801C62A8Obj;
extern void func_80014728(s32 angle, AcPos *dir);
extern Func801C62A8Obj *func_8006D5BC(Func801C62A8Obj *o, AcPos *pos, s32 range, s32 height);
extern void func_800835E8(Func801C62A8Obj *o, Func801C62A8Obj *hit, void *p, AcPos *dir, AcPos *pos);
extern void func_800873F8(Func801C62A8Obj *hit, AcPos *at, s32 a, s32 b, s32 c, s32 d);
extern void func_800869F0(s32 a, AcPos *at, s32 b, s32 c, Func801C62A8Col *col);
extern s32 func_800745D0(Func801C62A8Obj *o, Func801C62A8Pair *pts, s32 a, s32 b, s32 c, s32 d, s32 e);
extern void func_8007A334(Func801C62A8Obj *o, Func801C62A8Pair *pts, void *p);
extern Func801C62A8Obj D_801A26B8[];
void func_801C62A8(Func801C62A8Obj *o, Func801C62A8Def *d, void *p)
{
    Func801C62A8Pair pts;
    Func801C62A8Pair mid;
    AcPos tgt;
    AcPos at;
    AcPos dir;
    Func801C62A8Col col; /* a struct: GCC 2.7 then keeps its store ahead of the d/o loads */
    s32 range, n;
    Func801C62A8Obj *hit;

    if (d->attack & 0x8000) {
        col.rgbc = 0xFA581C4A;
        range = d->attack & 0xFFF;
        func_80014728(o->yaw, &dir);
        tgt.x = o->pos.x + ((dir.x * range) >> 12);
        tgt.z = o->pos.z + ((dir.z * range) >> 12);
        tgt.y = o->pos.y;
        hit = func_8006D5BC(o, &o->pos, range, o->height);
        n = 10;
        if (hit != 0 && hit >= &D_801A26B8[0] && hit <= &D_801A26B8[15]) {
            dir.y = (hit->pos.y - (hit->height >> 1)) - (o->pos.y - (o->height >> 1));
            at.x = hit->pos.x - (dir.x >> 1);
            at.z = hit->pos.z - (dir.z >> 1);
            at.y = hit->pos.y - (o->height >> 1);
            func_800835E8(o, hit, p, &dir, &o->pos);
            func_800873F8(hit, &at, 20, 20, n, 1);
            func_800869F0(0, &at, 500, 6, &col);
        }
    } else if (func_800745D0(o, &pts, d->b2c, d->b2d, 2, 0, 1)) {
        mid.p0.x = (o->pts.p0.x + pts.p0.x) >> 1;
        mid.p0.y = (o->pts.p0.y + pts.p0.y) >> 1;
        mid.p0.z = (o->pts.p0.z + pts.p0.z) >> 1;
        mid.p1.x = (o->pts.p1.x + pts.p1.x) >> 1;
        mid.p1.y = (o->pts.p1.y + pts.p1.y) >> 1;
        mid.p1.z = (o->pts.p1.z + pts.p1.z) >> 1;
        func_8007A334(o, &mid, p);
        func_8007A334(o, &pts, p);
        o->pts.p0 = pts.p0;
        o->pts.p1 = pts.p1;
    }
}
