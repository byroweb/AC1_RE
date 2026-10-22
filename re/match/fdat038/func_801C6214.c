#include "ac1/ac1.h"

typedef struct {
    u8 pad0[6];
    u8 f6;
} Obj;
extern void func_8004C318();
extern s32 func_80052338();
extern void func_800529F0();
extern u16 D_8019F528;
extern s32 D_801C6504;
extern s32 D_801C6508;
extern Obj *D_801D3A20;
extern Obj *D_801D3AE0;
extern Obj *D_801D3C20;
extern Obj *D_801D3C60;

s32 func_801C6214(s32 a0)
{
    if (D_8019F528 == 0) {
        switch (a0) {
        case 1:
            if (D_801C6508 != 0) {
                if (func_80052338() != 0) {
                    func_800529F0(0, 0, 0x68);
                    func_8004C318(0x80);
                    return 1;
                }
            }
            return 0;
        case 2:
            func_800529F0(0, 0, 0x7A);
            func_800529F0(0, 0, 0x1E);
            func_800529F0(0, 0, 0x7B);
            D_801D3C20->f6 = D_801D3C60->f6 = 0x66;
            D_801D3A20->f6 = D_801D3AE0->f6 = 0;
            D_801C6504 = 1;
            return 1;
        }
    }
    return 0;
}
