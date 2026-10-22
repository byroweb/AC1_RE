#include "ac1/ac1.h"

typedef struct {
    u8 pad[0x28];
    u16 h28;            /* +0x28 */
    u16 h2a;            /* +0x2A */
    u8 pad2c[0x0E];
    u8 kind;            /* +0x3A */
    u8 pad3b[0x2F];
    u16 h6a;            /* +0x6A */
    u16 h6c;            /* +0x6C */
    u8 pad6e[0x5E];
    SVECTOR vcc;        /* +0xCC */
} AcLand;

typedef struct {
    SVECTOR lo;
    SVECTOR hi;
} Box16;

extern SVECTOR D_80039C78;
extern long SquareRoot0(long a);
extern void func_80076D20(AcEntity *e, s32 a, s32 b, void *fn);
extern s32 func_8004E06C(SVECTOR *d, SVECTOR *out, s32 r, s32 h, s32 k);
extern s32 func_800749A0(AcEntity *e, AcPos *pos, SVECTOR *tmp, SVECTOR *v);
extern void func_80014338(SVECTOR *v, s32 r, Box16 *box);
extern void *func_80071DC4(Box16 *box, s32 kind);
extern void func_8004DFF8(AcMover *m, s32 n, SVECTOR *out);

#define l ((AcLand *)e)

void func_80076F60(AcEntity *e, s32 a, s32 b, void *fn)
{
    SVECTOR d;
    SVECTOR out;
    SVECTOR tmp;
    Box16 box;
    s32 n;

    func_80076D20(e, a, b, fn);
    d.vx = l->vcc.vx >> 6;
    d.vy = l->vcc.vy >> 6;
    d.vz = l->vcc.vz >> 6;
    d.vy += SquareRoot0(d.vx * d.vx + d.vz * d.vz) >> 1;
    n = func_8004E06C(&d, &out, l->h28 >> 1, l->h2a >> 1, ((u8 *)&e->mover)[4]);
    func_800749A0(e, &e->pos, &tmp, &out);
    if (out.vy > -3 && out.vy < 3) {
        func_80014338(&out, l->h6a, &box);
        box.hi.vx = box.lo.vx += e->pos.x;
        box.lo.vy += e->pos.y;
        box.hi.vy = box.lo.vy + 64;
        box.lo.vy -= l->h6c;
        box.hi.vz = box.lo.vz += e->pos.z;
        if (func_80071DC4(&box, l->kind) != 0) {
            l->vcc.vx = (out.vx << 6) / n;
            l->vcc.vy = (out.vy << 6) / n;
            l->vcc.vz = (out.vz << 6) / n;
            func_8004DFF8(&e->mover, n, &out);
            return;
        }
    }
    l->vcc = D_80039C78;
    func_8004DFF8(&e->mover, 2, &D_80039C78);
}
