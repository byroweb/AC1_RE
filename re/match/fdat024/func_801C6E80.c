#include "ac1/ac1.h"

typedef struct {
    u8 pad0[0x10];
    u8 pad10[0x20];
    u32 flags;          /* +0x30 */
    u8 pad34[0xC];
} Obj;
extern void func_8004CE44();
extern void func_800529F0();
extern void func_8008C2FC();
extern void func_801C6D94();
extern u16 D_8019F528;
extern s32 D_801C70EC;
extern Obj D_801D3368, D_801D33A8, D_801D33E8, D_801D3428, D_801D3468;

s32 func_801C6E80(u32 a0)
{
    Obj *o;

    if (D_8019F528 == 0) {
        switch (a0) {
        case 0:
            o = &D_801D3368;
        blk:
            o->flags &= ~0x1000;
            if (D_801C70EC == 0) {
                func_800529F0(0, 0, 0x85);
            } else if (D_801C70EC == 4) {
                func_800529F0(0, 0, 0x8C);
            } else {
                func_800529F0(0, 0, 0x93);
            }
            func_8004CE44(0x4D, (u8 *)o + 0x10, 0x64, 0);
            D_801C70EC++;
            return 1;
        case 1:
            o = &D_801D3428;
            goto blk;
        case 2:
            o = &D_801D3468;
            goto blk;
        case 3:
            o = &D_801D33A8;
            goto blk;
        case 4:
            o = &D_801D33E8;
            goto blk;
        case 10:
            if (D_801C70EC >= 5) {
                func_8008C2FC(0x100, func_801C6D94);
                return 1;
            }
            return 0;
        }
    }
    return 0;
}
