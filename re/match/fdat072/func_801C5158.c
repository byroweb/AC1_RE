#include "ac1/ac1.h"

typedef struct {
    u8 pad0[0x1C];
    u16 f1c;
    u16 f1e;
    u8 f20;
    u8 f21;
} Part;
typedef struct {
    Part *part;         /* +0x00 */
    u8 pad4[0x3A];
    u8 f3e;
    u8 f3f;
    u8 f40;
    u8 pad41[7];
    s32 target;         /* +0x48 */
    u8 pad4c[0xFC];
    u16 f148;
    u8 f14a;
    u8 f14b;
    u8 f14c;
    u8 pad14d[0xF];
    s32 f15c;
} Ent;
extern s32 func_80073714();
extern s32 func_800754D0();
extern void func_80077B78();
extern void func_80078E1C();
extern s32 func_801C4F3C();
extern s32 func_801C5050();
extern s32 rand();
extern u8 D_801C6A5C[];
extern u8 D_801C6A80[];
extern u8 D_801C6AA0[];

void func_801C5158(Ent *e)
{
    s32 d;
    s32 r;
    s32 b;
    s32 a;
    Part *p;

    a = 0;
    b = 0;
    e->f148 = 0;
    e->f15c = 0;
    if (e->f40 != 0xFF) {
        if (func_800754D0(e, D_801C6A5C) != 0) {
            func_80078E1C(e);
        }
        return;
    }
    d = func_80073714(e, 0x7FFFFFFF);
    if (d != -1) {
        if (d > 12000) {
            d = 15000;
            r = 1600;
            if (!(rand() & 0x7F)) {
                b = 1;
            }
        } else {
            d = 12000;
            r = 1600;
            if (rand() & 7) {
                a = 1;
            } else {
                b = 1;
            }
        }
        if (func_801C4F3C(e, d, r) != 0 || func_801C5050(e, d, 2000) != 0) {
            if (e->f3f != 0) {
                goto dec;
            }
            if (a != 0) {
                p = e->part;
                func_80077B78(e, 23, p->f1c, p->f20, D_801C6A80, 1, 0, 1);
                e->f3f = (rand() & 7) + 30;
            }
            if (b != 0) {
                p = e->part;
                func_80077B78(e, 23, p->f1e, p->f21, D_801C6AA0, 2, 0, 1, 0, -128, e->target, 2, 3, 0, 128, e->target);
                e->f3f = (rand() & 0x1F) + 20;
            }
            e->f148 = 15;
            e->f14a = 180;
            e->f14b = 160;
            e->f14c = 140;
            e->f15c = 2;
        }
    }
    if (e->f3f != 0) {
dec:
        e->f3f--;
    }
}
