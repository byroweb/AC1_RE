#include "ac1/ac1.h"

typedef struct {
    u16 x;
    u16 y;
    u16 z;
} Vec3u;

typedef struct Obj Obj;
struct Obj {
    u8 pad0[0x08];
    AcPos pos;          /* +0x08 */
    u8 pad10[0x2A - 0x10];
    u16 h2a;            /* +0x2A */
    s16 h2c;            /* +0x2C */
    u8 pad2e[0x54 - 0x2E];
    void (*fn_move)(Obj *, s32);   /* +0x54 */
    u8 pad58[0x64 - 0x58];
    u8 mover[4];        /* +0x64 */
    u8 b68;             /* +0x68 */
    u8 pad69;
    u16 h6a;            /* +0x6A */
    u16 h6c;            /* +0x6C */
    u8 pad6e[2];
    u16 h70;            /* +0x70 */
};

extern int abs(int);
extern void func_80076548(Obj *, s32, void *);
extern void func_8004E98C(void *, s32, s32, s32);
extern s32 func_8004E06C(AcPos *, AcPos *, s32, s32, s32);
extern s32 func_80074CA0(Obj *, AcPos *, AcPos *, AcPos *);
extern void func_8004DFF8(void *, s32, AcPos *);

void func_801C5384(Obj *o, s32 a, s32 b, s32 c, Vec3u *t, void *fn)
{
    AcPos v;
    AcPos step;
    AcPos buf;
    s32 r;
    s32 ha, hc;

    func_80076548(o, 192, fn);
    if (a != -1) {
        func_8004E98C(o->mover, a, b, c);
    } else {
        o->h70 = b;
    }
    ha = o->h6a;
    hc = o->h6c;
    v.x = (s16)t->x >> 6;
    v.y = (s16)t->y >> 6;
    v.z = (s16)t->z >> 6;
    r = func_8004E06C(&v, &step, ha, hc, o->b68);
    v = step;
    if (func_80074CA0(o, &o->pos, &buf, &step)) {
        if (o->fn_move) {
            o->fn_move(o, 4);
        }
        if ((v.x - step.x >= 0 ? v.x - step.x : -(v.x - step.x)) >= 7) {
            t->x = (step.x << 6) / r;
        }
        if ((v.z - step.z >= 0 ? v.z - step.z : -(v.z - step.z)) >= 7) {
            t->z = (step.z << 6) / r;
        }
        if (abs(step.y) >= 33 || o->h2c >= o->pos.y + (o->h2a >> 1)) {
            t->y = (step.y << 6) / r;
        }
    }
    func_8004DFF8(o->mover, r, &step);
}
