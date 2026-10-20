/* fdat000 (mission library, entry 0 @ 0x801C4B40; 29 twins): animation step for a
 * unit by its current animation (+0x64): the "start" animations 2/4/6, 15/17/19, 9 and
 * 12 switch to 7, 20, 10 and 13 (func_80076E20 with the 0x8007618C callback) and reload
 * the wait time from the def; those four wait for the animation to finish
 * (func_8004EBF8) and then restart.  Always returns 1. */
#include "ac1/ac1.h"
typedef struct {
    u8 pad0[0x22];
    u16 wait;           /* +0x22 */
} Func801C96F4Def;
typedef struct {
    u8 pad0[0x64];
    u8 anim;            /* +0x64 mover */
    u8 pad65[0x72 - 0x65];
    u16 wait;           /* +0x72 */
    u8 pad74[0x108 - 0x74];
    Func801C96F4Def *def; /* +0x108 */
} Func801C96F4Obj;
extern void func_8007618C();
extern void func_80076E20(Func801C96F4Obj *o, s32 anim, s32 a, void (*cb)());
extern s32 func_8004EBF8(u8 *mover);
s32 func_801C96F4(Func801C96F4Obj *o)
{
    Func801C96F4Def *d = o->def;

    switch (o->anim) {
    case 2:
    case 4:
    case 6:
        func_80076E20(o, 7, 1, func_8007618C);
        o->wait = d->wait;
        return 1;
    case 7:
        if (!func_8004EBF8(&o->anim))
            func_80076E20(o, 7, 1, func_8007618C);
        break;
    case 15:
    case 17:
    case 19:
        func_80076E20(o, 20, 1, func_8007618C);
        o->wait = d->wait;
        return 1;
    case 20:
        if (!func_8004EBF8(&o->anim))
            func_80076E20(o, 20, 1, func_8007618C);
        break;
    case 9:
        func_80076E20(o, 10, 1, func_8007618C);
        o->wait = d->wait;
        return 1;
    case 10:
        if (!func_8004EBF8(&o->anim))
            func_80076E20(o, 10, 1, func_8007618C);
        break;
    case 12:
        func_80076E20(o, 13, 1, func_8007618C);
        o->wait = d->wait;
        return 1;
    case 13:
        if (!func_8004EBF8(&o->anim))
            func_80076E20(o, 13, 1, func_8007618C);
        break;
    }
    return 1;
}
