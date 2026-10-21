#include "ac1/ac1.h"

typedef struct {
    u8 pad[0x28];
    u16 h28;            /* +0x28 */
    u16 h2a;            /* +0x2A */
    u8 pad2c[0x11];
    u8 sub3d;           /* +0x3D */
    u8 pad3e[0x32];
    s16 h70;            /* +0x70 */
    u8 pad72[0x5A];
    SVECTOR vcc;        /* +0xCC */
} AcFall;

extern SVECTOR D_80039C78;
extern long SquareRoot0(long a);
extern void func_80076548(AcEntity *e, s32 sub, void *fn);
extern void func_8004E98C(AcMover *m, s32 a, s32 b, s32 c);
extern s32 func_8004E06C(SVECTOR *d, SVECTOR *out, s32 r, s32 h, s32 k);
extern s32 func_800749A0(AcEntity *e, AcPos *pos, SVECTOR *tmp, SVECTOR *v);
extern void func_8004DFF8(AcMover *m, s32 n, SVECTOR *out);

void func_80077170(AcEntity *e, s32 a, s32 b, SVECTOR *v, void *fn)
{
    SVECTOR d;
    SVECTOR out;
    SVECTOR tmp;
    s32 n;

    if (((AcFall *)e)->sub3d != 18) {
        ((AcFall *)e)->vcc = D_80039C78;
    }
    func_80076548(e, 18, fn);
    if (a != -1) {
        func_8004E98C(&e->mover, a, b, 0);
    } else {
        ((AcFall *)e)->h70 = b;
    }
    d.vx = v->vx >> 6;
    d.vy = v->vy >> 6;
    d.vz = v->vz >> 6;
    d.vy += SquareRoot0(d.vx * d.vx + d.vz * d.vz) >> 1;
    n = func_8004E06C(&d, &out, ((AcFall *)e)->h28 >> 1, ((AcFall *)e)->h2a >> 1, ((u8 *)&e->mover)[4]);
    if (func_800749A0(e, &e->pos, &tmp, &out) != 0) {
        v->vx = (out.vx << 6) / n;
        v->vy = (out.vy << 6) / n;
        v->vz = (out.vz << 6) / n;
    }
    func_8004DFF8(&e->mover, n, &out);
}
