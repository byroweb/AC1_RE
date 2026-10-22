#include "ac1/ac1.h"

typedef struct {
    u8 pad0[0x1C];
    u16 f1c;
    u16 f1e;
    u8 f20;
    u8 f21;
} Part;
typedef struct {
    u8 pad0[0x38];
    u16 f38;
} Tgt;
typedef struct {
    Part *part;         /* +0x00 */
    u8 pad4[0x3A];
    u8 f3e;
    u8 f3f;
    u8 f40;
    u8 pad41[7];
    Tgt *target;        /* +0x48 */
    u8 pad4c[0x30];
    s16 f7c;
    u8 pad7e[0xCA];
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
extern s32 func_801C5B10();
extern s32 rand();
extern u8 D_801C74CC[];
extern u8 D_801C74F0[];
extern u8 D_801C7510[];

void func_801C5C00(Ent *e)
{
    Part *p;

    e->f148 = 0;
    e->f15c = 0;
    if (e->f40 != 0xFF) {
        if (func_800754D0(e, D_801C74CC) != 0) {
            func_80078E1C(e);
        }
    } else if (func_80073714(e, 0x7FFFFFFF) != -1 && e->f7c == 0 && func_801C5B10(e) != 0) {
        if (rand() < 0x148 && (!(e->target->f38 & 8) || rand() >= 0x4000)) {
            p = e->part;
            func_80077B78(e, 0x17, p->f1e, p->f21, D_801C74F0, 1, 0, 1, 0, 0x100, e->target);
            p = e->part;
            func_80077B78(e, 0x17, p->f1e, p->f21, D_801C74F0, 1, 2, 3, 0, -256, e->target);
            e->f148 = 0xF0;
            e->f14a = 0xB4;
            e->f14b = 0xA0;
            e->f14c = 0x8C;
            e->f15c = 1;
        }
        if (e->f3e == 0) {
            if (rand() < 0x290) {
                e->f3e = 8;
                goto blk13;
            }
        } else {
        blk13:
            if (!(e->f3e & 1)) {
                p = e->part;
                func_80077B78(e, 0x17, p->f1c, p->f20, D_801C7510, 1, 0, 1);
                e->f148 = 0xF;
                e->f14a = 0xB4;
                e->f14b = 0xA0;
                e->f14c = 0x8C;
                e->f15c = 2;
            }
            e->f3e--;
        }
    }
}
