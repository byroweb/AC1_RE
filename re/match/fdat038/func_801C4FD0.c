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
extern s32 func_800754D0();
extern void func_80077B78();
extern s32 func_800781E8();
extern void func_80078E1C();
extern s32 func_801C4EDC();
extern s32 rand();
extern u8 D_801C6438[];
extern u8 D_801C645C[];
extern u8 D_801C647C[];

void func_801C4FD0(Ent *e)
{
    Part *p;

    e->f148 = 0;
    e->f15c = 0;
    if (e->f40 != 0xFF) {
        if (func_800754D0(e, D_801C6438) != 0) {
            func_80078E1C(e);
        }
    } else {
        if (func_801C4EDC(e) != 0) {
            if (e->f3f == 0) {
                if (func_800781E8(e) != 0 && !(rand() & 7)) {
                    if (e->f3e != 0 || !(rand() & 7)) {
                        p = e->part;
                        func_80077B78(e, 0x17, p->f1c, p->f20, D_801C645C, 2, 0, 1, 0, -300, e->target, 2, 3, 0, 300, e->target);
                        e->f3f = (rand() & 0xF) + 0x3C;
                        e->f3e = 0;
                    } else {
                        p = e->part;
                        func_80077B78(e, 0x17, p->f1e, p->f21, D_801C647C, 1, 0, 1);
                        e->f3f = (rand() & 0xF) + 0x14;
                    }
                    e->f148 = 0xF;
                    e->f14a = 0xC8;
                    e->f14b = 0xA0;
                    e->f14c = 0x8C;
                    e->f15c = 1;
                }
                goto blk12;
            }
            goto blk13;
        }
    blk12:
        if (e->f3f != 0) {
        blk13:
            e->f3f--;
        }
    }
}
