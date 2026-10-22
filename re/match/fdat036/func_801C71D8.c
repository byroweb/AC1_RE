#include "ac1/ac1.h"

typedef struct {
    u8 pad0[6];
    u8 f6;
    u8 pad7[9];
    u8 f10;
} Obj;
extern void func_8004CFC8();
extern void func_800529F0();
extern u16 D_8019F528;
extern s32 D_801C7364;
extern s32 D_801C7368;
extern Obj *D_801D3214;
extern Obj *D_801D3220;
extern Obj *D_801D3260;

s32 func_801C71D8(s32 a0)
{
    if (D_8019F528 == 0) {
        switch (a0) {
        case 0:
            if (D_801C7364 != 0) {
                return 1;
            }
            if (D_801D3214->f10 != 0) {
                return 0;
            }
            func_8004CFC8(0x4F, 0x78);
            func_800529F0(0, 0, 0x3B);
            D_801D3220->f6 = D_801D3260->f6 = 0x66;
            D_801C7364 = 1;
            return 1;
        case 1:
            D_801C7368 = a0;
            return 1;
        default:
            return 1;
        }
    }
    return 1;
}
