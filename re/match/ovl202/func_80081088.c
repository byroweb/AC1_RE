#include "ac1/ac1.h"
/* ovl202 0x80081088: put a mission actor into the airborne move state (0xB0, step
 * 0x80080DA0) with velocity *v; clipped against the ground, v updated from the result
 * (sibling of 0x80077170). */
typedef struct {
    u8 pad[0x28];
    u16 h28;            /* +0x28 */
    u16 h2a;            /* +0x2A */
    u8 pad2c[0x70 - 0x2C];
    s16 h70;            /* +0x70 */
} AcJump;

#define ABS(x) ((x) >= 0 ? (x) : -(x))

extern long SquareRoot0(long a);
extern void func_80076548(AcEntity *e, s32 sub, void *fn);
extern void func_8004E98C(AcMover *m, s32 a, s32 b, s32 c);
extern s32 func_8004E06C(SVECTOR *d, SVECTOR *out, s32 r, s32 h, s32 k);
extern s32 func_80074800(AcEntity *e, SVECTOR *p, SVECTOR *d, s32 z);
extern s32 func_800749A0(AcEntity *e, AcPos *pos, SVECTOR *tmp, SVECTOR *v);
extern void func_8004DFF8(AcMover *m, s32 n, SVECTOR *out);
extern void func_80080DA0(AcEntity *o);

void func_80081088(AcEntity *e, s32 a, s32 b, s32 c, SVECTOR *v)
{
    SVECTOR d;
    SVECTOR unused;
    SVECTOR out;
    SVECTOR tmp;
    SVECTOR p;
    s32 n;

    func_80076548(e, 0xB0, func_80080DA0);
    if (a != -1) {
        func_8004E98C(&e->mover, a, b, c);
    } else {
        ((AcJump *)e)->h70 = b;
    }
    d.vx = v->vx >> 6;
    d.vz = v->vz >> 6;
    d.vy = SquareRoot0(d.vx * d.vx + d.vz * d.vz) >> 1;
    n = func_8004E06C(&d, &out, ((AcJump *)e)->h28 - (((AcJump *)e)->h28 >> 2),
                      ((AcJump *)e)->h2a - (((AcJump *)e)->h2a >> 2), ((u8 *)&e->mover)[4]);
    d = out;
    p.vx = e->pos.x + out.vx;
    p.vy = e->pos.y + out.vy;
    p.vz = e->pos.z + out.vz;
    if (func_80074800(e, &p, &d, 0) != 0) {
        if (d.vy <= 0) {
            out.vy = 0;
        }
        d = out;
    }
    if (func_800749A0(e, &e->pos, &tmp, &out) != 0) {
        if (ABS(d.vx - out.vx) >= 7) {
            v->vx = (out.vx << 6) / n;
        }
        if (ABS(d.vz - out.vz) >= 7) {
            v->vz = (out.vz << 6) / n;
        }
        v->vy = (out.vy << 6) / n;
    }
    func_8004DFF8(&e->mover, n, &out);
}
