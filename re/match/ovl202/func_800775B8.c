#include "ac1/ac1.h"

typedef struct {
    u8 pad[0x28];
    u16 h28;            /* +0x28 */
    u16 h2a;            /* +0x2A */
    u8 pad2c[0x0A];
    u16 faction;        /* +0x36 */
    u8 pad38[4];
    u8 sub3c;           /* +0x3C */
    u8 pad3d[0x33];
    s16 h70;            /* +0x70 */
} AcFall;

#define F(e) ((AcFall *)(e))
#define ABS(x) ((x) >= 0 ? (x) : -(x))

typedef void (*MoveFn)(AcEntity *, s32);

extern s32 D_801A5DC8;
extern void func_80076548(AcEntity *e, s32 sub, void *fn);
extern void func_8004E98C(AcMover *m, s32 a, s32 b, s32 c);
extern s32 func_8004E06C(SVECTOR *d, SVECTOR *out, s32 r, s32 h, s32 k);
extern s32 func_80074CA0(AcEntity *e, AcPos *pos, SVECTOR *tmp, SVECTOR *v);
extern s32 func_80074FA4(AcEntity *e, SVECTOR *p);
extern void func_8004DFF8(AcMover *m, s32 n, SVECTOR *out);

void func_800775B8(AcEntity *e, s32 a, s32 b, s32 c, SVECTOR *v, void *fn)
{
    SVECTOR d;
    SVECTOR out;
    SVECTOR tmp;
    SVECTOR p;
    s32 n;
    s32 x;
    s32 y;

    func_80076548(e, 0xC0, fn);
    if (a != -1) {
        func_8004E98C(&e->mover, a, b, c);
    } else {
        F(e)->h70 = b;
    }
    if (v->vx == 0 && v->vz == 0 && (u16)v->vy <= 3200) {
        D_801A5DC8 = 1;
    }
    d.vx = v->vx >> 6;
    d.vy = v->vy >> 6;
    d.vz = v->vz >> 6;
    n = func_8004E06C(&d, &out, F(e)->h28 - (F(e)->h28 >> 2), F(e)->h2a - (F(e)->h2a >> 2), ((u8 *)&e->mover)[4]);
    d = out;
    if (func_80074CA0(e, &e->pos, &tmp, &out) != 0) {
        if (ABS(out.vy) < (ABS(v->vy) >> 7) + (ABS(v->vy) >> 8)) {
            p.vx = e->pos.x;
            p.vz = e->pos.z;
            p.vy = e->pos.y + 50;
            if (func_80074FA4(e, &p) != 0) {
                if (v->vy < 4481) {
                    F(e)->sub3c = 96;
                } else {
                    F(e)->sub3c = 97;
                }
                F(e)->faction |= 0x10;
                if (e->fn_move != 0) {
                    ((MoveFn)e->fn_move)(e, 4);
                }
            }
        }
        if (ABS(d.vx - out.vx) >= 7) {
            v->vx = (out.vx << 6) / n;
        }
        if (ABS(d.vz - out.vz) >= 7) {
            v->vz = (out.vz << 6) / n;
        }
        if (d.vy < 0 && d.vy < out.vy) {
            v->vy = (out.vy << 6) / n;
        }
    }
    D_801A5DC8 = 0;
    func_8004DFF8(&e->mover, n, &out);
}
