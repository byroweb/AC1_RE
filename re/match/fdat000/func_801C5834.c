/* fdat000 (mission library, entry 0 @ 0x801C4B40; 31 twins, group g0023): install the
 * think routine func_801C59EC, start a move (or just set +0x70 when a == -1), then
 * aim along the target vector t (X/Z scaled by 1/64, Y = half the horizontal length),
 * let func_8004E06C turn that into a step, clip it against the world with
 * func_80074800 and hand the result to func_800749A0 / func_8004DFF8. */
#include "ac1/ac1.h"
typedef struct {
    u16 x;
    u16 y;
    u16 z;
} Func801C5834Vec;
typedef struct {
    u8 pad0[0x08];
    AcPos pos;          /* +0x08 */
    u8 pad10[0x64 - 0x10];
    u8 mover[4];        /* +0x64 AcMover */
    u8 b68;             /* +0x68 */
    u8 pad69;
    u16 h6a;            /* +0x6A */
    u16 h6c;            /* +0x6C */
    u8 pad6e[2];
    u16 h70;            /* +0x70 */
} Func801C5834Obj;
extern void func_801C59EC();
extern void func_80076548(Func801C5834Obj *o, s32 n, void *fn);
extern void func_8004E98C(void *mover, s32 a, s32 b, s32 c);
extern s32 func_80027324(s32 v);
extern s32 func_8004E06C(AcPos *dir, AcPos *step, s32 a, s32 b, s32 c);
extern s32 func_80074800(Func801C5834Obj *o, AcPos *to, AcPos *step, s32 flags);
extern void func_800749A0(Func801C5834Obj *o, AcPos *pos, AcPos *out, AcPos *step);
extern void func_8004DFF8(void *mover, s32 r, AcPos *step);
void func_801C5834(Func801C5834Obj *o, s32 a, s32 b, s32 c, Func801C5834Vec *t)
{
    AcPos v;
    AcPos unused;       /* sp+32: never touched, but the retail frame has the slot */
    AcPos tmp;
    AcPos buf;
    AcPos p;
    s32 x, z, r;

    func_80076548(o, 16, func_801C59EC);
    if (a != -1)
        func_8004E98C(o->mover, a, b, c);
    else
        o->h70 = b;
    x = (s16)t->x >> 6;
    v.x = x;
    z = (s16)t->z >> 6;
    v.z = z;
    v.y = func_80027324(x * x + z * z) >> 1;
    r = func_8004E06C(&v, &tmp, o->h6a, o->h6c, o->b68);
    v = tmp;
    p.x = o->pos.x + tmp.x;
    p.y = o->pos.y + tmp.y;
    p.z = o->pos.z + tmp.z;
    if (func_80074800(o, &p, &v, 0)) {
        if (v.y <= 0)
            tmp.y = 0;
        v = tmp;
    }
    func_800749A0(o, &o->pos, &buf, &tmp);
    func_8004DFF8(o->mover, r, &tmp);
}
