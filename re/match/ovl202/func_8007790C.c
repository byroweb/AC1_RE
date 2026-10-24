#include "ac1/ac1.h"

typedef struct {
    u8 pad[0x28];
    u16 h28;            /* +0x28 */
    u16 h2a;            /* +0x2A */
    u8 pad2c[0xA0];
    SVECTOR vcc;        /* +0xCC */
} AcFall;

#define F(e) ((AcFall *)(e))

extern long SquareRoot0(long a);
extern void func_80076548(AcEntity *e, s32 sub, void *fn);
extern void func_8004E98C(AcMover *m, s32 a, s32 b, s32 c);
extern s32 func_8004ED1C(AcMover *m);
extern s32 func_8004E06C(SVECTOR *d, SVECTOR *out, s32 r, s32 h, s32 k);
extern s32 func_800749A0(AcEntity *e, AcPos *pos, SVECTOR *tmp, SVECTOR *v);
extern void func_8004DFF8(AcMover *m, s32 n, SVECTOR *out);

void func_8007790C(AcEntity *e, s32 a, s32 b, s32 c, SVECTOR *v, s32 lim, void *fn)
{
    SVECTOR d;
    SVECTOR out;
    SVECTOR tmp;
    s32 n;
    s32 r;
    s32 k;
    s32 x;

    F(e)->vcc = *v;
    F(e)->vcc.vy = 0;
    k = SquareRoot0(F(e)->vcc.vx * F(e)->vcc.vx + F(e)->vcc.vz * F(e)->vcc.vz);
    if (k != 0) {
        r = k;
        x = k < 0x80;
        k = k - 0x80;
        if (x) {
            k = 0;
        }
        F(e)->vcc.vx = F(e)->vcc.vx * k / r;
        F(e)->vcc.vz = F(e)->vcc.vz * k / r;
    }
    func_80076548(e, 0x80, fn);
    func_8004E98C(&e->mover, a, b, c);
    if (lim != -1) {
        s32 t = func_8004ED1C(&e->mover);
        if (t < lim) {
            lim = t;
        }
        F(e)->vcc.pad = lim;
    }
    d.vx = F(e)->vcc.vx >> 6;
    d.vy = F(e)->vcc.vy >> 6;
    d.vz = F(e)->vcc.vz >> 6;
    d.vy += SquareRoot0(d.vx * d.vx + d.vz * d.vz) >> 1;
    k = n = func_8004E06C(&d, &out, F(e)->h28 >> 1, F(e)->h2a >> 1, ((u8 *)&e->mover)[4]);
    if (func_800749A0(e, &e->pos, &tmp, &out) != 0) {
        F(e)->vcc.vx = (out.vx << 6) / n;
        F(e)->vcc.vy = (out.vy << 6) / n;
        F(e)->vcc.vz = (out.vz << 6) / n;
    }
    func_8004DFF8(&e->mover, n, &out);
}
