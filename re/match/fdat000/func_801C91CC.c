/* fdat000 (mission library, entry 0 @ 0x801C4B40; 31 twins, group g0014): take a
 * hit.  With no armour left, start the death effect first.  For a real hit, clear the
 * AI flags, get the knock-back (func_80075798, scaled by 64) and the hit quadrant;
 * direction 34 just flies off (func_80077170 with think routine 801C6A28), kind 192 or
 * an upward knock (with def+0x16 set) staggers into the hover routine 801C5CB8 with
 * animation 41..44, anything else falls with animation 37..40 (func_8007790C). */
#include "ac1/ac1.h"
typedef struct {
    u8 pad0[4];
    u8 power;           /* +0x04 */
} Func801C91CCHit;
typedef struct {
    u8 pad0[0x16];
    u16 sink;           /* +0x16 */
    u8 pad18[0x4B - 0x18];
    u8 death_fx;        /* +0x4B */
} Func801C91CCDef;
typedef struct {
    u8 pad0[0x36];
    u16 faction;        /* +0x36 */
    u8 pad38[0x3C - 0x38];
    u8 dir;             /* +0x3C */
    u8 kind;            /* +0x3D */
    u8 pad3e[0x64 - 0x3E];
    u8 mode;            /* +0x64 AcMover.mode */
    u8 pad65[0xCC - 0x65];
    AcPos vel;          /* +0xCC */
    u8 padd4[0x108 - 0xD4];
    Func801C91CCDef *def; /* +0x108 */
    u8 pad10c[0x135 - 0x10C];
    u8 b135;            /* +0x135 */
    u8 pad136[0x148 - 0x136];
    u16 fire_flags;     /* +0x148 */
    u8 pad14a[0x15C - 0x14A];
    u32 flags;          /* +0x15C */
    u16 ap;             /* +0x160 */
} Func801C91CCObj;
extern void func_80075418(Func801C91CCObj *o, Func801C91CCHit *h, s32 c, s32 fx, s32 n);
extern s32 func_80075798(Func801C91CCObj *o, Func801C91CCHit *h, s32 a, s32 b, s32 n, AcPos *knock);
extern void func_80077170(Func801C91CCObj *o, s32 a, s32 b, AcPos *vel, void *fn);
extern s32 func_80075750(Func801C91CCObj *o, s32 power);
extern void func_800778B8(Func801C91CCObj *o, s32 anim, s32 n, AcPos *vel, s32 t, void *fn);
extern void func_8007790C(Func801C91CCObj *o, s32 anim, s32 a, s32 n, AcPos *vel, s32 t, void *fn);
extern void func_801C6A28();
extern void func_801C5CB8();
extern void func_800764C4();
void func_801C91CC(Func801C91CCObj *o, Func801C91CCHit *h, s32 a, s32 b, s32 c)
{
    Func801C91CCDef *d = o->def;
    AcPos knock;
    u8 fx[2];
    s32 q, anim;            /* q: hit angle, then its quadrant */

    if (o->ap == 0) {
        fx[0] = 60;
        fx[1] = 120;
        func_80075418(o, h, c, fx[d->death_fx], 256);
    }
    if (h->power == 0)
        return;
    o->flags = 0;
    o->fire_flags = 0;
    q = func_80075798(o, h, a, b, 3276, &knock);
    knock.x <<= 6;
    knock.y <<= 6;
    knock.z <<= 6;
    if (o->dir == 34) {
        o->vel = knock;
        func_80077170(o, o->b135, 1, &o->vel, func_801C6A28);
        return;
    }
    q = ((q - 512) & 0xFFF) >> 10;
    if ((o->kind == 192 || knock.y < -29) && d->sink != 0) {
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
        func_800778B8(o, anim, 1024, &o->vel, func_80075750(o, h->power), func_801C5CB8);
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
        func_8007790C(o, anim, 1, 1024, &knock, func_80075750(o, h->power), func_800764C4);
    }
}
