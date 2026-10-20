/* fdat000 (mission library, entry 0 @ 0x801C4B40; 29 twins): per-frame update of a
 * unit with a vertical attack window.  While its height (+0x2E) lies between y - range
 * (+0x2A) and y, fire the attack for the current animation: mode 22 / frame 6 a fixed
 * burst (func_8007847C), frame 12 or 5 the def's two shots (func_80078250).  Then
 * run func_80075FC0, step the mover, and call the +0x50 hook when it stops. */
#include "ac1/ac1.h"
typedef struct {
    u8 pad0[0x30];
    u8 shot_a[2];       /* +0x30 */
    u8 shot_b[2];       /* +0x32 */
} Func801C95B8Def;
typedef struct Func801C95B8Obj Func801C95B8Obj;
struct Func801C95B8Obj {
    u8 pad0[8];
    AcPos pos;          /* +0x08 */
    u8 pad10[0x2A - 0x10];
    u16 range;          /* +0x2A */
    u8 pad2c[2];
    s16 height;         /* +0x2E */
    u8 pad30[0x41 - 0x30];
    u8 b41;             /* +0x41 */
    u8 pad42[0x50 - 0x42];
    void (*fn)(Func801C95B8Obj *, s32); /* +0x50 */
    u8 pad54[0x64 - 0x54];
    u8 mode;            /* +0x64 mover */
    u8 pad65[0x6E - 0x65];
    s16 frame;          /* +0x6E */
    u8 pad70[0x108 - 0x70];
    Func801C95B8Def *def; /* +0x108 */
};
extern void func_8007847C(Func801C95B8Obj *o, s32 a, s32 b, s32 c, s32 d, s32 e, s32 f, s32 g);
extern void func_80078250(Func801C95B8Obj *o, s32 a, s32 b, s32 c, s32 d, s32 e, s32 f, s32 g);
extern void func_80075FC0(Func801C95B8Obj *o);
extern s32 func_8004F024(u8 *mover, AcPos *pos);
void func_801C95B8(Func801C95B8Obj *o)
{
    Func801C95B8Def *d;

    if (o->height <= o->pos.y && o->height >= o->pos.y - o->range) {
        if (o->mode == 22) {
            if (o->frame == 6)
                func_8007847C(o, 100, 5, 300, 700, 100, 300, 64);
        } else {
            d = o->def;
            if (o->frame == 12)
                func_80078250(o, d->shot_a[0], d->shot_a[1], 0, 80, 200, 65, 0);
            else if (o->frame == 5)
                func_80078250(o, d->shot_b[0], d->shot_b[1], 0, 80, 200, 65, 0);
        }
    }
    func_80075FC0(o);
    if (!func_8004F024(&o->mode, &o->pos)) {
        if (o->fn != 0)
            o->fn(o, o->b41);
    }
}
