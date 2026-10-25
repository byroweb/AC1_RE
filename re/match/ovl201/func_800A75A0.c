#include "ac1/ac1.h"

extern s32 func_800309A8(s32 cmd, s32 arg, s32 param);
extern void func_800506EC(s32);
extern void func_80050804(s32, s16, s32);
extern void func_8005C2D0(s32, s32, s32, s32, s32);
extern u8 func_800A8618(void);
extern u8 func_800A86D0(s32 *, s32 *, s32);
extern void func_800A771C(UiObj *);
extern void func_800A78A0(UiObj *);
extern u8 D_80039CAE;
typedef struct { u8 pad[4]; u8 sel; } Rec411F4;
extern Rec411F4 D_800411F4;
extern u8 D_80048663[2];
extern s32 D_8017F458[8];
extern s32 D_8017F478[8];

void func_800A75A0(UiObj *o)
{
    s32 a, b, x;
    a = D_800411F4.sel;
    D_8017F478[0] = a;
    b = D_80048663[0];
    D_8017F478[1] = b;
    x = D_80048663[1];
    D_8017F478[2] = x;
    VSync(3);
    if (func_800A8618() == 0) {
        o->update = func_800A78A0;
        func_800309A8(2, 0, 0);
        VSync(0);
        func_800309A8(1, 1, 0);
        VSync(0);
        func_800506EC(12);
        obj_find_by_id(11)->kill = 1;
        func_8005C2D0(28, 170, 264, 32, 11);
        return;
    }
    if (func_800A86D0(D_8017F478, D_8017F458, 8) == 0) {
        o->update = func_800A78A0;
        func_800309A8(2, 0, 0);
        VSync(0);
        func_800309A8(1, 1, 0);
        VSync(0);
        func_800506EC(12);
        obj_find_by_id(11)->kill = 1;
        func_8005C2D0(28, 170, 264, 32, 11);
        return;
    }
    if (o->w.raw[0xD] == 0) {
        a = D_8017F458[0];
        b = D_8017F458[1];
        D_800411F4.sel = a;
        D_80039CAE = a;
        D_80048663[0] = b;
        D_80048663[1] = D_8017F458[2];
        x = a - 38;
        func_80050804(4, x, D_8017F458[2]);
    }
    o->update = func_800A771C;
}
