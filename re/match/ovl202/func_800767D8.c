#include "ac1/ac1.h"

typedef struct {
    u8 pad[0x10];
    u8 h10[8];          /* +0x10 */
    s16 vx, vy, vz;     /* +0x18 */
    u8 pad1e[0x1C];
    u8 kind;            /* +0x3A */
    u8 pad3b[2];
    u8 sub3d;           /* +0x3D */
    u8 pad3e[0x2C];
    u16 h6a;            /* +0x6A */
    u16 h6c;            /* +0x6C */
} AcLand;

typedef struct {
    SVECTOR lo;
    SVECTOR hi;
} Box16;

#define ABS(x) ((x) < 0 ? -(x) : (x))

extern SVECTOR D_80039C78;
extern long SquareRoot0(long a);
extern void func_80076548(AcEntity *e, s32 sub, void *fn);
extern void func_8004EA98(AcMover *m, s32 a, s32 b, s32 c);
extern void func_8004E98C(AcMover *m, s32 a, s32 b, s32 c);
extern void func_8004EA3C(AcMover *m, s32 a, s32 b);
extern s32 func_8004E22C(AcMover *m, void *a, void *b, SVECTOR *v, SVECTOR *o);
extern s32 func_800749A0(AcEntity *e, AcPos *pos, SVECTOR *tmp, SVECTOR *v);
extern void func_80014338(SVECTOR *v, s32 r, Box16 *box);
extern void *func_80071DC4(Box16 *box, s32 kind);
extern void func_8004DEAC(AcMover *m, s32 n, SVECTOR *v, SVECTOR *o);

#define l ((AcLand *)e)

typedef void (*MoveFn)(AcEntity *, s32, s32);

void func_800767D8(AcEntity *e, s32 a, s32 b, s32 mode, s32 c, s32 d, void *fn)
{
    SVECTOR v;
    SVECTOR w;
    SVECTOR o;
    SVECTOR tmp;
    Box16 box;
    s32 n;
    s32 r;
    s32 p;
    s32 q;

    if (l->sub3d != 16) {
        l->vz = 0;
        l->vy = 0;
        l->vx = 0;
    }
    func_80076548(e, 16, fn);
    if (mode != 0) {
        func_8004EA98(&e->mover, a, b, 0);
    } else {
        func_8004E98C(&e->mover, a, b, 0);
        func_8004EA3C(&e->mover, c, d);
    }
    n = func_8004E22C(&e->mover, l->h10, l->h10, &v, &o);
    w = v;
    v.vy += SquareRoot0(v.vx * v.vx + v.vz * v.vz) >> 1;
    r = func_800749A0(e, &e->pos, &tmp, &v);
    if ((u16)(v.vy + 2) < 5) {
        func_80014338(&v, l->h6a, &box);
        box.hi.vx = box.lo.vx += e->pos.x;
        box.lo.vy += e->pos.y;
        box.hi.vy = box.lo.vy + 64;
        box.lo.vy -= l->h6c;
        box.hi.vz = box.lo.vz += e->pos.z;
        if (func_80071DC4(&box, l->kind) != 0) {
            func_8004DEAC(&e->mover, n, &v, &o);
            p = v.vx;
            q = w.vx;
            if (p < 0) {
                p = -p;
            }
            if (q < 0) {
                q = -q;
            }
            p = p < q >> 1;
            if (p) {
                goto call;
            }
            p = v.vz;
            q = w.vz;
            if (p < 0) {
                p = -p;
            }
            if (q < 0) {
                q = -q;
            }
            p = p < q >> 1;
            if (!p) {
                return;
            }
            goto call;
        }
    }
    func_8004DEAC(&e->mover, n, &D_80039C78, &o);
call:
    if (e->fn_move != 0) {
        ((MoveFn)e->fn_move)(e, 5, r);
    }
}
