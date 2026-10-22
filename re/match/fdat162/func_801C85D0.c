#include "ac1/ac1.h"

typedef struct {
    u8 pad0[0x30];
    u32 flags;          /* +0x30 */
    u8 pad34[0xC];
} World;
typedef struct {
    u8 pad0[0x10];
    u8 f10;
    u8 f11;
    u8 f12;
} Cnt;
typedef struct {
    u8 pad0[6];
    u8 f6;
} Def;
typedef struct {
    u8 pad0[0x28];
    u16 *ids;           /* +0x28 */
    Cnt *cnt;           /* +0x2C */
    u8 pad30[8];
    Def *def;           /* +0x38 */
} Obj;
extern World D_801D4488[];
extern void func_801C8580();

s32 func_801C85D0(Obj *o, s32 msg, s32 **arg, ...)
{

    switch (msg) {
    case 0x8100:
        if (o->cnt->f10 >= 4) {
            return 0;
        }
        if (((*arg)[3] & 7) != 2) {
            return 0;
        }
        msg = 1;
        if (o->def->f6 != 0) {
            return 1;
        }
        goto blk;
    case 0x8200:
        msg = 1;
    blk:
        if (o->cnt->f11 < o->cnt->f12) {
            World *w = &D_801D4488[*o->ids];
            w->flags &= ~0x100;
            func_801C8580(o, msg);
            return 1;
        }
        return 1;
    case 0x8201:
        msg = 4;
        goto blk;
    }
    return 0;
}
