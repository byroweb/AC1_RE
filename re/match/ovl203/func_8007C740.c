/* ovl203 take a hit (cf. fdat000 0x801C91CC), 2-player version: D_80039D18 is
 * switched to the record of the side that owns o for the duration. */
#include "ac1/ac1.h"
typedef struct {
    u8 pad0[4];
    u8 power;           /* +0x04 */
} HitInfo3;
typedef struct {
    u8 pad0[0x36];
    u16 faction;        /* +0x36 */
    u8 pad38[0x3C - 0x38];
    u8 dir;             /* +0x3C */
    u8 kind;            /* +0x3D */
    u8 pad3e[2];
    u8 b40;             /* +0x40 */
    u8 pad41[0x64 - 0x41];
    u8 mode;            /* +0x64 AcMover.mode */
    u8 pad65[0xCC - 0x65];
    AcPos vel;          /* +0xCC */
    u8 padd4[0x148 - 0xD4];
    u16 fire_flags;     /* +0x148 */
    u8 pad14a[0x15C - 0x14A];
    u32 flags;          /* +0x15C */
    u16 ap;             /* +0x160 */
} HitObj3;
typedef struct {
    u8 pad0[0x1A];
    u8 b1A;             /* +0x1A */
    u8 pad1b[0x250 - 0x1B];
    s16 hit_timer;      /* +0x250 */
    u8 pad252[0x394 - 0x252];
    HitObj3 *ent;       /* +0x394 */
} Side3;
extern Side3 *D_80039D18;
extern Side3 *D_80039D30;
extern Side3 *D_80039D34;
extern u32 D_8019B48C;
extern void func_80074198(HitObj3 *o, HitInfo3 *h, s32 c, s32 fx, s32 n);
extern s32 func_80074518(HitObj3 *o, HitInfo3 *h, s32 a, s32 b, s32 n, AcPos *knock);
extern void func_80075EF0(HitObj3 *o, s32 a, s32 b, AcPos *vel, void *fn);
extern s32 func_800744D0(HitObj3 *o, s32 power);
extern void func_80076638(HitObj3 *o, s32 anim, s32 n, AcPos *vel, s32 t, void *fn);
extern void func_8007668C(HitObj3 *o, s32 anim, s32 a, s32 n, AcPos *vel, s32 t, void *fn);
extern void func_800789D8();
extern void func_8007A09C();
extern void func_8007A3C8();
void func_8007C740(HitObj3 *o, HitInfo3 *h, s32 a, s32 b, s32 c)
{
    Side3 *save;
    AcPos knock;
    s32 q, anim;

    save = D_80039D18;
    if (o == D_80039D30->ent) {
        D_80039D18 = D_80039D30;
    } else {
        D_80039D18 = D_80039D34;
    }
    if (D_8019B48C & 0x400) {
        if (o->ap != 0) {
            o->ap += c;
        }
    } else {
        D_80039D18->hit_timer = 40;
        if (o->ap == 0 && o->b40 != 254) {
            func_80074198(o, h, c, 40, 256);
        }
    }
    if (h->power != 0) {
        o->fire_flags = 0;
        o->flags = 0;
        q = func_80074518(o, h, a, b, 3276, &knock);
        knock.x <<= 6;
        knock.y <<= 6;
        knock.z <<= 6;
        if (o->dir == 34) {
            o->vel = knock;
            func_80075EF0(o, D_80039D18->b1A, 1, &o->vel, func_800789D8);
        } else {
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
                func_80076638(o, anim, 1024, &o->vel, func_800744D0(o, h->power), func_8007A09C);
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
                func_8007668C(o, anim, 1, 1024, &knock, func_800744D0(o, h->power), func_8007A3C8);
            }
        }
    }
    D_80039D18 = save;
}
