#include "ac1/ac1.h"
/* ovl202 0x8007DB98: mission actor takes a hit (port of fdat000 0x801C91CC): during the end
 * sequence (flag 0x400) only adjust AP; else alarm 40 and the death effect at AP 0; then
 * knock-back: direction 34 flies off, kind 192 / upward knock staggers (anim 41..44),
 * anything else falls (anim 37..40). */
typedef struct {
    u8 pad0[4];
    u8 power;           /* +0x04 */
} Func8007DB98Hit;
typedef struct {
    u8 pad0[0x36];
    u16 faction;        /* +0x36 */
    u8 pad38[0x3C - 0x38];
    u8 dir;             /* +0x3C */
    u8 kind;            /* +0x3D */
    u8 pad3e[0x40 - 0x3E];
    u8 ai_mode;         /* +0x40 */
    u8 pad41[0x64 - 0x41];
    u8 mode;            /* +0x64 AcMover.mode */
    u8 pad65[0xCC - 0x65];
    AcPos vel;          /* +0xCC */
    u8 padd4[0x148 - 0xD4];
    u16 fire_flags;     /* +0x148 */
    u8 pad14a[0x15C - 0x14A];
    u32 flags;          /* +0x15C */
    u16 ap;             /* +0x160 */
} Func8007DB98Obj;
extern u8 D_80040FBA;
extern s16 D_800411F0[];
extern void func_80075418(Func8007DB98Obj *o, Func8007DB98Hit *h, s32 c, s32 fx, s32 n);
extern s32 func_80075798(Func8007DB98Obj *o, Func8007DB98Hit *h, s32 a, s32 b, s32 n, AcPos *knock);
extern void func_80077170(Func8007DB98Obj *o, s32 a, s32 b, AcPos *vel, void *fn);
extern s32 func_80075750(Func8007DB98Obj *o, s32 power);
extern void func_800778B8(Func8007DB98Obj *o, s32 anim, s32 n, AcPos *vel, s32 t, void *fn);
extern void func_8007790C(Func8007DB98Obj *o, s32 anim, s32 a, s32 n, AcPos *vel, s32 t, void *fn);
extern void func_80079EF4();
extern void func_8007B548();
extern void func_8007B868();
void func_8007DB98(Func8007DB98Obj *o, Func8007DB98Hit *h, s32 a, s32 b, s32 c)
{
    AcPos knock;
    s32 q, anim;

    if (g_mission_flags & 0x400) {
        if (o->ap != 0) {
            o->ap += c;
        }
    } else {
        D_800411F0[0] = 40;
        if (o->ap == 0 && o->ai_mode != 0xFE) {
            func_80075418(o, h, c, 40, 256);
        }
    }
    if (h->power == 0)
        return;
    o->fire_flags = 0;
    o->flags = 0;
    q = func_80075798(o, h, a, b, 3276, &knock);
    knock.x <<= 6;
    knock.y <<= 6;
    knock.z <<= 6;
    if (o->dir == 34) {
        o->vel = knock;
        func_80077170(o, D_80040FBA, 1, &o->vel, func_80079EF4);
        return;
    }
    q = ((q - 512) & 0xFFF) >> 10;
    if (o->kind == 192 || knock.y < -29) {
        switch (q) {
        case 0:
            anim = 41;
            break;
        case 1:
            anim = 43;
            break;
        case 2:
            anim = 42;
            break;
        case 3:
            anim = 44;
            break;
        }
        o->dir = 129;
        o->faction |= 0x10;
        o->vel.x += knock.x;
        o->vel.y += knock.y >> 1;
        o->vel.z += knock.z;
        func_800778B8(o, anim, 1024, &o->vel, func_80075750(o, h->power), func_8007B548);
    } else {
        switch (q) {
        case 0:
            anim = 37;
            break;
        case 1:
            anim = 39;
            break;
        case 2:
            anim = 38;
            break;
        case 3:
            anim = 40;
            break;
        }
        o->dir = 128;
        o->mode = 255;
        o->faction |= 0x10;
        knock.x = o->vel.x + knock.x;
        knock.y = o->vel.y + knock.y;
        knock.z = o->vel.z + knock.z;
        func_8007790C(o, anim, 1, 1024, &knock, func_80075750(o, h->power), func_8007B868);
    }
}
