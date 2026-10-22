#include "ac1/ac1.h"

typedef struct {
    u8 pad0[4];
    u8 power;           /* +0x04 */
} Hit;

typedef struct {
    u8 pad0[0x36];
    u16 faction;        /* +0x36 */
    u8 pad38[0x3C - 0x38];
    u8 dir;             /* +0x3C */
    u8 kind;            /* +0x3D */
    u8 pad3e[2];
    u8 state;           /* +0x40 */
    u8 pad41[0x64 - 0x41];
    u8 mode;            /* +0x64 AcMover.mode */
    u8 pad65[0xCC - 0x65];
    AcPos vel;          /* +0xCC */
    u8 padd4[0x148 - 0xD4];
    u16 fire_flags;     /* +0x148 */
    u8 pad14a[0x15C - 0x14A];
    u32 flags;          /* +0x15C */
    u16 ap;             /* +0x160 */
} Obj;

typedef struct {
    u8 unk0[0x1A];
    u8 unk1A;           /* +0x1A */
    u8 unk1B[0x250 - 0x1B];
    s16 unk250;         /* +0x250 */
    u8 unk252[0x39C - 0x252];
} Player;

extern Player *D_80039D18;
extern Player D_80040FA0[2];
extern Obj D_801AB748;
extern s32 D_801AB684;
extern void func_80073FDC(Obj *o, Hit *h, s32 c, s32 fx, s32 n);
extern s32 func_8007435C(Obj *o, Hit *h, s32 a, s32 b, s32 n, AcPos *knock);
extern void func_80075D34(Obj *o, s32 a, s32 b, AcPos *vel, void *fn);
extern s32 func_80074314(Obj *o, s32 power);
extern void func_8007647C(Obj *o, s32 anim, s32 n, AcPos *vel, s32 t, void *fn);
extern void func_800764D0(Obj *o, s32 anim, s32 a, s32 n, AcPos *vel, s32 t, void *fn);
extern void func_80078688();
extern void func_80079D4C();
extern void func_8007A078();

/* take a hit (2-player twin of fdat000 0x801C91CC): selects the player record of
 * the hit AC, then knock-back / stagger / fall */
void func_8007C3F0(Obj *o, Hit *h, s32 a, s32 b, s32 c)
{
    Player *save;
    AcPos knock;
    s32 q, anim;

    save = D_80039D18;
    if (o == &D_801AB748) {
        D_80039D18 = &D_80040FA0[0];
    } else {
        D_80039D18 = &D_80040FA0[1];
    }
    if (D_801AB684 & 0x400) {
        if (o->ap != 0) {
            o->ap += c;
        }
    } else {
        D_80039D18->unk250 = 40;
        if (o->ap == 0 && o->state != 254) {
            func_80073FDC(o, h, c, 40, 256);
        }
    }
    if (h->power != 0) {
        o->fire_flags = 0;
        o->flags = 0;
        q = func_8007435C(o, h, a, b, 3276, &knock);
        knock.x <<= 6;
        knock.y <<= 6;
        knock.z <<= 6;
        if (o->dir == 34) {
            o->vel = knock;
            func_80075D34(o, D_80039D18->unk1A, 1, &o->vel, func_80078688);
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
                func_8007647C(o, anim, 1024, &o->vel, func_80074314(o, h->power), func_80079D4C);
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
                func_800764D0(o, anim, 1, 1024, &knock, func_80074314(o, h->power), func_8007A078);
            }
        }
    }
    D_80039D18 = save;
}
