#include "ac1/ac1.h"

typedef struct {
    u8 pad0[0xC];
    u16 timer;          /* +0x0C */
    u8 f0e;
    u8 f0f;
    u8 f10;
    u8 f11;
    u8 f12;
    u8 f13;
    u16 f14;
} Def;
typedef struct {
    u8 pad0[0x10];
    s16 pos[3];         /* +0x10 */
    s16 pad16;
    s16 a[3];           /* +0x18 */
    s16 pad1e;
    s16 b[3];          /* +0x20 */
    u8 pad26[0x12];
    Def *def;           /* +0x38 */
} Obj;
typedef struct {
    u8 pad0[2];
    u16 dt;             /* +2 */
} Tick;
extern void func_8004CE44();
extern void func_8006D39C();
extern void func_800869F0();
extern void func_800873F8();
extern void func_80087798();
typedef struct { u32 lo:17; u32 f:10; u32 hi:5; } BF;
extern BF D_801CDAF4;
extern BF D_801CDAF8;

s32 func_801C605C(Obj *o, s32 msg, s32 a2, ...)
{
    s16 sp18[3];
    struct {
        s8 ty;
        u8 pad1;
        u16 w;
        u8 c;
    } sp20;
    Def *d;
    s32 t;
    s32 w;
    u32 x;
    u8 n;
    BF *p4;
    BF *p8;

    if (msg == 0x8000) {
        d = o->def;
        t = d->timer;
        if (t != 0xFFFF) {
            t -= (((Tick **)&a2)[1])->dt;
            if (t <= 0) {
                sp18[0] = (o->a[0] + o->b[0]) >> 1;
                sp18[1] = (o->a[1] + o->b[1]) >> 1;
                sp18[2] = (o->a[2] + o->b[2]) >> 1;
                d->timer = 0xFFFF;
                if (d->f10 != 0) {
                    func_8004CE44(d->f10, o->pos, d->f11, 0);
                }
                n = d->f0f;
                if (n != 0) {
                    p8 = &D_801CDAF8;
                    p4 = &D_801CDAF4;
                    p8->f = n * 100;
                    p4->f = n * 100;
                    func_800869F0(0, sp18, p4->f & 0x3FC, d->f0f * 2 - 1, p4);
                }
                if (d->f0e != 0) {
                    func_800873F8(o, o->pos, 0x1E, 0x14, d->f0e, 0);
                }
                if (d->f12 != 0) {
                    sp20.ty = 0x17;
                    sp20.w = d->f14;
                    sp20.c = d->f13;
                    func_80087798(0, sp18, &sp20, d->f12, 1);
                }
                func_8006D39C(o);
            } else {
                d->timer = t;
            }
        }
    }
    return 0;
}
